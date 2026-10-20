# msptiPmSamplingGetCounterDataInfo<a name="ZH-CN_TOPIC_0000002100000109"></a>

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

Obtains the sample statistics of a counter data image (total slots, populated samples, and completed samples).

If `pCounterDataImage` is set to **NULL**, the statistics of the internal sample queue are returned, without decoding first.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetCounterDataInfo(msptiPmSamplingGetCounterDataInfoParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetCounterDataInfoParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetCounterDataInfoParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.pPmSamplingObject|Input|PM Sampling object.|
|params.pNumTotalSamples|Output|Total number of sample slots in the image.|
|params.pNumPopulatedSamples|Output|Number of populated samples.|
|params.pNumCompletedSamples|Output|Number of completed samples.|
|params.pCounterDataImage|Input|Optional. If it is **NULL**, the internal sample queue is queried.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails.
