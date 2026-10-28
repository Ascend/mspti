# `msptiActivityGetEnabledKinds`<a name="ZH-CN_TOPIC_0000002091012003"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

<!-- npu="950" id1 -->
- Ascend 950PR and 950DT products: supported
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

Obtains the list of Activity Kinds that are enabled for the subscriber in the specified domain.

If the provided `buffer` has insufficient space to store all enabled Activity Kinds, the function fills the buffer with as many Activity Kinds as possible, and returns the actual number of enabled Activity Kinds through `enabledKindsCount`.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetEnabledKinds(msptiSubscriberHandle subscriber, msptiActivityKind *buffer, uint32_t *bufferSize, uint32_t *enabledKindsCount)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| subscriber | Input | Subscriber handle. |
| buffer | Output | Buffer used to store enabled Activity Kinds. If it is NULL, only the number of enabled Activity Kinds is returned through `enabledKindsCount`. |
| bufferSize | Input | Size of `buffer`. If it is NULL, only the number of enabled Activity Kinds is returned through `enabledKindsCount`. If it is NULL and `buffer` is not NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. |
| enabledKindsCount | Output | Number of enabled Activity Kinds. If it is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `enabledKindsCount` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure. If `buffer` is not NULL and `bufferSize` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
