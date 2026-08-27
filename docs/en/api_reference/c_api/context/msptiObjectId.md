# msptiObjectId<a name="ZH-CN_TOPIC_0000002012690846"></a>

msptiObjectId is called by [msptiActivityMarker](msptiActivityMarker.md) to identify the process ID, thread ID, device ID, and stream ID of a marker. The definition is as follows:

```cpp
typedef union PACKED_ALIGNMENT {
    struct {
        uint32_t processId; // Process ID of the activity marker
        uint32_t threadId; // Thread ID of the activity marker
    } pt;
    struct {
        uint32_t deviceId; // Device ID of the device where the activity marker process is located
        uint32_t streamId; // Stream ID of the stream where the activity marker process is located.
    } ds;
} msptiObjectId;
```
