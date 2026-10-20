# msptiPmSamplingMetricProperties<a name="ZH-CN_TOPIC_0000002100000207"></a>

msptiPmSamplingMetricProperties为PM采样指标的静态属性结构体，通过[msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md)查询，用于展示时将[msptiPmSamplingMetricId](msptiPmSamplingMetricId.md)翻译为可读的名字与单位。定义如下：

```cpp
typedef struct {
    char shortName[64];   // 指标短名，如"hccs_tx_bw"
    msptiPmSamplingMetricGroup metricGroup;   // 指标所属分组
    msptiPmSamplingMetricDataType dataType;   // 指标值的数据类型
    char unit[32];   // 指标值单位，如"bytes/s"、"%"
    bool available;   // 该指标在当前芯片型号上是否可采集
} msptiPmSamplingMetricProperties;
```
