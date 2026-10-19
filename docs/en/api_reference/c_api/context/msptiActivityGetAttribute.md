# `msptiActivityGetAttribute`<a name="ZH-CN_TOPIC_0000002091015002"></a>

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

Obtains the value of the specified Activity attribute, which you can use to query the current configuration of Activity collection. Currently, the attributes that can be obtained include `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE` and `MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK` in [msptiActivityAttribute](./msptiActivityAttribute.md).

- When you obtain `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE`, the size of the current Channel Buffer is returned, in bytes. The data type is `uint32_t`.
- When you obtain `MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK`, the currently registered timestamp callback function is returned. The data type is [msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md).

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| attr | Input | Activity attribute to obtain, configured as an enumeration value of [msptiActivityAttribute](./msptiActivityAttribute.md). |
| valueSize | Input/Output | On input, specifies the size of the `value` buffer, in bytes. After the API returns successfully, it is updated to the size of the attribute value actually written to `value`. |
| value | Output | Pointer to the buffer that receives the attribute value, with the buffer size specified by `valueSize`. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `valueSize` or `value` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. If the size of the `value` buffer is smaller than the size required by the attribute, `MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT` is returned, indicating failure. If `attr` is invalid, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
