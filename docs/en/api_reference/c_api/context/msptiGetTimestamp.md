# `msptiGetTimestamp`<a name="ZH-CN_TOPIC_0000002091012005"></a>

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

Obtains the msPTI timestamp.

Returns the normalized timestamp corresponding to the start and end timestamps in an Activity Record, in nanoseconds (ns).

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetTimestamp(uint64_t *timestamp)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| timestamp | Output | Returns a pointer to the memory that stores the msPTI timestamp. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `timestamp` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
