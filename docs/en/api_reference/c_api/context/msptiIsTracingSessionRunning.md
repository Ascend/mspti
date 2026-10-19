# `msptiIsTracingSessionRunning`<a name="ZH-CN_TOPIC_0000002091012013"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/Productform/hardwaredesc_0001.html).

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

Queries whether msPTI data collection is still running.

If another thread has already loaded the msPTI library by calling any msPTI API and is performing data collection, this API returns 1 through the `isRunning` parameter. When the msPTI collection process has finished and no msPTI library is currently loaded, this API returns 0 through the `isRunning` parameter.

You can use this API to determine whether it is safe to uninstall msPTI-based tools. Note: This API itself does not load the msPTI library. It only checks whether the msPTI library has already been loaded.

## Function Prototype<a name="section11218831947"></a>

```cpp
msptiResult msptiIsTracingSessionRunning(uint8_t *isRunning)
```

## Parameters<a name="section11506138144714"></a>

**Table 1** Parameters

| Parameter Name | Input/Output | Description |
| --- | --- | --- |
| isRunning | Output | Returns whether data collection is still running. |

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates success. If `isRunning` is NULL, `MSPTI_ERROR_INVALID_PARAMETER` is returned, indicating failure.
