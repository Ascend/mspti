# msptiPmSamplingGetCounterAvailability<a name="ZH-CN_TOPIC_0000002100000112"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type| Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products|    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products|    √     |
| Atlas inference products|    ×     |
| Atlas training products|    ×     |

## Description<a name="section20806203412478"></a>

Queries the availability of specified metrics on a target device.

Whether a metric can be collected depends on the chip model (for example, some models do not support the Vector frequency). Before configuring metrics, you can use this API to filter out unavailable metrics.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetCounterAvailability(msptiPmSamplingGetCounterAvailabilityParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetCounterAvailabilityParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetCounterAvailabilityParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.deviceId|Input|Target device ID.|
|params.numMetrics|Input|Number of metrics.|
|params.pMetricIds|Input|Array of metric IDs.|
|params.pAvailability|Output|Per-metric availability array. **0** indicates unavailable, and **1** indicates available.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC` is returned when the metric ID is out of range, indicating that the operation fails.
