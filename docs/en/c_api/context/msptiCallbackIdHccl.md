# msptiCallbackIdHccl<a name="ZH-CN_TOPIC_0000002049465541"></a>

msptiCallbackIdHccl is an enumeration class used by [msptiEnableCallback](msptiEnableCallback.md). Only functions defined in this enumeration class can be traced by the callback API. These enumerations are globally unique and defined as follows:

```cpp
typedef enum {
    MSPTI_CBID_HCCL_ALLREDUCE = 1,//Indicates that the traced function is HcclAllReduce
    MSPTI_CBID_HCCL_BROADCAST = 2,//Indicates that the traced function is HcclBroadcast
    //Other enumeration values are similar to the examples above and are not described further
} msptiCallbackIdHccl;
```
