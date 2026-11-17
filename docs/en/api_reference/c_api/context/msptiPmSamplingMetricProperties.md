# msptiPmSamplingMetricProperties<a name="ZH-CN_TOPIC_0000002100000207"></a>

msptiPmSamplingMetricProperties is a structure of the static properties of a PM sampling metric, queried by [msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md). It translates a [msptiPmSamplingMetricId](msptiPmSamplingMetricId.md) into a readable name and unit for display. The definition is as follows:

```cpp
typedef struct {
    char shortName[64];   // Metric short name, for example, "hccs_tx_bw"
    msptiPmSamplingMetricGroup metricGroup;   // Group that the metric belongs to
    msptiPmSamplingMetricDataType dataType;   // Data type of the metric value
    char unit[32];   // Unit of the metric value, for example, "bytes/s" or "%"
    bool available;   // Whether the metric can be collected on the current chip model
} msptiPmSamplingMetricProperties;
```
