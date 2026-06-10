# msptiCallbackIdHccl<a name="ZH-CN_TOPIC_0000002049465541"></a>

msptiCallbackIdHccl is the enumeration class invoked by [msptiEnableCallback](msptiEnableCallback.md). Only the functions defined in the enumeration class can be traced by the callback API. These enumerations are globally unique and are defined as follows:

```cpp
typedef enum {
    MSPTI_CBID_HCCL_ALLREDUCE = 1,// Indicates that the function to be traced is HccAllReduce.
    MSPTI_CBID_HCCL_BROADCAST = 2,// Indicates that the function to be traced is HccBoardCast.
    // Other enumerated values are similar to the preceding example. Details are not described here.
} msptiCallbackIdHccl;
```
