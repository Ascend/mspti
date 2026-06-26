# msptiActivityMemoryKind<a name="ZH-CN_TOPIC_0000002155584861"></a>

Requested memory type. msptiActivityMemoryKind is an enumeration class called by [msptiActivityMemory](./msptiActivityMemory.md), defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMORY_UNKNOWN = 0,    // Internally reserved, undefined
    MSPTI_ACTIVITY_MEMORY_DEVICE = 1,    // Device memory
} msptiActivityMemoryKind;
```
