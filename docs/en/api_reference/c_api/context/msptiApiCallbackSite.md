# msptiApiCallbackSite<a name="ZH-CN_TOPIC_0000002013131536"></a>

msptiApiCallbackSite is the enumeration class invoked by [msptiCallbackData](msptiCallbackData.md).

Specifies the point where the callback is issued in the API call. The definition is as follows:

```cpp
typedef enum {
    MSPTI_API_ENTER = 0, // Callback is performed when the API is entered.
    MSPTI_API_EXIT = 1, // Callback is performed after the API is exited.
    MSPTI_API_CBSITE_FORCE_INT = 0x7fffffff
} msptiApiCallbackSite;
```
