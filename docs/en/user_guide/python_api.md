# Python API Usage Guide

## 1. Overview

The msPTI Python API provides a high-level wrapper that enables Python developers to quickly integrate NPU performance data collection capabilities. The Python API is designed around the **Monitor** mode, where each Monitor is responsible for collecting one type of data.

### 1.1 Monitor Design Pattern

Each Monitor follows a unified lifecycle:

```text
Create Monitor → start(callback) → Run the service code → stop()
```

- `start()`: Starts collection and registers the user callback.
- `stop()`: Stops collection and triggers a Flush to ensure that all data has been consumed.
- `set_buffer_size(size)`: Sets the internal buffer size (up to 256 MB).
- `flush_all()`: Manually flushes the buffer.

### 1.2 Available Monitors

| Monitor | Data Type | Data Collected |
| --- | --- | --- |
| `KernelMonitor` | `KernelData` | NPU Kernel execution time |
| `HcclMonitor` | `HcclData` | HCCL communication operations (bandwidth, time) |
| `CommunicationMonitor` | `CommunicationData` | Communication operator data (data type, algorithm, and so on) |
| `MstxMonitor` | `MarkerData`, `RangeMarkerData` | User-defined markers (instant/range) |

All Monitors are exposed at the top level of the `mspti` package and can be imported directly:

```python
from mspti import KernelMonitor, KernelData
from mspti import HcclMonitor, HcclData
from mspti import CommunicationMonitor, CommunicationData
from mspti import MstxMonitor, MarkerData, RangeMarkerData
```

---

## 2. Data Structures

### 2.1 KernelData

```python
class KernelData:
    kind: MsptiActivityKind       # Activity type
    start: int                    # Start timestamp (ns)
    end: int                      # End timestamp (ns)
    device_id: int                # Device ID
    stream_id: int                # Stream ID
    correlation_id: int           # Correlation ID
    type: str                     # Kernel type
    name: str                     # Kernel name
```

### 2.2 HcclData

```python
class HcclData:
    kind: MsptiActivityKind       # Activity type
    start: int                    # Start timestamp (ns)
    end: int                      # End timestamp (ns)
    device_id: int                # Device ID
    stream_id: int                # Stream ID
    bandwidth: float              # Bandwidth (GB/s)
    name: str                     # Communication operator name
    comm_name: str                # Communication group name
```

### 2.3 CommunicationData

```python
class CommunicationData:
    kind: MsptiActivityKind       # Activity type
    data_type: MsptiCommunicationDataType  # Data type
    count: int                    # Data count
    device_id: int                # Device ID
    stream_id: int                # Stream ID
    start: int                    # Start timestamp (ns)
    end: int                      # End timestamp (ns)
    alg_type: str                 # Algorithm type
    name: str                     # Operator name
    comm_name: str                # Communication group name
    correlation_id: int           # Correlation ID
```

### 2.4 MarkerData (Instant Markers)

```python
class MarkerData:
    kind: MsptiActivityKind       # Activity type
    flag: MsptiActivityFlag       # Marker flag
    source_kind: MsptiActivitySourceKind  # Data source
    timestamp: int                # Timestamp (ns)
    id: int                       # Marker ID
    object_id: MsptiObjectId      # Object ID
    name: str                     # Marker name
    domain: str                   # Domain
```

### 2.5 RangeMarkerData (Range Markers)

```python
class RangeMarkerData:
    kind: MsptiActivityKind       # Activity type
    source_kind: MsptiActivitySourceKind  # Data source
    id: int                       # Marker ID
    object_id: MsptiObjectId      # Object ID
    name: str                     # Marker name
    domain: str                   # Domain
    start: int                    # Range start timestamp (ns)
    end: int                      # Range end timestamp (ns)
```

### 2.6 MsptiObjectId

```python
class MsptiObjectId:
    process_id: int               # Process ID
    thread_id: int                # Thread ID
    device_id: int                # Device ID
    stream_id: int                # Stream ID
```

---

## 3. KernelMonitor Usage Guide

### 3.1 Basic Usage

```python
from mspti import KernelMonitor, KernelData

def on_kernel(data: KernelData):
    print(f"Kernel: {data.name}, type={data.type}, "
          f"start={data.start}, end={data.end}, "
          f"duration={(data.end - data.start) / 1000} us")

monitor = KernelMonitor()
monitor.start(on_kernel)

# Run the service code

monitor.stop()
```

### 3.2 Complete Example (Single Device)

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

# Run NPU computation
x = torch.randn(1024, 1024, dtype=torch.float16).npu()
y = torch.randn(1024, 1024, dtype=torch.float16).npu()
for _ in range(10):
    z = torch.matmul(x, y)
torch.npu.synchronize()

monitor.stop()
```

---

## 4. HcclMonitor Usage Guide

Collects the time and bandwidth information of HCCL communication operations.

```python
from mspti import HcclMonitor, HcclData

def on_hccl(data: HcclData):
    print(f"[HCCL] {data.name} | comm={data.comm_name} | "
          f"bandwidth={data.bandwidth:.2f} GB/s | "
          f"duration={(data.end - data.start) / 1000:.2f} us")

monitor = HcclMonitor()
monitor.start(on_hccl)

# Run distributed training code (such as all_reduce)

monitor.stop()
```

---

## 5. CommunicationMonitor Usage Guide

Collects detailed information about communication operators, including the data type, algorithm type, and so on.

```python
from mspti import CommunicationMonitor, CommunicationData

def on_comm(data: CommunicationData):
    print(f"[COMM] {data.name} | alg={data.alg_type} | "
          f"type={data.data_type} | count={data.count} | "
          f"duration={(data.end - data.start) / 1000:.2f} us")

monitor = CommunicationMonitor()
monitor.start(on_comm)

# Run distributed training code

monitor.stop()
```

---

## 6. MstxMonitor Usage Guide

Collects user-defined marker data and supports both instant marker and range marker modes.

### 6.1 Basic Usage

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

# Run the service code

monitor.stop()
```

### 6.2 Complete Example of Integration with PyTorch MSTX

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

    # Create and start the MstxMonitor
    monitor = MstxMonitor()
    monitor.start(range_parser)

    # Run NPU computation with markers
    device = int(os.getenv('LOCAL_RANK', '0'))
    torch.npu.set_device(device)

    x = torch.randn(256, 256, dtype=torch.float16).npu()
    y = torch.randn(256, 256, dtype=torch.float16).npu()

    stream = torch_npu.npu.current_stream()
    range_id = torch_npu.npu.mstx.range_start("matmul_range", stream)
    z = torch.matmul(x, y)
    torch_npu.npu.mstx.range_end(range_id)

    torch.npu.synchronize()

    # Stop collection
    monitor.stop()
    data_queue.put(None)
    consumer_thread.join()

if __name__ == "__main__":
    test()
```

### 6.3 Domain Control

MstxMonitor supports dynamically enabling and disabling marker collection by domain:

```python
# Create a domain (through the MSTX API)
domain_name = "my_domain"

# Disable collection for the specified domain
monitor.disable_domain(domain_name)

# Re-enable collection for the specified domain
monitor.enable_domain(domain_name)
```

---

## 7. Advanced Usage

### 7.1 Multithreaded Consumer Mode

In high-throughput scenarios, you are advised to use a dedicated thread to consume callback data and avoid blocking the collection callback:

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
        # Process the data
        process_kernel_data(data)

# Start the consumer thread
consumer_thread = threading.Thread(target=consumer)
consumer_thread.start()

# Start collection
monitor = KernelMonitor()
monitor.start(kernel_parser)
# ... Run the service code ...
monitor.stop()

# Notify the consumer to exit
data_queue.put(None)
consumer_thread.join()
```

### 7.2 Using Multiple Monitors at the Same Time

Multiple Monitors can run at the same time without interfering with each other:

```python
from mspti import KernelMonitor, CommunicationMonitor

kernel_mon = KernelMonitor()
comm_mon = CommunicationMonitor()

kernel_mon.start(kernel_callback)
comm_mon.start(comm_callback)

# Run the service code

kernel_mon.stop()
comm_mon.stop()
```

### 7.3 Setting the Buffer Size

Adjust the buffer size before starting collection:

```python
monitor = KernelMonitor()
monitor.set_buffer_size(64)  # Set a 64 MB buffer
monitor.start(callback)
```

The maximum buffer size is 256 MB, and the default value is determined by the C extension.

---

## 8. How to Run

### 8.1 Environment Requirements

- Python 3.8+
- CANN software (including the msPTI Python package)
- PyTorch + torch_npu (required for NPU computation)
- Set the `LD_PRELOAD` environment variable:

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
```

### 8.2 Running on a Single Device

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
python your_script.py
```

### 8.3 Distributed Running on Multiple Devices

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
torchrun --nproc_per_node=8 your_script.py
```

---

## 9. Complete Sample Reference

| Sample | Description |
| --- | --- |
| `samples/python_monitor/` | Basic usage of KernelMonitor + CommunicationMonitor |
| `samples/python_mstx_monitor/` | Custom marker usage of MstxMonitor |
