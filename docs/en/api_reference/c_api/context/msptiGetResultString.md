# msptiGetResultString<a name="ZH-CN_TOPIC_0000002091012063"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training products                         |    ×     |

## Description <a name="section20806203412478"></a>

Obtains the human-readable descriptive string for a msPTI result code.

This API is a pure query API. It is thread-safe, can be called at any time, and does not require msPTI to be initialized. The returned string is a static constant, which does not need to be released by the caller.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetResultString(msptiResult result, const char **str)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| result | Input | msPTI result code to be translated. |
| str | Output | On success, points to the static result string. On failure, the pointed-to value is not modified. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success, and `str` points to the result string. If `result` is not a valid msPTI result code (including `MSPTI_ERROR_FORCE_INT` and unknown values) or `str` is `NULL`, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
