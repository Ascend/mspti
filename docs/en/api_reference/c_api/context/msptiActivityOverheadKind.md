# `msptiActivityOverheadKind`<a name="ZH-CN_TOPIC_0000002155584861"></a>

Overhead type. msptiActivityOverheadKind is an enumeration class used by [msptiActivityOverhead](./msptiActivityOverhead.md). It identifies the phase in which the msPTI overhead is generated. The definition is as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_OVERHEAD_UNKNOWN = 0,    // Unknown overhead type
    MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE = 1,    // Overhead of creating and destroying msPTI internal resources
    MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST = 2,    // Overhead of requesting an Activity Buffer
    MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH = 3,    // Overhead of flushing an Activity Buffer
} msptiActivityOverheadKind;
```
