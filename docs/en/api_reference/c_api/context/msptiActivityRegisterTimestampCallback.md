# `msptiActivityRegisterTimestampCallback`<a name="ZH-CN_TOPIC_0000002091012006"></a>

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

Registers a custom timestamp callback function with msPTI. After registration, when msPTI needs to obtain a timestamp, for example, when recording the start and end timestamps of an Activity Record, it calls this callback function to obtain a normalized timestamp instead of using the default timestamp acquisition method of msPTI. This implements time alignment in distributed scenarios.

By default, msPTI uses `clock_gettime(CLOCK_REALTIME)` to obtain timestamps on Linux (x86_64 and aarch64) platforms. For activities that record timestamps directly on the NPU, such as kernels and communication, msPTI calls this timestamp callback function during post-processing when it converts NPU timestamps to CPU timestamps. For activities that record timestamps directly on the CPU, msPTI calls this timestamp callback function immediately when the activity occurs.

To ensure that all Activity Records report timestamps through the callback function registered by this API, you are advised to complete the registration of the timestamp callback function before enabling any msPTI Activity Kind. If you replace the timestamp callback function through this API during collection, Activity Records generated before the replacement still report timestamps through the previous timestamp acquisition method.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityRegisterTimestampCallback(msptiTimestampCallbackFunc funcTimestamp)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| funcTimestamp | Input | Timestamp callback function. Its type is [msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md), and the returned timestamp must be in nanoseconds (ns). |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `funcTimestamp` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
