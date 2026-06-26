# MsptiActivityKind<a name="ZH-CN_TOPIC_0000002154810617"></a>

MsptiActivityKind is an enumeration class called by [HcclData](HcclData.md), [KernelData](KernelData.md), [MarkerData](MarkerData.md), and [RangeMarkerData](RangeMarkerData.md).

MSPTI classifies all profiled data through MsptiActivityKind, with each enumeration value corresponding to a data structure type. The definition is as follows:

```python
class MsptiActivityKind(Enum):
    MSPTI_ACTIVITY_KIND_INVALID = 0   # Invalid value
    MSPTI_ACTIVITY_KIND_MARKER = 1   # Activity Record type for MSPTI marking capability (marking an instantaneous moment), supporting a maximum number of marks equal to the maximum value of uint32_t, returning the structure MarkerData or RangeMarkerData
    MSPTI_ACTIVITY_KIND_KERNEL = 2   # In the aclnn scenario, the Activity Record type for collecting compute operator information returns the KernelData structure
    MSPTI_ACTIVITY_KIND_API = 3   # Reserved parameter, not yet available
    MSPTI_ACTIVITY_KIND_HCCL = 4   # The Activity Record type for collecting communication operators returns the HcclData structure
    MSPTI_ACTIVITY_KIND_COUNT
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
```
