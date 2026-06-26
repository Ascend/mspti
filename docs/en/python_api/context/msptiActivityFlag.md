# MsptiActivityFlag<a name="ZH-CN_TOPIC_0000002154732225"></a>

Activity flags for activity record. Flags can be combined using bitwise OR to associate multiple flags with an activity record. Each flag is specific to a particular activity record.

MsptiActivityFlag is an enumeration class called within the [MarkerData](MarkerData.md) structure, defined as follows:

```python
class MsptiActivityFlag(Enum):
    MSPTI_ACTIVITY_FLAG_NONE = 0   # Indicates an activity flag with no activity record
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS = 1 << 0 # When calling the mstxMarkA interface with stream set to nullptr, it marks an instantaneous event on the host side, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_START = 1 << 1 # When calling the mstxRangeStartA interface with stream set to nullptr, it indicates the start of a marker on the Host side, using MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_END = 1 << 2 # When calling mstxRangeEnd with an ID originating from mstxRangeStartA where stream was set to nullptr, MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE = 1 << 3 # When calling the mstxMarkA interface with a valid stream, the corresponding marker data type uses MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE = 1 << 4 # When calling the mstxRangeStartA interface with a valid stream, the corresponding marker data type uses MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE = 1 << 5 # When calling mstxRangeEnd with an ID originating from mstxRangeStartA where a valid stream was passed, MSPTI_ACTIVITY_KIND_MARKER is used
```
