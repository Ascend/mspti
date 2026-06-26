# HcclData<a name="ZH-CN_TOPIC_0000002119450412"></a>

HcclData is the structure called by [HcclMonitor.start](HcclMonitor-start.md), defined as follows:

```python
class HcclData:
    self.kind   # Activity Record type MSPTI_ACTIVITY_KIND_HCCL
    self.start   # Start timestamp of the communication operator execution on the NPU device, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    self.end   # End timestamp of the communication operator execution, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    self.device_id   # Device ID of the device where the communication operator runs
    self.stream_id   # Stream ID of the stream where the communication operator runs
    self.bandwidth   # Bandwidth of the communication operator during execution, in GB/s
    self.name   # Name of the communication operator
    self.comm_name   # Name of the communication domain
```
