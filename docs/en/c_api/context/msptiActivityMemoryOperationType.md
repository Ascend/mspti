# msptiActivityMemoryOperationType<a name="ZH-CN_TOPIC_0000002155706477"></a>

Memory operation type. msptiActivityMemoryOperationType is an enumeration class for the [msptiActivityMemory](./msptiActivityMemory.md) call, defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_ALLOCATION = 0,    // Allocate memory
    MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_RELEASE = 1    // Free memory
} msptiActivityMemoryOperationType;
```
