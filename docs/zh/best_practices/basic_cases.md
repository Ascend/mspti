# msPTI 最佳实践与典型案例

## 一、接口选型指南

msPTI 提供三套编程接口，应根据分析目标和开发语言选择合适的 API：

| 分析目标 | 推荐接口 | 理由 |
| --- | --- | --- |
| 采集 Kernel / Memory / Memcpy 耗时 | Activity API | 异步缓冲区模式，开销低，覆盖 Activity 类型最全 |
| 在 API 调用前后插入自定义逻辑 | Callback API | ENTER/EXIT 回调点，支持 userdata 透传和 correlationData 共享 |
| 快速为 Python 训练脚本添加性能监控 | Python API | Monitor 封装，一行启动，内置多线程消费者模式 |
| API 调用与 Kernel 执行关联分析 | Activity API + correlationId | 通过 `correlationId` 字段建立一一对应关系 |
| 分布式通信分析（AllReduce 等） | Activity API（Communication）+ Python CommunicationMonitor | C 侧采集全量 HCCL 元数据，Python 侧快速接入 |
| 自定义代码段打点 + 采集 | Callback API + MSTX（C），MstxMonitor（Python） | 按域控制，动态启停，减少不必要的性能损耗 |

### 混合使用策略

Callback API 和 Activity API 可以同时使能，互不冲突。典型组合：

- **Callback 打点 + Activity 采集**：使用 Callback API 在 Launch Kernel 入口/出口调用 `mstxMarkA` 打点，同时使能 Activity API 采集 `MSPTI_ACTIVITY_KIND_MARKER` 和 `MSPTI_ACTIVITY_KIND_KERNEL`，实现 API 上下文与 Kernel 执行数据的关联分析。
- **Activity 采集 + 自定义消费**：使用 Activity API 使能多种 Kind，在 CompleteFunc 中自定义数据解析和存储逻辑。

---

## 二、Activity API 最佳实践

### 2.1 缓冲区管理

缓冲区管理直接影响数据采集的完整性和性能：

**缓冲区大小选择：**

| 场景 | 推荐缓冲区大小 | 说明 |
| --- | --- | --- |
| 轻量分析（单 Kernel 调试） | 2 ~ 4 MB | 减少内存占用，快速启动 |
| 通用采集（Kernel + API + Memcpy） | 8 ~ 16 MB | 平衡内存与采集完整性 |
| 大规模通信采集（多卡 HCCL） | 32 ~ 64 MB | 避免高频 Flush 导致的数据丢失 |
| Python Monitor | 最大 256 MB | 通过 `set_buffer_size()` 设置 |

**缓冲区复用策略（推荐）：**

```cpp
uint8_t *g_cachedBuffer = nullptr;

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
    if (g_cachedBuffer) {
        // 复用上次消费完的缓冲区，避免反复 malloc/free
        *buffer = g_cachedBuffer;
        g_cachedBuffer = nullptr;
    } else {
        *buffer = (uint8_t*)malloc(BUFFER_SIZE);
    }
    *size = BUFFER_SIZE;
    *maxNumRecords = 0;
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
    // 消费数据
    ConsumeRecords(buffer, validSize);
    // 缓存缓冲区以便复用
    if (!g_cachedBuffer) {
        g_cachedBuffer = buffer;
    } else {
        free(buffer);
    }
}
```

**避免在 CompleteFunc 中执行耗时操作**（文件写入、网络传输等），应将原始数据放入队列由后台线程异步处理。

### 2.2 Activity Kind 使能原则

- **按需使能**：只使能分析所需的 Kind。每多使能一个 Kind 都会增加性能开销。
- **默认全关**：所有 Kind 默认为关闭状态，必须显式调用 `msptiActivityEnable`。
- **分段采集**：若需要分析不同阶段的不同数据类型，可分段使能/禁用：

```cpp
// 阶段一：只采集 Kernel
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
DoPhaseOne();
msptiActivityFlushAll(1);
msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);

// 阶段二：只采集 Communication
msptiActivityEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
DoPhaseTwo();
msptiActivityFlushAll(1);
msptiActivityDisable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
```

### 2.3 correlationId 关联分析

`correlationId` 是将 API 调用与 Kernel 执行关联的关键字段。最佳实践：

1. **深拷贝记录**：缓冲区在 CompleteFunc 返回后会被释放，必须深拷贝需要保留的记录。
2. **使用 Map 索引**：以 `correlationId` 为 key 建立 `unordered_map`，实现 O(1) 查找。

```cpp
// 推荐：使用 vector 支持 1:N 关联
std::unordered_map<uint64_t, std::vector<msptiActivityKernel*>> kernelMap;
kernelMap[correlationId].push_back(copiedKernel);
```

### 2.4 Quick Sort

在 CompleteFunc 中通过 `kind` 快速分派：

```cpp
void DispatchRecord(msptiActivity *record) {
    switch (record->kind) {
        case MSPTI_ACTIVITY_KIND_KERNEL:
            HandleKernel((msptiActivityKernel*)record);
            break;
        case MSPTI_ACTIVITY_KIND_API:
            HandleApi((msptiActivityApi*)record);
            break;
        case MSPTI_ACTIVITY_KIND_MEMCPY:
            HandleMemcpy((msptiActivityMemcpy*)record);
            break;
        case MSPTI_ACTIVITY_KIND_COMMUNICATION:
            HandleCommunication((msptiActivityCommunication*)record);
            break;
        // ... 其他 Kind
        default:
            break;
    }
}
```

### 2.5 外部关联 ID 使用要点

`msptiActivityPushExternalCorrelationId` / `Pop` 采用栈语义：

- **必须成对使用**：每个 Push 对应一个 Pop，否则会导致栈状态混乱。
- **支持嵌套**：不同 `msptiExternalCorrelationKind` 的栈相互独立。
- **线程局部**：Push/Pop 仅影响调用线程。
- **适用场景**：框架层封装（如 PyTorch 自定义算子），需要将高层语义（前向/反向/优化）映射到底层 Runtime API。

---

## 三、Callback API 最佳实践

### 3.1 Domain 粒度 vs Callback ID 粒度

| 订阅方式 | 适用场景 | 性能开销 |
| --- | --- | --- |
| `msptiEnableDomain` | 全量 API 调用跟踪，调试阶段 | 较高（每次 API 调用都触发回调） |
| `msptiEnableCallback` | 仅关注特定 API（如 Launch Kernel） | 较低（仅目标 API 触发回调） |

**建议**：生产环境使用 `msptiEnableCallback` 精确定位关心的 API；调试阶段可使用 `msptiEnableDomain` 全量观察。

### 3.2 Userdata 透传

通过 userdata 在订阅时传入上下文，避免使用全局变量：

```cpp
struct ProfileContext {
    aclrtContext ctx;
    aclrtStream stream;
    uint64_t sessionId;
    std::function<void(const char*)> logFunc;
};

void Callback(void *userdata, msptiCallbackDomain domain,
              msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    auto *ctx = (ProfileContext*)userdata;
    ctx->logFunc(cbdata->functionName);
}

auto context = std::make_unique<ProfileContext>(...);
msptiSubscribe(&subscriber, Callback, context.get());
```

### 3.3 correlationData 共享数据

`correlationData` 指针在同一 API 调用的 ENTER 和 EXIT 之间指向同一块内存，可用于计算 API 耗时或传递临时状态：

```cpp
void TimingCallback(void *userdata, msptiCallbackDomain domain,
                    msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (cbdata->callbackSite == MSPTI_API_ENTER) {
        if (cbdata->correlationData) {
            *cbdata->correlationData = GetNanoTimestamp();
        }
    } else {
        if (cbdata->correlationData && *cbdata->correlationData > 0) {
            uint64_t elapsed = GetNanoTimestamp() - *cbdata->correlationData;
            printf("%s took %lu ns\n", cbdata->functionName, elapsed);
        }
    }
}
```

### 3.4 回调函数性能约束

- **回调函数应保持轻量**：避免在回调中执行文件 I/O、内存分配、锁竞争等操作。
- **使用线程安全队列**：若需要将回调数据传递到其他线程，使用无锁队列（如 `moodycamel::ConcurrentQueue`）。
- **减少分支判断**：在回调入口先过滤 Domain 和 Callback ID，快速返回不关心的调用。

---

## 四、Python API 最佳实践

### 4.1 Monitor 生命周期管理

```python
# 推荐：使用上下文管理器风格
monitor = KernelMonitor()
monitor.start(callback)
try:
    run_training()
finally:
    monitor.stop()  # 确保异常时也能正确停止
```

### 4.2 消费者线程模式

在高吞吐场景（如多卡训练），回调频率可能很高。**始终使用独立消费者线程**处理数据：

```python
from multiprocessing import Queue
import threading

data_queue = Queue(maxsize=5000)

def fast_callback(data):
    # 回调中只做入队操作，不做任何处理
    data_queue.put(data)

def consumer():
    while True:
        data = data_queue.get()
        if data is None:
            break
        process(data)  # 耗时操作在消费者线程中执行
```

### 4.3 缓冲区大小设置

```python
# 根据场景调整缓冲区大小
monitor = KernelMonitor()

# 轻量场景（单算子调试）
monitor.set_buffer_size(8)    # 8 MB

# 通用训练场景
monitor.set_buffer_size(64)   # 64 MB

# 大规模通信场景
monitor.set_buffer_size(256)  # 256 MB（上限）

monitor.start(callback)
```

缓冲区过小会导致频繁 Flush，增加 CPU 开销；缓冲区过大则增加内存占用。建议从 64 MB 开始，根据 `top` 或 `npu-smi` 的内存监控调整。

### 4.4 多 Monitor 并发

```python
from mspti import KernelMonitor, HcclMonitor, CommunicationMonitor

# 同时启动三个 Monitor
kernel_mon = KernelMonitor()
hccl_mon = HcclMonitor()
comm_mon = CommunicationMonitor()

kernel_mon.start(kernel_cb)
hccl_mon.start(hccl_cb)
comm_mon.start(comm_cb)

run_distributed_training()

# 按任意顺序停止
kernel_mon.stop()
hccl_mon.stop()
comm_mon.stop()
```

多 Monitor 之间互不干扰，每个 Monitor 维护独立的回调链和缓冲区。

### 4.5 分布式训练环境

使用 `torchrun` 启动多进程时，每个进程应创建独立的 Monitor 实例。注意：

- **环境变量传递**：通过 `LOCAL_RANK` 环境变量区分设备。
- **单进程单设备**：每个进程只采集自身所在设备的性能数据。
- **汇总分析**：建议将各进程的数据通过 `torch.distributed.all_gather` 或日志文件汇总后统一分析。

---

## 五、典型案例分析

### 案例一：单卡训练性能瓶颈定位

**场景**：PyTorch 单卡训练，怀疑某个算子是性能瓶颈。

**方案**：使用 Python KernelMonitor 采集 Kernel 执行耗时。

```python
import torch
import torch_npu
from mspti import KernelMonitor, KernelData

kernel_stats = {}

def on_kernel(data: KernelData):
    name = data.name
    duration = data.end - data.start
    if name not in kernel_stats:
        kernel_stats[name] = {"count": 0, "total": 0, "max": 0}
    kernel_stats[name]["count"] += 1
    kernel_stats[name]["total"] += duration
    kernel_stats[name]["max"] = max(kernel_stats[name]["max"], duration)

monitor = KernelMonitor()
monitor.set_buffer_size(64)
monitor.start(on_kernel)

# 执行训练 Loop
for epoch in range(10):
    run_epoch()

monitor.stop()

# 输出热点 Kernel Top-N
sorted_kernels = sorted(kernel_stats.items(),
                        key=lambda x: x[1]["total"], reverse=True)
for name, stat in sorted_kernels[:10]:
    avg_us = stat["total"] / stat["count"] / 1000
    total_ms = stat["total"] / 1000000
    print(f"{name:50s} count={stat['count']:5d} avg={avg_us:8.2f}us total={total_ms:8.2f}ms")
```

**分析要点**：

- 关注 `total` 最大的算子（整体耗时最多）。
- 关注 `avg` 最大且 `count` 较高的算子（单次慢且频繁调用）。
- 结合 `max` 判断是否存在偶发性长尾。

### 案例二：分布式训练通信分析

**场景**：8 卡分布式训练，AllReduce 通信耗时异常高。

**方案**：使用 CommunicationMonitor + KernelMonitor 分别采集计算和通信耗时。

```python
import os
import torch
import torch_npu
import torch.distributed as dist
from multiprocessing import Queue
from mspti import KernelMonitor, CommunicationMonitor, KernelData, CommunicationData

data_queue = Queue(maxsize=10000)

def on_kernel(data: KernelData):
    data_queue.put(("kernel", data))

def on_comm(data: CommunicationData):
    data_queue.put(("comm", data))

def consumer():
    kernel_times = []
    comm_times = []
    while True:
        item = data_queue.get()
        if item is None:
            break
        kind, data = item
        duration = (data.end - data.start) / 1000
        if kind == "kernel":
            kernel_times.append(duration)
        else:
            comm_times.append(duration)
    # 计算计算/通信比例
    total_kernel = sum(kernel_times)
    total_comm = sum(comm_times)
    ratio = total_comm / total_kernel if total_kernel > 0 else 0
    print(f"Total compute: {total_kernel:.2f} us")
    print(f"Total comm:    {total_comm:.2f} us")
    print(f"Comm/Compute ratio: {ratio:.2%}")

import threading
consumer_thread = threading.Thread(target=consumer)
consumer_thread.start()

kernel_mon = KernelMonitor()
comm_mon = CommunicationMonitor()
kernel_mon.start(on_kernel)
comm_mon.start(on_comm)

# 分布式训练
dist.init_process_group(backend='hccl', ...)
device = int(os.getenv('LOCAL_RANK'))
torch.npu.set_device(device)

for _ in range(100):
    x = torch.randn(1024, 1024, dtype=torch.float16).npu()
    y = torch.randn(1024, 1024, dtype=torch.float16).npu()
    z = torch.matmul(x, y)
    dist.all_reduce(z)

torch.npu.synchronize()

kernel_mon.stop()
comm_mon.stop()
data_queue.put(None)
consumer_thread.join()
```

**分析要点**：

- **计算/通信比例**：若通信占比超过 30%，考虑算子融合或梯度累积。
- **通信带宽**：结合 HcclData 的 `bandwidth` 字段判断是否达到硬件上限。
- **通信算子类型**：通过 CommunicationData 的 `alg_type` 字段确认使用的通信算法（如 `Ring`、`Tree` 等）。

### 案例三：自定义算子打点分析

**场景**：在 PyTorch 自定义算子的前后打点，精确测量算子耗时。

**方案**：使用 MstxMonitor + `torch_npu.npu.mstx` API。

```python
import torch
import torch_npu
from mspti import MstxMonitor, RangeMarkerData

results = []

def on_range(data: RangeMarkerData):
    duration = (data.end - data.start) / 1000
    results.append((data.name, duration))

monitor = MstxMonitor()
monitor.start(range_cb=on_range)

# 对关心的算子打点
x = torch.randn(1024, 1024, dtype=torch.float16).npu()
y = torch.randn(1024, 1024, dtype=torch.float16).npu()
stream = torch_npu.npu.current_stream()

# 打点：matmul
rid1 = torch_npu.npu.mstx.range_start("matmul", stream)
z1 = torch.matmul(x, y)
torch_npu.npu.mstx.range_end(rid1)

# 打点：add
rid2 = torch_npu.npu.mstx.range_start("add", stream)
z2 = x + y
torch_npu.npu.mstx.range_end(rid2)

# 打点：自定义 fusion 算子
rid3 = torch_npu.npu.mstx.range_start("custom_fusion", stream)
z3 = custom_fusion_op(x, y)
torch_npu.npu.mstx.range_end(rid3)

torch.npu.synchronize()
monitor.stop()

for name, duration in results:
    print(f"{name}: {duration:.2f} us")
```

### 案例四：C 侧全链路追踪

**场景**：C/C++ 推理应用，需要追踪从 API 下发到 Kernel 执行的全链路耗时。

**方案**：Activity API + correlationId 关联。

```text
应用调用 aclrtLaunchKernel
  → msPTI 记录 MSPTI_ACTIVITY_KIND_API 记录（correlationId=100）
  → CANN Runtime 下发 Kernel 到 NPU
  → msPTI 记录 MSPTI_ACTIVITY_KIND_KERNEL 记录（correlationId=100）
  → 关联分析：API(100).name == "LaunchKernel" -> Kernel(100).name == "MatMul"
```

**代码要点**：

```cpp
// 1. 使能 API 和 Kernel 采集
msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

// 2. 在 CompleteFunc 中按 correlationId 关联
void AnalyzeCallback(uint8_t *buffer, size_t size, size_t validSize) {
    msptiActivity *record = nullptr;
    while (msptiActivityGetNextRecord(buffer, validSize, &record) == MSPTI_SUCCESS) {
        if (record->kind == MSPTI_ACTIVITY_KIND_API) {
            auto *api = (msptiActivityApi*)record;
            apiMap[api->correlationId] = DeepCopyApi(api);
        } else if (record->kind == MSPTI_ACTIVITY_KIND_KERNEL) {
            auto *ker = (msptiActivityKernel*)record;
            kernelMap[ker->correlationId].push_back(DeepCopyKernel(ker));
        }
    }
}

// 3. 输出全链路耗时
for (auto &[corrId, api] : apiMap) {
    printf("API: %s (%lu ~ %lu, %lu ns)\n",
           api->name, api->start, api->end, api->end - api->start);
    for (auto *kernel : kernelMap[corrId]) {
        printf("  └─ Kernel: %s (%lu ~ %lu, %lu ns)\n",
               kernel->name, kernel->start, kernel->end,
               kernel->end - kernel->start);
    }
}
```

### 案例五：按阶段动态控制采集范围

**场景**：训练脚本分为数据加载、前向传播、反向传播、参数更新四个阶段，只想采集前向传播阶段的 Kernel 数据。

**方案**：结合 Activity API + Callback API + MSTX 域控制。

```cpp
// 在前向传播开始处使能采集
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

// 前向传播代码
ForwardPass();

// 在前向传播结束后禁用采集，避免反向传播产生的数据
msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);
msptiActivityFlushAll(0);
```

或使用 MSTX 域控制实现更细粒度的打点管理：

```cpp
// 创建两个域
auto domainForward = mstxDomainCreateA("forward");
auto domainBackward = mstxDomainCreateA("backward");

// 仅使能前向域的 Marker 采集
msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
msptiActivityDisableMarkerDomain("backward");

// 前向传播打点（会被采集）
mstxDomainRangeStartA(domainForward, "conv1", stream);
// ... 前向计算 ...
mstxDomainRangeEnd(domainForward, id1);

// 使能反向域，关闭前向域
msptiActivityEnableMarkerDomain("backward");
msptiActivityDisableMarkerDomain("forward");

// 反向传播打点（会被采集，但前向打点不会被采集）
mstxDomainRangeStartA(domainBackward, "conv1_grad", stream);
// ... 反向计算 ...
mstxDomainRangeEnd(domainBackward, id2);
```

---

## 六、性能考量

### 6.1 采集开销

| 操作 | 相对开销 | 说明 |
| --- | --- | --- |
| `msptiActivityEnable` 使能 Kind | 忽略不计 | 仅设置标志位 |
| Activity Buffer 写入 | 低 | 内存写入操作，纳秒级 |
| RequestFunc 回调 | 中 | 涉及内存分配或缓存查找 |
| CompleteFunc 回调 | 取决于用户逻辑 | 应保持轻量，避免 I/O |
| Callback ENTER/EXIT | 中 | 每次 API 调用触发函数调用 |
| Python Monitor 回调 | 中 | Python C 扩展到 Python 层的转换开销 |

### 6.2 降低开销的技巧

1. **精准使能**：只使能需要的 Activity Kind，避免无意义的采集。
2. **使用 `msptiEnableCallback` 替代 `msptiEnableDomain`**：精确定位关心的 API。
3. **避免在回调中执行 I/O**：将数据入队后异步处理。
4. **合理设置缓冲区大小**：过小的缓冲区导致频繁 Flush，增加 CPU 开销。
5. **使用 `msptiActivityFlushPeriod` 控制 Flush 频率**：设置合理的周期（如 100 ms）平衡实时性和开销。
6. **利用域控制减少打点数据量**：关闭不需要的 Marker 域。

### 6.3 内存占用

- 每个 Activity Buffer 占用 `size` 参数指定的内存。
- 多 Kind 使能时，每个 Kind 独立生成记录，总数据量与采集时长和 Activity 密度成正比。
- 建议在采集完成后及时调用 `flush_all` 或 `stop` 释放资源。

---

## 七、常见不好的使用模式

| 模式 | 问题 | 正确做法 |
| --- | --- | --- |
| 在 CompleteFunc 中写文件 | 阻塞缓冲区回收，导致数据丢失 | 将数据入队，由后台线程写入 |
| 使能所有 Activity Kind | 不必要的数据采集增加开销 | 只使能分析所需的 Kind |
| 不处理 `correlationId` 重复 | 漏掉 1:N 的 Kernel 记录 | 使用 `vector` 或 `multimap` |
| 忘记调用 `msptiActivityRegisterCallbacks` | 使能 Kind 后无数据返回 | 先注册回调，再使能 Kind |
| Push 后忘记 Pop | 外部关联栈错乱 | 确保 Push/Pop 成对出现 |
| Python 回调中做重量级处理 | 阻塞 Monitor 内部线程 | 使用消费者线程异步处理 |
| 多进程共用 Monitor 实例 | 数据错乱或崩溃 | 每个进程独立创建 Monitor |
