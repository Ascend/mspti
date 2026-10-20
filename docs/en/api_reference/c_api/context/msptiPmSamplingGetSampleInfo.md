# msptiPmSamplingGetSampleInfo<a name="ZH-CN_TOPIC_0000002100000111"></a>

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

Obtains the time window (start and end timestamps, in host nanoseconds) of a sample by index.

If `pCounterDataImage` is set to **NULL**, the internal sample queue is queried. Use this API together with [msptiPmSamplingGetSampleEntries](msptiPmSamplingGetSampleEntries.md): this API obtains timestamps, and the latter obtains value entries.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetSampleInfo(msptiPmSamplingGetSampleInfoParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetSampleInfoParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetSampleInfoParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.pPmSamplingObject|Input|PM Sampling object.|
|params.pCounterDataImage|Input|Optional. If it is **NULL**, the internal sample queue is queried.|
|params.sampleIndex|Input|Sample index.|
|params.pStartTimestamp|Output|Start timestamp of the sample, in host nanoseconds.|
|params.pEndTimestamp|Output|End timestamp of the sample, in host nanoseconds.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails.
