# msptiActivityExternalCorrelation<a name="ZH-CN_TOPIC_0000002122003924"></a>

msptiActivityExternalCorrelation is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_EXTERNAL\_CORRELATION](msptiActivityKind.md). It is used to associate with the Activity Record. The definition is as follows:

```cpp
typedef struct {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
    msptiExternalCorrelationKind externalKind; // Type of the external API associated with the record
    uint64_t externalId; // ID of the associated external API
    uint64_t correlationId; // ID of the associated CANN API.
} msptiActivityExternalCorrelation;
```
