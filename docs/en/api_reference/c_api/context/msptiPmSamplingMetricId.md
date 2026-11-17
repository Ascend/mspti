# msptiPmSamplingMetricId<a name="ZH-CN_TOPIC_0000002100000201"></a>

msptiPmSamplingMetricId is the metric ID of the PM sampling static metric table. It is used for metric configuration and query by APIs such as [msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md) and [msptiPmSamplingGetMetricProperties](msptiPmSamplingGetMetricProperties.md).

The collection capability of metrics depends on the chip model, which can be queried by [msptiPmSamplingGetCounterAvailability](msptiPmSamplingGetCounterAvailability.md). The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL = 0,   // NPU overall utilization, in percentage (semantics may vary with HDK versions)
    MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL = 1,   // AI Core utilization, in percentage
    MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL = 2,   // AI Vector utilization, in percentage
    MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL = 3,   // AI Cube utilization, in percentage
    MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL = 4,   // HBM bandwidth utilization, in percentage (only the bandwidth utilization is provided, not read/write byte rates)
    MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL = 5,   // HBM memory utilization (used capacity / total capacity), in percentage
    MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW = 6,   // HCCS transmit bandwidth, in bytes/s, reported per link
    MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW = 7,   // HCCS receive bandwidth, in bytes/s, reported per link
    MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW = 8,   // PCIe transmit bandwidth, in bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW = 9,   // PCIe receive bandwidth, in bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW = 10,   // RoCE transmit bandwidth, in bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW = 11,   // RoCE receive bandwidth, in bytes/s
    MSPTI_PM_SAMPLING_METRIC_ID_POWER = 12,   // Power consumption, in watts
    MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE = 13,   // Temperature, in degrees Celsius
    MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ = 14,   // Current AI Core frequency, in MHz
    MSPTI_PM_SAMPLING_METRIC_ID_COUNT = 15,
    MSPTI_PM_SAMPLING_METRIC_ID_FORCE_INT = 0x7fffffff
} msptiPmSamplingMetricId;
```
