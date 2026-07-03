# CommunicationData<a name="ZH-CN_TOPIC_0000002119450412"></a>

CommunicationData为[CommunicationMonitor.start](CommunicationMonitor-start.md)调用的结构体，定义如下：

```python
class CommunicationData:
    self.kind   # Activity Record类型MSPTI_ACTIVITY_KIND_COMMUNICATION
    self.start   # 通信算子在NPU设备上执行开始时间戳，单位ns。开始和结束时间戳均为0时则无法收集通信算子的时间戳信息
    self.end   # 通信算子执行的结束时间戳，单位ns。开始和结束时间戳均为0时则无法收集通信算子的时间戳信息
    self.device_id   # 通信算子运行设备的Device ID
    self.stream_id   # 通信算子运行流的Stream ID
    self.name   # 通信算子的名称
    self.comm_name   # 通信域的名称
    self.data_type : msptiCommunicationDataType   # 通信算子传输的数据类型
    self.count   # 通信算子传输的数据量
    self.alg_type   # 通信算子使用的算法类型
    self.correlation_id   # Runtime在Launch Kernel时生成的唯一ID，其他Activity可通过该值与此算子进行关联
```
