# `msptiActivityGetNumDroppedRecords`<a name="ZH-CN_TOPIC_0000002091012004"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

<!-- npu="950" id1 -->
- Atlas 950PR and 950DT products: supported
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

Obtains the number of Activity Records dropped due to insufficient Activity Buffer space.

The dropped count includes the number of Records that cannot be recorded because msPTI has no available Activity Buffer space (for example, the `msptiBuffersCallbackRequestFunc` callback does not return an empty buffer of a sufficient size). After you call this API, the dropped count is reset to 0.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetNumDroppedRecords(void *context, uint32_t streamId, size_t *dropped)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| context | Input | Pointer to the context. |
| streamId | Input | Stream ID. |
| dropped | Output | Number of Records dropped since the last call to this API. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `dropped` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
