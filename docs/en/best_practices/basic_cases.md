# msPTI Best Practices and Typical Use Cases

## 1. Interface Selection Guide

msPTI provides three sets of programming interfaces. Select the appropriate API based on your analysis goals and development language:

| Analysis Goal | Recommended Interface | Reason |
| --- | --- | --- |
| Collecting Kernel/Memory/Memcpy durations | Activity API | Asynchronous buffer mode, low overhead, and the most complete Activity type coverage |
| Inserting custom logic before and after API calls | Callback API | ENTER/EXIT callback points, with userdata passing and correlationData sharing |
| Quickly adding performance monitoring to Python training scripts | Python API | Monitor wrapper, one-line startup, and a built-in multithreaded consumer mode |
| Correlating API calls with Kernel execution | Activity API + correlationId | Establishes a one-to-one mapping through the `correlationId` field. |
| Distributed communication analysis (AllReduce and so on) | Activity API (Communication) + Python CommunicationMonitor | The C side collects all HCCL metadata, and the Python side integrates quickly |
| Instrumenting and collecting custom code segments | Callback API + MSTX (C), MstxMonitor (Python) | Domain-based control, dynamic start/stop, and reduced unnecessary performance loss |

### Hybrid Usage Strategy

The Callback API and Activity API can be enabled at the same time without conflicting with each other. Typical combinations:

- **Callback instrumentation + Activity collection**: Use the Callback API to call `mstxMarkA` at the entry and exit of Launch Kernel, and enable the Activity API to collect `MSPTI_ACTIVITY_KIND_MARKER` and `MSPTI_ACTIVITY_KIND_KERNEL` at the same time, enabling correlation analysis between the API context and Kernel execution data.
- **Activity collection + custom consumption**: Use the Activity API to enable multiple Kinds and customize data parsing and storage logic in CompleteFunc.

---

## 2. Activity API Best Practices

### 2.1 Buffer Management

Buffer management directly affects the completeness and performance of data collection:

**Buffer Size Selection:**

| Scenario | Recommended Buffer Size | Description |
| --- | --- | --- |
| Lightweight analysis (single-Kernel debugging) | 2 to 4 MB | Reduces memory usage and enables quick startup. |
| General collection (Kernel + API + Memcpy) | 8 to 16 MB | Balances memory and collection completeness. |
| Large-scale communication collection (multi-device HCCL) | 32 to 64 MB | Prevents data loss caused by frequent Flush. |
| Python Monitor | Up to 256 MB | Set through `set_buffer_size()`. |

**Buffer Reuse Strategy (Recommended):**

```cpp
uint8_t *g_cachedBuffer = nullptr;

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
    if (g_cachedBuffer) {
        // Reuse the buffer that was consumed in the last round to avoid repeated malloc/free
        *buffer = g_cachedBuffer;
        g_cachedBuffer = nullptr;
    } else {
        *buffer = (uint8_t*)malloc(BUFFER_SIZE);
    }
    *size = BUFFER_SIZE;
    *maxNumRecords = 0;
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
    // Consume the data
    ConsumeRecords(buffer, validSize);
    // Cache the buffer for reuse
    if (!g_cachedBuffer) {
        g_cachedBuffer = buffer;
    } else {
        free(buffer);
    }
}
```

**Avoid time-consuming operations in CompleteFunc** (such as file writes and network transfers). Instead, put the raw data into a queue and have a background thread process it asynchronously.

### 2.2 Activity Kind Enablement Principles

- **Enable on demand**: Enable only the Kinds required for your analysis. Each additional enabled Kind increases performance overhead.
- **All disabled by default**: All Kinds are disabled by default. You must explicitly call `msptiActivityEnable`.
- **Collect by phase**: If you need to analyze different data types in different phases, you can enable or disable Kinds by phase:

```cpp
// Phase 1: collect only Kernel
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
DoPhaseOne();
msptiActivityFlushAll(1);
msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);

// Phase 2: collect only Communication
msptiActivityEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
DoPhaseTwo();
msptiActivityFlushAll(1);
msptiActivityDisable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
```

### 2.3 `correlationId` Correlation Analysis

`correlationId` is the key field for correlating API calls with Kernel execution. Best practices:

1. **Deep-copy records**: The buffer is released after CompleteFunc returns. You must deep-copy the records that need to be retained.
2. **Index with a Map**: Build an `unordered_map` with `correlationId` as the key to achieve O(1) lookup.

```cpp
// Recommended: use a vector to support 1:N correlation
std::unordered_map<uint64_t, std::vector<msptiActivityKernel*>> kernelMap;
kernelMap[correlationId].push_back(copiedKernel);
```

### 2.4 Quick Dispatch

Dispatch records quickly by `kind` in CompleteFunc:

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
        // ... other Kinds
        default:
            break;
    }
}
```

### 2.5 External Correlation ID Usage Points

`msptiActivityPushExternalCorrelationId`/`Pop` uses stack semantics:

- **Must be used in pairs**: Each Push must have a matching Pop. Otherwise, the stack state becomes inconsistent.
- **Nesting supported**: Stacks of different `msptiExternalCorrelationKind` values are independent of each other.
- **Thread-local**: Push/Pop affects only the calling thread.
- **Applicable scenarios**: Framework-level wrappers (such as PyTorch custom operators) that need to map high-level semantics (forward/backward/optimization) to the underlying Runtime APIs.

---

## 3. Callback API Best Practices

### 3.1 Domain Granularity vs Callback ID Granularity

| Subscription Method | Applicable Scenario | Performance Overhead |
| --- | --- | --- |
| `msptiEnableDomain` | Full API call tracking during the debugging phase | Higher (a callback is triggered on every API call) |
| `msptiEnableCallback` | Only specific APIs are of interest (such as Launch Kernel) | Lower (callbacks are triggered only for the target APIs) |

**Recommendation**: In production environments, use `msptiEnableCallback` to precisely locate the APIs you care about. During the debugging phase, you can use `msptiEnableDomain` for full observation.

### 3.2 Userdata Passing

Pass the context in through userdata at subscription time to avoid global variables:

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

### 3.3 `correlationData` Shared Data

The `correlationData` pointer points to the same memory block between the ENTER and EXIT of the same API call. You can use it to calculate the API duration or pass temporary state:

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

### 3.4 Callback Function Performance Constraints

- **Keep callbacks lightweight**: Avoid file I/O, memory allocation, lock contention, and other operations in callbacks.
- **Use a thread-safe queue**: If you need to pass callback data to other threads, use a lock-free queue (such as `moodycamel::ConcurrentQueue`).
- **Reduce branch checks**: Filter the Domain and Callback ID at the callback entry and return quickly for calls that are not of interest.

---

## 4. Python API Best Practices

### 4.1 Monitor Lifecycle Management

```python
# Recommended: use the context manager style
monitor = KernelMonitor()
monitor.start(callback)
try:
    run_training()
finally:
    monitor.stop()  # Ensure that the monitor stops correctly even when an exception occurs
```

### 4.2 Consumer Thread Pattern

In high-throughput scenarios (such as multi-device training), the callback frequency can be very high. **Always use a dedicated consumer thread** to process data:

```python
from multiprocessing import Queue
import threading

data_queue = Queue(maxsize=5000)

def fast_callback(data):
    # Only enqueue data in the callback, without any processing
    data_queue.put(data)

def consumer():
    while True:
        data = data_queue.get()
        if data is None:
            break
        process(data)  # Time-consuming operations run in the consumer thread
```

### 4.3 Buffer Size Settings

```python
# Adjust the buffer size based on the scenario
monitor = KernelMonitor()

# Lightweight scenario (single-operator debugging)
monitor.set_buffer_size(8)    # 8 MB

# General training scenario
monitor.set_buffer_size(64)   # 64 MB

# Large-scale communication scenario
monitor.set_buffer_size(256)  # 256 MB (upper limit)

monitor.start(callback)
```

A buffer that is too small causes frequent Flush and increases CPU overhead, whereas a buffer that is too large increases memory usage. Start with 64 MB and adjust it according to the memory monitoring of `top` or `npu-smi`.

### 4.4 Multiple Monitors in Parallel

```python
from mspti import KernelMonitor, HcclMonitor, CommunicationMonitor

# Start the three monitors at the same time
kernel_mon = KernelMonitor()
hccl_mon = HcclMonitor()
comm_mon = CommunicationMonitor()

kernel_mon.start(kernel_cb)
hccl_mon.start(hccl_cb)
comm_mon.start(comm_cb)

run_distributed_training()

# Stop them in any order
kernel_mon.stop()
hccl_mon.stop()
comm_mon.stop()
```

Multiple monitors do not interfere with each other. Each monitor maintains an independent callback chain and buffer.

### 4.5 Distributed Training Environment

When you use `torchrun` to start multiple processes, each process should create an independent Monitor instance. Note:

- **Environment variable passing**: Use the `LOCAL_RANK` environment variable to distinguish devices.
- **One process per device**: Each process collects performance data only for the device it resides on.
- **Aggregate for analysis**: You are advised to aggregate the data of each process through `torch.distributed.all_gather` or log files before unified analysis.

---

## 5. Typical Case Analysis

### Case 1: Identifying Performance Bottlenecks in Single-Device Training

**Scenario**: PyTorch training on a single device, where you suspect that an operator is the performance bottleneck.

**Solution**: Use the Python KernelMonitor to collect Kernel execution durations.

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

# Run the training loop
for epoch in range(10):
    run_epoch()

monitor.stop()

# Output the hot Kernel top-N
sorted_kernels = sorted(kernel_stats.items(),
                        key=lambda x: x[1]["total"], reverse=True)
for name, stat in sorted_kernels[:10]:
    avg_us = stat["total"] / stat["count"] / 1000
    total_ms = stat["total"] / 1000000
    print(f"{name:50s} count={stat['count']:5d} avg={avg_us:8.2f}us total={total_ms:8.2f}ms")
```

**Analysis Points**:

- Focus on the operator with the largest `total` (the one that consumes the most time overall).
- Focus on operators with the largest `avg` and a high `count` (slow per invocation and frequently called).
- Use `max` to determine whether sporadic long-tail delays exist.

### Case 2: Distributed Training Communication Analysis

**Scenario**: Distributed training on 8 devices, where AllReduce communication takes an abnormally long time.

**Solution**: Use CommunicationMonitor and KernelMonitor to collect compute and communication durations separately.

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
    # Calculate the compute/communication ratio
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

# Distributed training
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

**Analysis Points**:

- **Compute/communication ratio**: If communication accounts for more than 30%, consider operator fusion or gradient accumulation.
- **Communication bandwidth**: Use the `bandwidth` field of HcclData to determine whether the hardware upper limit is reached.
- **Communication operator type**: Use the `alg_type` field of CommunicationData to confirm the communication algorithm used (such as `Ring` and `Tree`).

### Case 3: Custom Operator Instrumentation Analysis

**Scenario**: Add instrumentation before and after a PyTorch custom operator to measure the operator duration precisely.

**Solution**: Use MstxMonitor and the `torch_npu.npu.mstx` API.

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

# Instrument the operators of interest
x = torch.randn(1024, 1024, dtype=torch.float16).npu()
y = torch.randn(1024, 1024, dtype=torch.float16).npu()
stream = torch_npu.npu.current_stream()

# Instrumentation: matmul
rid1 = torch_npu.npu.mstx.range_start("matmul", stream)
z1 = torch.matmul(x, y)
torch_npu.npu.mstx.range_end(rid1)

# Instrumentation: add
rid2 = torch_npu.npu.mstx.range_start("add", stream)
z2 = x + y
torch_npu.npu.mstx.range_end(rid2)

# Instrumentation: custom fusion operator
rid3 = torch_npu.npu.mstx.range_start("custom_fusion", stream)
z3 = custom_fusion_op(x, y)
torch_npu.npu.mstx.range_end(rid3)

torch.npu.synchronize()
monitor.stop()

for name, duration in results:
    print(f"{name}: {duration:.2f} us")
```

### Case 4: Full-Path Tracing on the C Side

**Scenario**: A C/C++ inference application needs to trace the full-path duration from API submission to Kernel execution.

**Solution**: Use the Activity API with correlationId correlation.

```text
The application calls aclrtLaunchKernel
  → msPTI records an MSPTI_ACTIVITY_KIND_API record (correlationId=100)
  → CANN Runtime submits the Kernel to the NPU
  → msPTI records an MSPTI_ACTIVITY_KIND_KERNEL record (correlationId=100)
  → Correlation analysis: API(100).name == "LaunchKernel" -> Kernel(100).name == "MatMul"
```

**Key Code Points**:

```cpp
// 1. Enable API and Kernel collection
msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

// 2. Correlate by correlationId in CompleteFunc
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

// 3. Output the full-path durations
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

### Case 5: Dynamically Controlling the Collection Scope by Phase

**Scenario**: The training script is divided into four phases, that is, data loading, forward propagation, backward propagation, and parameter update. You want to collect only the Kernel data of the forward propagation phase.

**Solution**: Combine the Activity API, the Callback API, and MSTX domain control.

```cpp
// Enable collection at the start of forward propagation
msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

// Forward propagation code
ForwardPass();

// Disable collection after forward propagation to avoid data generated by backward propagation
msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);
msptiActivityFlushAll(0);
```

Alternatively, use MSTX domain control for finer-grained instrumentation management:

```cpp
// Create two domains
auto domainForward = mstxDomainCreateA("forward");
auto domainBackward = mstxDomainCreateA("backward");

// Enable Marker collection for the forward domain only
msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
msptiActivityDisableMarkerDomain("backward");

// Instrument forward propagation (the records are collected)
mstxDomainRangeStartA(domainForward, "conv1", stream);
// ... forward computation ...
mstxDomainRangeEnd(domainForward, id1);

// Enable the backward domain and disable the forward domain
msptiActivityEnableMarkerDomain("backward");
msptiActivityDisableMarkerDomain("forward");

// Instrument backward propagation (the records are collected, but forward instrumentation is not)
mstxDomainRangeStartA(domainBackward, "conv1_grad", stream);
// ... backward computation ...
mstxDomainRangeEnd(domainBackward, id2);
```

---

## 6. Performance Considerations

### 6.1 Collection Overhead

| Operation | Relative Overhead | Description |
| --- | --- | --- |
| `msptiActivityEnable` to enable Kinds | Negligible | Only sets flag bits. |
| Activity Buffer write | Low | Memory write operation, at the nanosecond level |
| RequestFunc callback | Medium | Involves memory allocation or cache lookup. |
| CompleteFunc callback | Depends on your logic | Keep it lightweight and avoid I/O. |
| Callback ENTER/EXIT | Medium | A function call is triggered on every API call |
| Python Monitor callback | Medium | Conversion overhead from the Python C extension to the Python layer |

### 6.2 Tips for Reducing Overhead

1. **Enable precisely**: Enable only the Activity Kinds you need to avoid meaningless collection.
2. **Use `msptiEnableCallback` instead of `msptiEnableDomain`**: Precisely locate the APIs you care about.
3. **Avoid I/O in callbacks**: Enqueue the data and process it asynchronously.
4. **Set a reasonable buffer size**: An overly small buffer causes frequent Flush and increases CPU overhead.
5. **Use `msptiActivityFlushPeriod` to control the Flush frequency**: Set a reasonable period (for example, 100 ms) to balance real-time performance and overhead.
6. **Use domain control to reduce the amount of instrumentation data**: Disable the Marker domains that are not needed.

### 6.3 Memory Usage

- Each Activity Buffer occupies the memory specified by the `size` parameter.
- When multiple Kinds are enabled, each Kind generates records independently. The total data volume is proportional to the collection duration and the Activity density.
- You are advised to call `flush_all` or `stop` promptly after collection to release resources.

---

## 7. Common Anti-Patterns

| Pattern | Problem | Correct Approach |
| --- | --- | --- |
| Writing files in CompleteFunc | Blocks buffer recycling, causing data loss. | Enqueue the data and write it in a background thread. |
| Enabling all Activity Kinds | Unnecessary data collection increases overhead | Enable only the Kinds required for analysis. |
| Not handling duplicate `correlationId` values | Missing 1:N Kernel records | Use a `vector` or `multimap`. |
| Forgetting to call `msptiActivityRegisterCallbacks` | No data is returned after enabling Kinds | Register callbacks first, and then enable Kinds. |
| Forgetting to Pop after Push | The external correlation stack becomes inconsistent | Ensure that Push and Pop appear in pairs. |
| Heavy processing in Python callbacks | Blocks the internal Monitor thread. | Use a consumer thread for asynchronous processing. |
| Sharing a Monitor instance across processes | Data corruption or crashes | Create an independent Monitor in each process. |
