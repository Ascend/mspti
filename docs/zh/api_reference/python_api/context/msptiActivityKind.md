# MsptiActivityKind<a name="ZH-CN_TOPIC_0000002154810617"></a>

MsptiActivityKind为[HcclData](HcclData.md)、[KernelData](KernelData.md)、[MarkerData](MarkerData.md)和[RangeMarkerData](RangeMarkerData.md)调用的枚举类。

MSPTI通过MsptiActivityKind对所有能采集到的数据进行分类，每个枚举值对应一个数据的结构体类型。定义如下：

```python
class MsptiActivityKind(Enum):
    MSPTI_ACTIVITY_KIND_INVALID = 0   # 非法值
    MSPTI_ACTIVITY_KIND_MARKER = 1   # MSPTI打点能力（标记瞬时时刻）的Activity Record类型，支持最大打点个数为uint32_t最大值，返回结构体MarkerData或RangeMarkerData
    MSPTI_ACTIVITY_KIND_KERNEL = 2   # aclnn场景下，计算类算子信息采集的Activity Record类型，返回结构体KernelData
    MSPTI_ACTIVITY_KIND_API = 3   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_HCCL = 4   # 通信算子采集Activity Record类型，返回结构体HcclData
    MSPTI_ACTIVITY_KIND_MEMORY = 5   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_MEMSET = 6   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_MEMCPY = 7   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION = 8   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_COMMUNICATION = 9   # 通信算子采集Activity Record类型，返回结构体CommunicationData
    MSPTI_ACTIVITY_KIND_ACL_API = 10   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_NODE_API = 11   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_RUNTIME_API = 12   # 预留参数，暂未开放
    MSPTI_ACTIVITY_KIND_COUNT = 13
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
```
