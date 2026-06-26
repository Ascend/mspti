# msptiActivityFlag<a name="ZH-CN_TOPIC_0000002045928149"></a>

Activity flags for activity records. Flags can be combined using bitwise XOR to associate multiple flags with an activity record. Each flag is specific to a particular Activity Record.

msptiActivityFlag is an enumeration class called within the [msptiActivityMarker](./msptiActivityMarker.md) struct, defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_FLAG_NONE = 0,   // Indicates that there is no activity flag for the Activity Record
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS = 1 << 0, // When calling the mstxMarkA interface with stream set to nullptr, it marks an instantaneous event on the host side, used by MSPTI_ACTIVITY_KIND_MARKER
    MSPTI_ACTIVITY_FLAG_MARKER_START = 1 << 1, // When calling the mstxRangeStartA API with the stream parameter set to nullptr, it indicates the start of a marker on the host side, and MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_END = 1 << 2, // When calling mstxRangeEnd with an ID that originates from an mstxRangeStartA call where the stream parameter was set to nullptr, MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE = 1 << 3, // When calling the mstxMarkA API with a valid stream, this corresponds to the marker data type, and MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE = 1 << 4, // When calling the mstxRangeStartA API with a valid stream, this corresponds to the marker data type, and MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE = 1 << 5 // When calling mstxRangeEnd with an ID that originates from an mstxRangeStartA call where a valid stream was passed, MSPTI_ACTIVITY_KIND_MARKER is used
} msptiActivityFlag;
```
