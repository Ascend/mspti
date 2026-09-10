# MsptiActivityFlag<a name="ZH-CN_TOPIC_0000002154732225"></a>

Activity record flag. Flags can be combined by bitwise OR to correlate multiple flags with activity records. Each flag is correlated with a specific activity record.

The MsptiActivityFlag is an enumeration class called in the [MarkerData](MarkerData.md) structure. It is defined as follows:

```python
class MsptiActivityFlag(Enum):
    MSPTI_ACTIVITY_FLAG_NONE = 0   # Indicates that there is no activity flag for the activity record
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS = 1 << 0 # When the mstxMarkA interface is called with stream set to nullptr, the Host side marks an instantaneous event, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_START = 1 << 1 # When the mstxRangeStartA interface is called with stream set to nullptr, the Host side marks the start of a range, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_END = 1 << 2 # The ID passed to mstxRangeEnd comes from mstxRangeStartA with stream set to nullptr, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE = 1 << 3 # The marker data type when the mstxMarkA interface is called with a valid stream, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE = 1 << 4 # The marker data type when the mstxRangeStartA interface is called with a valid stream, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE = 1 << 5 # The ID passed to mstxRangeEnd comes from mstxRangeStartA when a valid stream is passed, used by MSPTI_ACTIVITY_KIND_MARKER
```
