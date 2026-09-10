# msptiActivityApi<a name="ZH-CN_TOPIC_0000002045864213"></a>

msptiActivityApi is the structure corresponding to the Activity Record type MSPTI\_ACTIVITY\_KIND\_API (msptiActivityKind.md). The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_API
    uint64_t start; // Start timestamp of API execution, in ns. If both the start and end timestamps are 0, the API timestamp information cannot be collected.
    uint64_t end; // End timestamp of API execution, in ns. If both the start and end timestamps are 0, the API timestamp information cannot be collected.
    struct {
        uint32_t processId; // Process ID of the device where the API is running.
        uint32_t threadId; // Thread ID of the stream where the API is running.
    } pt;
    uint64_t correlationId; // Correlation ID of the API. Each API execution is assigned a unique correlation ID, which is the same as the correlation ID of the driver that starts the API or the runtime API Activity Record.
    const char* name; // Name of the API, which is consistent in the entire Activity Record. You are advised not to change it.
} msptiActivityApi;
```
