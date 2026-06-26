# msptiApiCallbackSite<a name="ZH-CN_TOPIC_0000002013131536"></a>

msptiApiCallbackSite is an enumeration class for [msptiCallbackData](msptiCallbackData.md) calls.

Specifies the point in an API call where a callback is issued. Defined as follows:

```cpp
typedef enum {
    MSPTI_API_ENTER = 0,    // Callback on API entry
    MSPTI_API_EXIT = 1,    // Callback after API exit
    MSPTI_API_CBSITE_FORCE_INT = 0x7fffffff
} msptiApiCallbackSite;
```
