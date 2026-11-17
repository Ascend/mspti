# msptiPmSamplingGetSampleEntries<a name="ZH-CN_TOPIC_0000002100000114"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT products: supported
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 products: supported
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 products: supported
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 inference products: supported
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas inference products: not supported
<!-- end id5 -->
<!-- npu="910" id6 -->
- Atlas training products: not supported
<!-- end id6 -->

## Description<a name="section20806203412478"></a>

Obtains the self-describing value entry list of a sample by index.

A sample consists of self-describing entries, each carrying a metric ID, a hardware instance ID, and a value. Therefore, the number of entries in a sample is not fixed and is usually greater than or equal to the number of configured metrics (per-instance metrics produce one entry for each instance). If `pCounterDataImage` is set to **NULL**, the internal sample queue is queried. If `pEntries` is set to **NULL**, only `pNumEntries` is filled to query the number of entries (two-step retrieval).

Use this API together with [msptiPmSamplingGetSampleInfo](msptiPmSamplingGetSampleInfo.md): this API obtains value entries, and the latter obtains the time window.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingGetSampleEntriesParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingGetSampleEntriesParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.pPmSamplingObject|Input|PM Sampling object.|
|params.pCounterDataImage|Input|Optional. If it is **NULL**, the internal sample queue is queried.|
|params.sampleIndex|Input|Sample index.|
|params.maxEntries|Input|Capacity of `pEntries`. It is ignored when `pEntries` is **NULL**.|
|params.pEntries|Output|Array of value entries, of the [msptiPmSamplingSampleEntry](msptiPmSamplingSampleEntry.md) type. If it is **NULL**, only the number of entries is queried.|
|params.pNumEntries|Output|Actual number of entries in the sample.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails.
