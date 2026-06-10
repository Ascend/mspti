# msptiCallbackData<a name="ZH-CN_TOPIC_0000002014599005"></a>

msptiCallbackData is the structure corresponding to the **cbdata** of msptiCallbackFunc, which is used to specify the data to be transferred to the callback function.

The definition is as follows:

```cpp
typedef struct {
    msptiApiCallbackSite callbackSite; // Location of the callback trigger point (start or end)
    const char *functionName; // Name of the current function
    const void *functionParams; // Parameters of the current function
    const void *functionReturnValue; // Pointer to the return value of the Runtime or driver API
    const char *symbolName; // Name of the symbol operated by the current function
    uint64_t correlationId; // This ID can be used to associate msptiCallbackData with activity records. In the scenario where the Runtime callback function is called, the activity records are msptiActivityApi data. This ID is the same as the correlationId in the msptiActivityApi data that records the Runtime function call and can be used for data association.
    uint64_t reserved1; // Reserved for internal use. It is not defined.
    uint64_t reserved2; // Reserved for internal use. It is not defined.
    uint64_t *correlationData; // Provides a pointer to share data at the entry and exit of the Runtime or driver API. This field can be used to transfer 64-bit data from the entry callback function to the exit callback function.
} msptiCallbackData;
```
