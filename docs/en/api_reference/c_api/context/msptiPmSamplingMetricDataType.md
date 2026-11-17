# msptiPmSamplingMetricDataType<a name="ZH-CN_TOPIC_0000002100000205"></a>

msptiPmSamplingMetricDataType is an enumeration of the data types of PM sampling metric values, queried by [msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md). The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO = 0,   // Dimensionless ratio
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT,   // Percentage
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT,   // Throughput, in bytes/s
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER,   // Power consumption, in watts
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE,   // Temperature, in degrees Celsius
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY,   // Frequency, in MHz
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_COUNT,
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricDataType;
```
