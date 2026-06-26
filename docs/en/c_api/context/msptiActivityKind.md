# msptiActivityKind<a name="ZH-CN_TOPIC_0000002045503493"></a>

msptiActivityKind is an enumeration class used by [msptiActivityEnable](msptiActivityEnable.md) and [msptiActivityDisable](msptiActivityDisable.md) calls.

MSPTI uses msptiActivityKind to classify all collectible activity data. Each enumeration value corresponds to a struct type for the activity data. The definition is as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_KIND_INVALID = 0,   // Invalid value
    MSPTI_ACTIVITY_KIND_MARKER = 1,   // Activity record type for MSPTI marker capability (marking instantaneous moments). The maximum number of markers supported is the maximum value of uint32_t. Calls the struct msptiActivityMarker
    MSPTI_ACTIVITY_KIND_KERNEL = 2,   // In the aclnn scenario, the activity record type for profiling compute operator information, using the struct msptiActivityKernel
    MSPTI_ACTIVITY_KIND_API = 3,   // In the aclnn scenario, the activity record type for profiling aclnn component information, using the struct msptiActivityApi
    MSPTI_ACTIVITY_KIND_HCCL = 4,   // The activity record type for profiling HCCL communication operators, using the struct msptiActivityHccl
    MSPTI_ACTIVITY_KIND_MEMORY = 5,   // Memory request (allocation or deallocation), using the struct msptiActivityMemory
    MSPTI_ACTIVITY_KIND_MEMSET = 6,   // Memory setting, using the struct msptiActivityMemset
    MSPTI_ACTIVITY_KIND_MEMCPY = 7,   // Memory copy, calling the struct msptiActivityMemcpy
    MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION = 8,   // Correlation records between different programming APIs, calling the struct msptiActivityExternalCorrelation
    MSPTI_ACTIVITY_KIND_COMMUNICATION = 9,   // HCCL and LCCL communication operator collection Activity Record type, calling the struct msptiActivityCommunication
    MSPTI_ACTIVITY_KIND_ACL_API = 10, // AscendCL API, a C language API library for developing deep neural network applications on the Ascend platform, calling the struct msptiActivityApi
    MSPTI_ACTIVITY_KIND_NODE_API = 11, // Corresponds to the CANN layer operator dispatch API, calling the struct msptiActivityApi
    MSPTI_ACTIVITY_KIND_RUNTIME_API = 12, // CANN runtime API, calling the struct msptiActivityApi
    MSPTI_ACTIVITY_KIND_COUNT,
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
} msptiActivityKind;
```
