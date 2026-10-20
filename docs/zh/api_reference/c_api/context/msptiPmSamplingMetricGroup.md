# msptiPmSamplingMetricGroup<a name="ZH-CN_TOPIC_0000002100000206"></a>

msptiPmSamplingMetricGroup为PM采样指标所属分组的枚举，通过[msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md)查询，用于展示归类。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY = 0,   // 内存类（HBM带宽利用率）
    MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK,   // 互连类（HCCS/PCIe/RoCE带宽）
    MSPTI_PM_SAMPLING_METRIC_GROUP_POWER,   // 功耗类
    MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL,   // 温度类
    MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION,   // 利用率类
    MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY,   // 频率类
    MSPTI_PM_SAMPLING_METRIC_GROUP_COUNT,
    MSPTI_PM_SAMPLING_METRIC_GROUP_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricGroup;
```
