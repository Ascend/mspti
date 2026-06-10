# msptiActivityMemcpy<a name="ZH-CN_TOPIC_0000002155584865"></a>

msptiActivityMemcpy is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_MEMCPY](msptiActivityKind.md). It is used to report the memcpy activity information. The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind; // Type of the memory copy operation
    uint64_t bytes; // Number of bytes transferred by the memory copy operation
    uint64_t start; // Start timestamp of the memory copy operation, in ns
    uint64_t end; // End timestamp of the memory copy operation, in ns
    uint32_t deviceId; // ID of the device where the memory copy operation is performed
    uint32_t streamId; // Stream ID of the memory copy operation
    uint64_t correlationId; // Correlation ID of the memory copy operation
    uint8_t isAsync; // Whether the memory copy operation is performed using asynchronous memory APIs.
} msptiActivityMemcpy;
```
