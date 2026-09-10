# msptiActivityKind<a name="ZH-CN_TOPIC_0000002045503493"></a>

msptiActivityKind is the enumeration class invoked by [msptiActivityEnable](msptiActivityEnable.md) and [msptiActivityDisable](msptiActivityDisable.md).

The MSPTI uses **msptiActivityKind** to classify all activity data that can be sampled. Each enumerated value corresponds to a structure type of activity data. The definition is as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_KIND_INVALID = 0, // Invalid value
    MSPTI_ACTIVITY_KIND_MARKER = 1, // Activity record type of the MSPTI dotting capability (marking the instantaneous moment). The maximum number of dots is the maximum value of uint32_t. The msptiActivityMarker structure is invoked.
    MSPTI_ACTIVITY_KIND_KERNEL = 2, // Activity record type for collecting information about computing operators in the aclnn scenario. The msptiActivityKernel structure is invoked.
    MSPTI_ACTIVITY_KIND_API = 3, // Activity record type for collecting information about the aclnn component in the aclnn scenario. The msptiActivityApi structure is invoked.
    MSPTI_ACTIVITY_KIND_HCCL = 4, // Activity record type for collecting information about the HCCL communication operator. The msptiActivityHccl structure is invoked.
    MSPTI_ACTIVITY_KIND_MEMORY = 5, // Memory request (allocation or release). The msptiActivityMemory structure is invoked.
    MSPTI_ACTIVITY_KIND_MEMSET = 6, // Memory setting. The msptiActivityMemset structure is invoked.
    MSPTI_ACTIVITY_KIND_MEMCPY = 7, // Memory copy. The msptiActivityMemcpy structure is invoked.
    MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION = 8, // Indicates the correlation between different programming APIs. The msptiActivityExternalCorrelation struct is called.
    MSPTI_ACTIVITY_KIND_COMMUNICATION = 9, // Indicates the activity record type of the HCCL and LCCL communication operators. The msptiActivityCommunication struct is called.
    MSPTI_ACTIVITY_KIND_ACL_API = 10, // Indicates the AscendCL API, which is a C language API library used to develop deep neural network applications on the Ascend platform. The msptiActivityApi struct is called.
    MSPTI_ACTIVITY_KIND_NODE_API = 11, // Indicates the operator delivery API at the CANN layer. The msptiActivityApi struct is called.
    MSPTI_ACTIVITY_KIND_RUNTIME_API = 12, // Indicates the CANN runtime API. The msptiActivityApi struct is called.
    MSPTI_ACTIVITY_KIND_COUNT,
    MSPTI_ACTIVITY_KIND_FORCE_INT = 0x7fffffff
} msptiActivityKind;
```
