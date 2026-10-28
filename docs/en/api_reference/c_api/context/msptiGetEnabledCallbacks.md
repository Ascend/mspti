# `msptiGetEnabledCallbacks`<a name="ZH-CN_TOPIC_0000002091012012"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/Productform/hardwaredesc_0001.html).

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

Obtains the list of callbacks that the subscriber has enabled in the specified domain.

If the provided `buffer` space is insufficient to store all enabled callbacks, the function fills the `buffer` with as many callbacks as possible, and returns the actual number of enabled callbacks through `enabledCallbacksCount`.

## Function Prototype<a name="section11218831947"></a>

```cpp
msptiResult msptiGetEnabledCallbacks(msptiSubscriberHandle subscriber, msptiCallbackDomain domain, msptiCallbackId *buffer, uint32_t *bufferSize, uint32_t *enabledCallbacksCount)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| subscriber | Input | Subscriber handle. |
| domain | Input | Domain for which to obtain callbacks. |
| buffer | Output | Buffer used to store enabled callbacks. If set to NULL, only the number of enabled callbacks is returned through `enabledCallbacksCount`. |
| bufferSize | Input | Size of the `buffer`. If set to NULL, only the number of enabled callbacks is returned through `enabledCallbacksCount`. If it is set to NULL and `buffer` is not NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. |
| enabledCallbacksCount | Output | Number of enabled callbacks. If set to NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `subscriber`, `domain`, `buffer`, `bufferSize`, or `enabledCallbacksCount` is invalid, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure. If msPTI is not initialized, `MSPTI_ERROR_NOT_INITIALIZED` is returned, indicating failure.
