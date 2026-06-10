# msptiCallbackIdRuntime<a name="ZH-CN_TOPIC_0000002049424597"></a>

msptiCallbackIdRuntime is the enumeration class invoked by [msptiEnableCallback](msptiEnableCallback.md). Only the functions defined in the enumeration class can be traced by the callback API. These enumerations are globally unique and are defined as follows:

```cpp
typedef enum {
    MSPTI_CBID_RUNTIME_DEVICE_SET = 1, // Indicates that the tracing function is aclrtSetDevice.
    MSPTI_CBID_RUNTIME_DEVICE_RESET = 2, // Indicates that the tracing function is aclrtSetDevice.
    // Other enumerated values are similar to the preceding example. Details are not described here.
} msptiCallbackIdRuntime;
```
