# msptiActivityMemory<a name="ZH-CN_TOPIC_0000002120186622"></a>

msptiActivityMemory is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_MEMORY](msptiActivityKind.md), used to report memory activity information. It is defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record type MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType;    // User request (allocate or free) memory operation
    msptiActivityMemoryKind memoryKind;    // Requested memory type
    uint64_t correlationId;    // Correlation ID of the memory request operation. Each memory request operation is assigned a unique correlation ID
    uint64_t start;    // Start timestamp of the memory request operation, in ns
    uint64_t end;    // End timestamp of the memory request operation, in ns
    uint64_t address;    // Requested memory address
    uint64_t bytes;    // Number of bytes of memory requested by the memory request operation
    uint32_t processId;    // Process ID to which the memory request operation belongs
    uint32_t deviceId;    // Device ID where the memory request operation resides
    uint32_t streamId;    // Stream ID of the memory request operation. If the memory request operation is asynchronous, the stream ID is set to MSPTI_INVALID_STREAM_ID
} msptiActivityMemory;
```
