# Python API 使用指南

## 1. 概述

msPTI Python API 提供了高层封装，使 Python 开发者能够快速接入 NPU 性能数据采集能力。Python API 围绕 **Monitor** 模式设计，每个 Monitor 负责一种类型的数据采集。

### Monitor 设计模式

每个 Monitor 遵循统一的生命周期：

```text
创建 Monitor → start(回调函数) → 执行业务代码 → stop()
```

- `start()`：启动采集，注册用户回调。
- `stop()`：停止采集，触发 Flush 确保所有数据已消费。
- `set_buffer_size(size)`：设置内部缓冲区大小（最大 256 MB）。
- `flush_all()`：手动刷新缓冲区。

### 可用 Monitor

| Monitor | 数据类 | 采集内容 |
| --- | --- | --- |
| `KernelMonitor` | `KernelData` | NPU Kernel 执行耗时 |
| `HcclMonitor` | `HcclData` | HCCL 通信操作（带宽、耗时） |
| `CommunicationMonitor` | `CommunicationData` | 通信算子数据（数据类型、算法等） |
| `MstxMonitor` | `MarkerData`, `RangeMarkerData` | 用户自定义打点（瞬时/范围） |

所有 Monitor 均在 `mspti` 包顶层暴露，可直接导入：

```python
from mspti import KernelMonitor, KernelData
from mspti import HcclMonitor, HcclData
from mspti import CommunicationMonitor, CommunicationData
from mspti import MstxMonitor, MarkerData, RangeMarkerData
```

---

## 2. 数据结构

### 2.1 KernelData

```python
class KernelData:
    kind: MsptiActivityKind       # 活动类型
    start: int                    # 开始时间戳（ns）
    end: int                      # 结束时间戳（ns）
    device_id: int                # 设备 ID
    stream_id: int                # 流 ID
    correlation_id: int           # 关联 ID
    type: str                     # Kernel 类型
    name: str                     # Kernel 名称
```

### 2.2 HcclData

```python
class HcclData:
    kind: MsptiActivityKind       # 活动类型
    start: int                    # 开始时间戳（ns）
    end: int                      # 结束时间戳（ns）
    device_id: int                # 设备 ID
    stream_id: int                # 流 ID
    bandwidth: float              # 带宽（GB/s）
    name: str                     # 通信算子名
    comm_name: str                # 通信组名
```

### 2.3 CommunicationData

```python
class CommunicationData:
    kind: MsptiActivityKind       # 活动类型
    data_type: MsptiCommunicationDataType  # 数据类型
    count: int                    # 数据计数
    device_id: int                # 设备 ID
    stream_id: int                # 流 ID
    start: int                    # 开始时间戳（ns）
    end: int                      # 结束时间戳（ns）
    alg_type: str                 # 算法类型
    name: str                     # 算子名
    comm_name: str                # 通信组名
    correlation_id: int           # 关联 ID
```

### 2.4 MarkerData（瞬时标记）

```python
class MarkerData:
    kind: MsptiActivityKind       # 活动类型
    flag: MsptiActivityFlag       # 标记标志
    source_kind: MsptiActivitySourceKind  # 数据来源
    timestamp: int                # 时间戳（ns）
    id: int                       # 标记 ID
    object_id: MsptiObjectId      # 对象标识
    name: str                     # 标记名称
    domain: str                   # 所属域
```

### 2.5 RangeMarkerData（范围标记）

```python
class RangeMarkerData:
    kind: MsptiActivityKind       # 活动类型
    source_kind: MsptiActivitySourceKind  # 数据来源
    id: int                       # 标记 ID
    object_id: MsptiObjectId      # 对象标识
    name: str                     # 标记名称
    domain: str                   # 所属域
    start: int                    # 范围开始时间戳（ns）
    end: int                      # 范围结束时间戳（ns）
```

### 2.6 MsptiObjectId

```python
class MsptiObjectId:
    process_id: int               # 进程 ID
    thread_id: int                # 线程 ID
    device_id: int                # 设备 ID
    stream_id: int                # 流 ID
```

---

## 3. KernelMonitor 使用指南

### 基本用法

```python
from mspti import KernelMonitor, KernelData

def on_kernel(data: KernelData):
    print(f"Kernel: {data.name}, type={data.type}, "
          f"start={data.start}, end={data.end}, "
          f"duration={(data.end - data.start) / 1000} us")

monitor = KernelMonitor()
monitor.start(on_kernel)

# 执行业务代码

monitor.stop()
```

### 完整示例（单卡）

```python
import torch
import torch_npu
from mspti import KernelMonitor, KernelData

def kernel_parser(data: KernelData):
    duration_us = (data.end - data.start) / 1000
    print(f"[Kernel] {data.name} | {data.type} | "
          f"{duration_us:.2f} us | device={data.device_id}")

monitor = KernelMonitor()
monitor.start(kernel_parser)

# 执行 NPU 计算
x = torch.randn(1024, 1024, dtype=torch.float16).npu()
y = torch.randn(1024, 1024, dtype=torch.float16).npu()
for _ in range(10):
    z = torch.matmul(x, y)
torch.npu.synchronize()

monitor.stop()
```

---

## 4. HcclMonitor 使用指南

采集 HCCL 通信操作的耗时和带宽信息。

```python
from mspti import HcclMonitor, HcclData

def on_hccl(data: HcclData):
    print(f"[HCCL] {data.name} | comm={data.comm_name} | "
          f"bandwidth={data.bandwidth:.2f} GB/s | "
          f"duration={(data.end - data.start) / 1000:.2f} us")

monitor = HcclMonitor()
monitor.start(on_hccl)

# 执行分布式训练代码（all_reduce 等）

monitor.stop()
```

---

## 5. CommunicationMonitor 使用指南

采集通信算子的详细信息，包括数据类型、算法类型等。

```python
from mspti import CommunicationMonitor, CommunicationData

def on_comm(data: CommunicationData):
    print(f"[COMM] {data.name} | alg={data.alg_type} | "
          f"type={data.data_type} | count={data.count} | "
          f"duration={(data.end - data.start) / 1000:.2f} us")

monitor = CommunicationMonitor()
monitor.start(on_comm)

# 执行分布式训练代码

monitor.stop()
```

---

## 6. MstxMonitor 使用指南

采集用户自定义的打点数据，支持瞬时标记和范围标记两种模式。

### 基本用法

```python
from mspti import MstxMonitor, MarkerData, RangeMarkerData

def on_marker(data: MarkerData):
    print(f"[MARK] {data.name} | timestamp={data.timestamp}")

def on_range(data: RangeMarkerData):
    print(f"[RANGE] {data.name} | domain={data.domain} | "
          f"start={data.start}, end={data.end}, "
          f"duration={(data.end - data.start) / 1000:.2f} us")

monitor = MstxMonitor()
monitor.start(on_marker, on_range)

# 执行业务代码

monitor.stop()
```

### 与 PyTorch MSTX 集成的完整示例

```python
import os
import threading
import time
import logging
from multiprocessing import Queue
import torch
import torch_npu
from mspti import MstxMonitor, MarkerData, RangeMarkerData

data_queue = Queue()
logging.basicConfig(level=logging.INFO)

def range_parser(data: RangeMarkerData):
    data_queue.put(data)

def consumer():
    while True:
        if not data_queue.empty():
            data = data_queue.get()
            if data is None:
                break
            duration_us = (data.end - data.start) / 1000
            logging.info(f"Range: {data.name}, {duration_us:.2f} us")
        else:
            time.sleep(0.1)

def test():
    consumer_thread = threading.Thread(target=consumer)
    consumer_thread.start()

    # 创建并启动 MstxMonitor
    monitor = MstxMonitor()
    monitor.start(range_parser)

    # 执行带打点的 NPU 计算
    device = int(os.getenv('LOCAL_RANK', '0'))
    torch.npu.set_device(device)

    x = torch.randn(256, 256, dtype=torch.float16).npu()
    y = torch.randn(256, 256, dtype=torch.float16).npu()

    stream = torch_npu.npu.current_stream()
    range_id = torch_npu.npu.mstx.range_start("matmul_range", stream)
    z = torch.matmul(x, y)
    torch_npu.npu.mstx.range_end(range_id)

    torch.npu.synchronize()

    # 停止采集
    monitor.stop()
    data_queue.put(None)
    consumer_thread.join()

if __name__ == "__main__":
    test()
```

### 域控制

MstxMonitor 支持按域动态启停打点采集：

```python
# 创建域（通过 MSTX API）
domain_name = "my_domain"

# 关闭指定域的采集
monitor.disable_domain(domain_name)

# 重新开启指定域的采集
monitor.enable_domain(domain_name)
```

---

## 7. 高级用法

### 7.1 多线程消费者模式

在高吞吐场景下，建议使用独立线程消费回调数据，避免阻塞采集回调：

```python
from multiprocessing import Queue
import threading
from mspti import KernelMonitor, KernelData

data_queue = Queue(maxsize=10000)

def kernel_parser(data: KernelData):
    data_queue.put(data)

def consumer():
    while True:
        data = data_queue.get()
        if data is None:
            break
        # 处理数据
        process_kernel_data(data)

# 启动消费者线程
consumer_thread = threading.Thread(target=consumer)
consumer_thread.start()

# 启动采集
monitor = KernelMonitor()
monitor.start(kernel_parser)
# ... 执行业务代码 ...
monitor.stop()

# 通知消费者退出
data_queue.put(None)
consumer_thread.join()
```

### 7.2 同时使用多个 Monitor

多个 Monitor 可以同时运行，互不干扰：

```python
from mspti import KernelMonitor, CommunicationMonitor

kernel_mon = KernelMonitor()
comm_mon = CommunicationMonitor()

kernel_mon.start(kernel_callback)
comm_mon.start(comm_callback)

# 执行业务代码

kernel_mon.stop()
comm_mon.stop()
```

### 7.3 设置缓冲区大小

在启动采集前调整缓冲区大小：

```python
monitor = KernelMonitor()
monitor.set_buffer_size(64)  # 设置 64 MB 缓冲区
monitor.start(callback)
```

缓冲区大小上限为 256 MB，默认值由 C 扩展决定。

---

## 8. 运行方式

### 环境要求

- Python 3.8+
- CANN 软件（含 msPTI Python 包）
- PyTorch + torch_npu（执行 NPU 计算时需要）
- 需设置 `LD_PRELOAD` 环境变量：

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
```

### 单卡运行

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
python your_script.py
```

### 多卡分布式运行

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
torchrun --nproc_per_node=8 your_script.py
```

---

## 9. 完整样例参考

| 样例 | 说明 |
| --- | --- |
| `samples/python_monitor/` | KernelMonitor + CommunicationMonitor 的基本用法 |
| `samples/python_mstx_monitor/` | MstxMonitor 的自定义打点用法 |
