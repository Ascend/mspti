# msptiActivityFlag<a name="ZH-CN_TOPIC_0000002045928149"></a>

Activity record flag. Multiple flags can be combined using bitwise XOR to associate them with an activity record. Each flag is correlated with a specific activity record.

The msptiActivityFlag is an enumeration class called in the [msptiActivityMarker](./msptiActivityMarker.md) structure. It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_FLAG_NONE = 0,   // Indicates that there is no activity flag for the activity record
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS = 1 << 0, // When the mstxMarkA API is called with the stream set to nullptr, an instantaneous event is marked on the host side. MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_START = 1 << 1, // When the mstxRangeStartA API is called with the stream set to nullptr, the start of a marker is identified on the host side. MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_END = 1 << 2, // The ID passed to mstxRangeEnd comes from mstxRangeStartA called with the stream set to nullptr. MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE = 1 << 3, // The corresponding marker data type when the mstxMarkA API is called with a valid stream MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE = 1 << 4, // The corresponding marker data type when the mstxRangeStartA API is called with a valid stream MSPTI_ACTIVITY_KIND_MARKER is used
    MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE = 1 << 5 // The ID passed to mstxRangeEnd comes from mstxRangeStartA called with a valid stream. MSPTI_ACTIVITY_KIND_MARKER is used
} msptiActivityFlag;
```
