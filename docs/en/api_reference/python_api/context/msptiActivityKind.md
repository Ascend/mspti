# MsptiActivityKind<a name="ZH-CN_TOPIC_0000002154810617"></a>

MsptiActivityKind is an enumeration class invoked by [HcclData](HcclData.md), [KernelData](KernelData.md), [MarkerData](MarkerData.md), and [RangeMarkerData](RangeMarkerData.md).

msPTI uses `MsptiActivityKind` to classify all data that can be profiled. Each enumerated value corresponds to a data structure type. The definition is as follows:

```python
class MsptiActivityKind(Enum):
    MSPTI_ACTIVITY_KIND_INVALID = 0 # Invalid value
    MSPTI_ACTIVITY_KIND_MARKER = 1 # Activity record type of the msPTI dotting capability (marking the instantaneous moment). The maximum number of dots supported is the maximum value of uint32_t. The returned structure is MarkerData or RangeMarkerData
    MSPTI_ACTIVITY_KIND_KERNEL = 2 # Activity record type for collecting information about computing operators in the aclnn scenario. The returned structure is KernelData
    MSPTI_ACTIVITY_KIND_API = 3 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_HCCL = 4 # Activity record type for collecting communication operators. The returned structure is HcclData
    MSPTI_ACTIVITY_KIND_MEMORY = 5 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_MEMSET = 6 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_MEMCPY = 7 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION = 8 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_COMMUNICATION = 9 # Activity record type for collecting communication operators. The returned structure is CommunicationData
    MSPTI_ACTIVITY_KIND_ACL_API = 10 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_NODE_API = 11 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_RUNTIME_API = 12 # Reserved parameter, which is not open to the public
    MSPTI_ACTIVITY_KIND_COUNT = 13
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
```
