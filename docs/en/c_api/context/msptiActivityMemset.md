# msptiActivityMemset<a name="ZH-CN_TOPIC_0000002155706481"></a>

msptiActivityMemset is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_MEMSET](msptiActivityKind.md), used to report Memset activity information. It is defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record type MSPTI_ACTIVITY_KIND_MEMSET
    uint32_t value;    // Target value set by Memset
    uint64_t bytes;    // Number of bytes set by Memset
    uint64_t start;    // Start timestamp of the Memset operation, in ns
    uint64_t end;    // End timestamp of the Memset operation, in ns
    uint32_t deviceId;    // Device ID where the Memset operation is located
    uint32_t streamId;    // Stream ID of the Memset operation
    uint64_t correlationId;    // Correlation ID of the Memset operation
    uint8_t isAsync;    // Whether the memory set operation is performed through the asynchronous memory API
} msptiActivityMemset;
```
