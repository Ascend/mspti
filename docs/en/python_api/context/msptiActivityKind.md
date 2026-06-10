# msptiActivityKind<a name="ZH-CN_TOPIC_0000002154810617"></a>

msptiActivityKind is an enumeration class invoked by [HcclData](HcclData.md), [KernelData](KernelData.md), [MarkerData](MarkerData.md), and [RangeMarkerData](RangeMarkerData.md).

MSPTI uses **msptiActivityKind** to classify all data that can be profiled. Each enumerated value corresponds to a data structure type. The definition is as follows:

```python
class MsptiActivityKind(Enum):
    MSPTI_ACTIVITY_KIND_INVALID = 0 # Invalid value
    MSPTI_ACTIVITY_KIND_MARKER = 1 # Activity record type of the MSPTI dotting capability (marking the instantaneous moment). The maximum number of dots supported is the maximum value of uint32_t. The returned structure is MarkerData or RangeMarkerData.
    MSPTI_ACTIVITY_KIND_KERNEL = 2 # Activity record type for collecting information about computing operators in the aclnn scenario. The returned structure is KernelData.
    MSPTI_ACTIVITY_KIND_API = 3 # Reserved parameter, which is not open to the public.
    MSPTI_ACTIVITY_KIND_HCCL = 4 # Activity record type for collecting communication operators. The returned structure is HcclData.
    MSPTI_ACTIVITY_KIND_COUNT
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
```
