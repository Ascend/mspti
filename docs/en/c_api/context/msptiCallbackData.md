# msptiCallbackData<a name="ZH-CN_TOPIC_0000002014599005"></a>

msptiCallbackData is the structure corresponding to the **cbdata** of [msptiCallbackFunc](msptiCallbackFunc.md), used to specify the data passed to the callback function.

The definition is as follows:

```cpp
typedef struct {
    msptiApiCallbackSite callbackSite;    // Position of the callback trigger point (start or end)
    const char *functionName;    // Current function name
    const void *functionParams;    // Parameters of the current function
    const void *functionReturnValue;    // Pointer to the return value of the Runtime or Driver API
    const char *symbolName;    // Name of the symbol operated by the current function
    uint64_t correlationId;    // This ID can be used to correlate msptiCallbackData with activity records. In the runtime callback function invocation scenario, the activity record is msptiActivityApi data. This ID is the same as the correlationId in the msptiActivityApi data that records the runtime function call, and can be used for data correlation
    uint64_t reserved1;    // Internally reserved, undefined
    uint64_t reserved2;    // Internally reserved, undefined
    uint64_t *correlationData;    // Provides a pointer for sharing data between the entry and exit of a runtime or driver API. This field can be used to pass 64-bit data from the entry callback function to the exit callback function
} msptiCallbackData;
```
