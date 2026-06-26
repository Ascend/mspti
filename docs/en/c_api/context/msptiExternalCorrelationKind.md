# msptiExternalCorrelationKind<a name="ZH-CN_TOPIC_0000002157365565"></a>

Types of external APIs that support correlation.

msptiExternalCorrelationKind is an enumeration class called by [msptiActivityPushExternalCorrelationId](msptiActivityPushExternalCorrelationId.md) and [msptiActivityExternalCorrelation](msptiActivityExternalCorrelation.md), defined as follows:

```cpp
typedef enum {
    MSPTI_EXTERNAL_CORRELATION_KIND_INVALID = 0,   // Invalid value
    MSPTI_EXTERNAL_CORRELATION_KIND_UNKNOWN = 1,   // MSPTI unknown external API
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0 = 2,   // The external API is CUSTOM0
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM1 = 3,   // The external API is CUSTOM1
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM2 = 4,   // The external API is CUSTOM2
    MSPTI_EXTERNAL_CORRELATION_KIND_SIZE,   // Add new types before this line
    MSPTI_EXTERNAL_CORRELATION_KIND_FORCE_INT = 0x7fffffff,
} msptiExternalCorrelationKind;
```
