# msptiActivityMemoryKind<a name="ZH-CN_TOPIC_0000002155584861"></a>

Requested memory type. msptiActivityMemoryKind is an enumeration class invoked by [msptiActivityMemory](./msptiActivityMemory.md). It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMORY_UNKNOWN = 0, // Reserved for internal use. Undefined.
    MSPTI_ACTIVITY_MEMORY_DEVICE = 1, // Device memory.
} msptiActivityMemoryKind;
```
