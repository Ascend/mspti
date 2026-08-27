# msptiCallbackDomain<a name="ZH-CN_TOPIC_0000002049212081"></a>

msptiCallbackDomain is the callback domain enumeration class invoked by [msptiEnableCallback](msptiEnableCallback.md), [msptiEnableDomain](msptiEnableDomain.md), and [msptiCallbackFunc](msptiCallbackFunc.md).

Each enumerated value represents a set of callback points for related API functions or CANN driver activities. The definition is as follows:

```cpp
typedef enum {
    MSPTI_CB_DOMAIN_INVALID = 0, // Invalid value
    MSPTI_CB_DOMAIN_RUNTIME = 1, // Callback points related to Runtime APIs
    MSPTI_CB_DOMAIN_HCCL = 2, // Callback points related to communication APIs.
    MSPTI_CB_DOMAIN_SIZE,
    MSPTI_CB_DOMAIN_FORCE_INT = 0x7fffffff
} msptiCallbackDomain;
```
