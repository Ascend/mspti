# msptiExternalCorrelationKind<a name="ZH-CN_TOPIC_0000002157365565"></a>

Supported types of external APIs that can be correlated.

msptiExternalCorrelationKind is an enumeration class invoked by [msptiActivityPushExternalCorrelationId](msptiActivityPushExternalCorrelationId.md) and [msptiActivityExternalCorrelation](msptiActivityExternalCorrelation.md). It is defined as follows:

```cpp
typedef enum {
    MSPTI_EXTERNAL_CORRELATION_KIND_INVALID = 0, // Invalid value.
    MSPTI_EXTERNAL_CORRELATION_KIND_UNKNOWN = 1, // Unknown external API of MSPTI.
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0 = 2, // The external API is CUSTOM0.
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM1 = 3, // The external API is CUSTOM1.
    MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM2 = 4, // The external API is CUSTOM2.
    MSPTI_EXTERNAL_CORRELATION_KIND_SIZE, // Add a new type before this line.
    MSPTI_EXTERNAL_CORRELATION_KIND_FORCE_INT = 0x7fffffff,
} msptiExternalCorrelationKind;
```
