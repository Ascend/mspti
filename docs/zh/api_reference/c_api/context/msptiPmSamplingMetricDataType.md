# msptiPmSamplingMetricDataType<a name="ZH-CN_TOPIC_0000002100000205"></a>

msptiPmSamplingMetricDataType为PM采样指标值的数据类型枚举，通过[msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md)查询。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO = 0,   // 无量纲比值
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT,   // 百分比
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT,   // 吞吐量，bytes/s
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER,   // 功耗，W
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE,   // 温度，摄氏度
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY,   // 频率，MHz
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_COUNT,
    MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricDataType;
```
