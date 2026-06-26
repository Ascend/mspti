# msptiActivityExternalCorrelation<a name="ZH-CN_TOPIC_0000002122003924"></a>

msptiActivityExternalCorrelation is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_EXTERNAL\_CORRELATION](msptiActivityKind.md), used to correlate activity records. It is defined as follows:

```cpp
typedef struct {
    msptiActivityKind kind;   // Activity Record type MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
    msptiExternalCorrelationKind externalKind;   // Records the type of the associated external API
    uint64_t externalId;   // Correlation ID for associating with an external API
    uint64_t correlationId;   // Correlation ID for associating with a CANN API
} msptiActivityExternalCorrelation;
```
