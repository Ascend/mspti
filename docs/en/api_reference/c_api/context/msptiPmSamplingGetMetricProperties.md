# msptiPmSamplingGetMetricProperties<a name="ZH-CN_TOPIC_0000002100000106"></a>

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

Obtains the static properties of a specified PM sampling metric (short name, group, data type, and unit).

The output `available` field indicates whether the metric can be collected on the chip model of the target device.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetMetricProperties(msptiPmSamplingGetMetricPropertiesParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetMetricPropertiesParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetMetricPropertiesParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.deviceId|Input|Target device ID.|
|params.metricId|Input|Metric ID. Set it to an enumerated value of [msptiPmSamplingMetricId](msptiPmSamplingMetricId.md).|
|params.pProperties|Output|Metric properties, of the [msptiPmSamplingMetricProperties](msptiPmSamplingMetricProperties.md) type.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC` is returned when the metric ID is out of range, indicating that the operation fails.
