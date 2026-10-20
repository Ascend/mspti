# msptiPmSamplingMetricGroup<a name="ZH-CN_TOPIC_0000002100000206"></a>

msptiPmSamplingMetricGroup is an enumeration of the groups that PM sampling metrics belong to, queried by [msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md) for display classification. The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY = 0,   // Memory group (HBM bandwidth utilization)
    MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK,   // Interconnect group (HCCS/PCIe/RoCE bandwidth)
    MSPTI_PM_SAMPLING_METRIC_GROUP_POWER,   // Power group
    MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL,   // Thermal group
    MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION,   // Utilization group
    MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY,   // Frequency group
    MSPTI_PM_SAMPLING_METRIC_GROUP_COUNT,
    MSPTI_PM_SAMPLING_METRIC_GROUP_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricGroup;
```
