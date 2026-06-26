# msptiCallbackIdRuntime<a name="ZH-CN_TOPIC_0000002049424597"></a>

msptiCallbackIdRuntime is an enumeration class used by [msptiEnableCallback](msptiEnableCallback.md). Only functions defined in the enumeration class can be traced by the callback API. These enumerations are globally unique and are defined as follows:

```cpp
typedef enum {
    MSPTI_CBID_RUNTIME_DEVICE_SET = 1, //Indicates that the traced function is aclrtSetDevice
    MSPTI_CBID_RUNTIME_DEVICE_RESET = 2, //Indicates that the traced function is aclrtResetDevice
    //Other enumeration values are similar to the above examples and are not described further
} msptiCallbackIdRuntime;
```
