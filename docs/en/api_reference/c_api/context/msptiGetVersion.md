# `msptiGetVersion`<a name="ZH-CN_TOPIC_0000002091012001"></a>

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

Obtains the version of the msPTI API.

The msPTI API version number uses the `xxyyzz` format (for example, "26.2.0" -> 260200), where:

- `xx`: major version number of the msPTI API (for example, msPTI 26.x corresponds to 26)
- `yy`: minor version number of the msPTI API (for example, 26.0 corresponds to 00, and 26.1 corresponds to 01)
- `zz`: msPTI-specific update or patch version number

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetVersion(uint32_t *version)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| version | Output | Pointer to the memory that stores the msPTI API version number. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `version` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure. If the version number cannot be obtained, `MSPTI_ERROR_INNER` is returned, indicating failure.
