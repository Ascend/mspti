# msptiPmSamplingSampleEntry<a name="ZH-CN_TOPIC_0000002100000208"></a>

msptiPmSamplingSampleEntry为PM采样样本的自描述值条目，通过[msptiPmSamplingGetSampleEntries](msptiPmSamplingGetSampleEntries.md)获取。

一个样本由一组条目组成。每个配置的指标按上报的硬件实例逐条产出（如HCCS逐链路0~15）；无实例概念的指标（功耗、温度、HBM带宽利用率等）instanceId固定为0；某指标本窗口无数据时产出一条{metricId, 0, 0.0}。定义如下：

```cpp
typedef struct {
    uint32_t metricId;   // 指标ID，取值为msptiPmSamplingMetricId的枚举值
    uint32_t instanceId;   // 硬件实例ID，如HCCS链路号0~15；指标无实例概念时为0
    double value;   // 指标值
} msptiPmSamplingSampleEntry;
```
