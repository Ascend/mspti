# msptiPmSamplingGetNumMetrics<a name="ZH-CN_TOPIC_0000002100000105"></a>

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

Obtains the total number of PM sampling metrics that can be configured.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetNumMetrics(msptiPmSamplingGetNumMetricsParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetNumMetricsParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetNumMetricsParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.deviceId|Input|Target device ID.|
|params.pNumMetrics|Output|Total number of metrics that can be configured.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails.
