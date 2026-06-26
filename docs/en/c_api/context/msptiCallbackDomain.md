# msptiCallbackDomain<a name="ZH-CN_TOPIC_0000002049212081"></a>

msptiCallbackDomain is the callback domain enumeration class for [msptiEnableCallback](msptiEnableCallback.md), [msptiEnableDomain](msptiEnableDomain.md), and [msptiCallbackFunc](msptiCallbackFunc.md) calls.

Each enumeration value represents a set of related API functions or callback points for CANN driver activities. The definitions are as follows:

```cpp
typedef enum {
    MSPTI_CB_DOMAIN_INVALID = 0,    // Invalid value
    MSPTI_CB_DOMAIN_RUNTIME = 1,    // Runtime API related callback points
    MSPTI_CB_DOMAIN_HCCL = 2,    // Communication API-related callback points
    MSPTI_CB_DOMAIN_SIZE,
    MSPTI_CB_DOMAIN_FORCE_INT = 0x7fffffff
} msptiCallbackDomain;
```
