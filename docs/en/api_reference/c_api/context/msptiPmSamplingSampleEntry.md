# msptiPmSamplingSampleEntry<a name="ZH-CN_TOPIC_0000002100000208"></a>

msptiPmSamplingSampleEntry is a self-describing value entry of a PM sampling sample, obtained by [msptiPmSamplingGetSampleEntries](msptiPmSamplingGetSampleEntries.md).

A sample consists of a set of entries. Each configured metric produces one entry per reported hardware instance (for example, HCCS per link 0–15); metrics without the instance concept (power consumption, temperature, HBM bandwidth utilization, etc.) have `instanceId` fixed to **0**; when a metric has no data in the current window, one entry of {metricId, 0, 0.0} is produced. The definition is as follows:

```cpp
typedef struct {
    uint32_t metricId;   // Metric ID. Set it to an enumerated value of msptiPmSamplingMetricId
    uint32_t instanceId;   // Hardware instance ID, for example, HCCS link number 0–15; 0 for metrics without the instance concept
    double value;   // Metric value
} msptiPmSamplingSampleEntry;
```
