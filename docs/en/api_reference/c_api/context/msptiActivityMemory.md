# msptiActivityMemory<a name="ZH-CN_TOPIC_0000002120186622"></a>

msptiActivityMemory is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_MEMORY](msptiActivityKind.md). It is used to report memory activity information. The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType; // Memory operation requested by the user (memory allocation or release)
    msptiActivityMemoryKind memoryKind; // Type of the requested memory
    uint64_t correlationId; // Correlation ID of the memory request operation. Each memory request operation is assigned a unique correlation ID.
    uint64_t start; // Start timestamp of the memory request operation, in ns.
    uint64_t end; // End timestamp of the memory request operation, in ns.
    uint64_t address; // Requested memory address
    uint64_t bytes; // Number of bytes requested by the memory request operation
    uint32_t processId; // ID of the process to which the memory request operation belongs
    uint32_t deviceId; // ID of the device where the memory request operation is performed
    uint32_t streamId; // Stream ID of the memory request operation. If the memory request operation is asynchronous, the stream ID is set to MSPTI_INVALID_STREAM_ID.
} msptiActivityMemory;
```
