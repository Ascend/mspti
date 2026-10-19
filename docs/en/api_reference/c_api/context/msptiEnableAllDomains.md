# `msptiEnableAllDomains`<a name="ZH-CN_TOPIC_0000002091012008"></a>

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

Enables or disables all callbacks of all domains for subscribers.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiEnableAllDomains(uint32_t enable, msptiSubscriberHandle subscriber)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| enable | Input | Callback switch. A non-zero value enables all callbacks, and 0 disables all callbacks. |
| subscriber | Input | Subscriber handle. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If msPTI is not initialized, `MSPTI_ERROR_NOT_INITIALIZED` is returned, indicating failure. If `libmspti.so` is not set in the `LD_PRELOAD` environment variable, `MSPTI_ERROR_WITHOUT_LD_PRELOAD` is returned, indicating failure. If `subscriber` is invalid, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
