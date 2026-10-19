# `msptiActivitySetAttribute`<a name="ZH-CN_TOPIC_0000002091015001"></a>

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

Sets the value of the specified Activity attribute, which you can use to configure the Activity collection behavior. Currently, the attributes that can be set include `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE` and `MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK` in [msptiActivityAttribute](./msptiActivityAttribute.md).

- When you set `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE`, the data type pointed to by `value` is `uint32_t`, which indicates the size of the Channel Buffer, in bytes. The value range is [2MB, 10MB]. If the value is outside the value range, the setting fails.
- When you set `MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK`, the data type pointed to by `value` is [msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md), which you use to register a custom timestamp callback function. Its function is equivalent to that of the [msptiActivityRegisterTimestampCallback](./msptiActivityRegisterTimestampCallback.md) API.

> [!NOTE]
>
> `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE` takes effect only on Channel Buffers created after the setting. It has no effect on Channels that have already been created. You are advised to complete the setting before you enable Activity collection.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivitySetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| attr | Input | Activity attribute to set, configured as an enumeration value of [msptiActivityAttribute](./msptiActivityAttribute.md). |
| valueSize | Input | Specifies the size of the `value` buffer, in bytes. If the value is smaller than the size required by the attribute corresponding to `attr`, the API returns `MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT`. |
| value | Input | Pointer to the buffer of the attribute value to set. The buffer size is specified by `valueSize`. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `valueSize` or `value` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned. If the size of the `value` buffer is smaller than the size required by the attribute, `MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT` is returned, indicating failure. If `attr` is invalid or the attribute value is invalid (for example, the Channel Buffer size is outside the value range, or the timestamp callback function is NULL), `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
