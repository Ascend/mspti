# msptiPmSamplingGetCounterDataSize<a name="ZH-CN_TOPIC_0000002100000107"></a>

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

Obtains the number of bytes required by a counter data image.

The counter data image is an opaque buffer to users. Users allocate memory of the size returned by this API, and can access the image only through the APIs in this header file. The per-sample entry capacity is derived internally from the metric set planned to be written into the image (per-instance metrics are expanded by the number of reported instances); users only need to specify the maximum number of samples the image can hold via `maxSamples`. A value of 10000 is sufficient for common scenarios (aligned with the default sample queue capacity). The upper limit is subject to the 512 MB total image size budget: maxSamples ≤ (512 MB - image directory size) / per-sample size. Requests exceeding the limit return **MSPTI_ERROR_INVALID_PARAMETER**; reduce `maxSamples` and retry.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetCounterDataSizeParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetCounterDataSizeParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.pPmSamplingObject|Input|PM Sampling object.|
|params.numMetrics|Input|Number of metrics to be written into the image.|
|params.pMetricIds|Input|Array of metric IDs.|
|params.maxSamples|Input|Maximum number of samples that the image can contain. The image size is subject to a per-image budget; requests exceeding the budget return **MSPTI_ERROR_INVALID_PARAMETER**. Reduce `maxSamples` and retry.|
|params.pCounterDataSize|Output|Number of bytes required by the image.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails.
