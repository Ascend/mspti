# `msptiSupportedDomains`<a name="ZH-CN_TOPIC_0000002091012011"></a>

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

Obtains the list of available callback domains.

The function returns an array of size `domainCount` through `domainTable`, containing all available callback domains.

## Function Prototype<a name="section11218831947"></a>

```cpp
msptiResult msptiSupportedDomains(size_t *domainCount, msptiDomainTable *domainTable)
```

## Parameters<a name="section8220926"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| domainCount | Output | Number of callback domains returned. |
| domainTable | Output | Returns a pointer to an array of available callback domains. |

## Returns<a name="T38657770"></a>

`MSPTI_SUCCESS` indicates success. If `domainCount` or `domainTable` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
