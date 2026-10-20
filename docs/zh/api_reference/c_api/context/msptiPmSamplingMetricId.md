# msptiPmSamplingMetricId<a name="ZH-CN_TOPIC_0000002100000201"></a>

msptiPmSamplingMetricId为PM采样静态指标表的指标ID，用于[msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md)、[msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md)等接口的指标配置与查询。

指标的采集能力与芯片型号相关，可通过[msptiPmSamplingGetCounterAvailability](msptiPmSamplingGetCounterAvailability.md)查询。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL = 0,   // NPU整体利用率，百分比（具体语义随HDK版本而定）
    MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL = 1,   // AI Core利用率，百分比
    MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL = 2,   // AI Vector利用率，百分比
    MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL = 3,   // AI Cube利用率，百分比
    MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL = 4,   // HBM带宽利用率，百分比（仅提供带宽利用率，不提供读写字节速率）
    MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL = 5,   // HBM内存利用率（已用容量/总容量），百分比
    MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW = 6,   // HCCS发送带宽，bytes/s，逐链路上报
    MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW = 7,   // HCCS接收带宽，bytes/s，逐链路上报
    MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW = 8,   // PCIe发送带宽，bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW = 9,   // PCIe接收带宽，bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW = 10,   // RoCE发送带宽，bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW = 11,   // RoCE接收带宽，bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_POWER = 12,   // 功耗，W
    MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE = 13,   // 温度，摄氏度
    MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ = 14,   // AI Core当前频率，MHz
    MSPTI_PM_SAMPLING_METRIC_ID_COUNT = 15,
    MSPTI_PM_SAMPLING_METRIC_ID_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricId;
```
