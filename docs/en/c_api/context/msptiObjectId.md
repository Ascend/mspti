# msptiObjectId<a name="ZH-CN_TOPIC_0000002012690846"></a>

msptiObjectId is used in [msptiActivityMarker](msptiActivityMarker.md) calls to identify the process ID, thread ID, Device ID, and Stream ID of the Marker. The definition is as follows:

```cpp
typedef union PACKED_ALIGNMENT {
    struct {
        uint32_t processId;   // Process ID of the ActivityMarker
        uint32_t threadId;   // Thread ID of the ActivityMarker
    } pt;
    struct {
        uint32_t deviceId;   // Device ID of the device where the ActivityMarker process resides
        uint32_t streamId;   // Stream ID of the stream where the ActivityMarker process resides
    } ds;
} msptiObjectId;
```
