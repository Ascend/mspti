# msptiActivityApi<a name="ZH-CN_TOPIC_0000002045864213"></a>

msptiActivityApi is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_API](msptiActivityKind.md), defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;   // Activity record type MSPTI_ACTIVITY_KIND_API
    uint64_t start;   // Start timestamp of API execution, in ns. If both the start and end timestamps are 0, the timestamp information of the API cannot be collected
    uint64_t end;   // End timestamp of API execution, in ns. If both the start and end timestamps are 0, the timestamp information of the API cannot be collected
    struct {
        uint32_t processId;   // Process ID of the device where the API runs
        uint32_t threadId;   // Thread ID of the API running stream
    } pt;
    uint64_t correlationId;   // Correlation ID of the API. Each API execution is assigned a unique correlation ID, which is the same as the correlation ID of the driver or runtime API Activity Record that initiated the API
    const char* name;   // API name, which remains consistent across the entire Activity Record and is not recommended to be changed
} msptiActivityApi;
```
