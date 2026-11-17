# msptiPmSamplingEnable<a name="ZH-CN_TOPIC_0000002100000101"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT products: supported
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

Creates a PM sampling session on the specified device and enables PM sampling.

Each device allows at most one enabled PM sampling session at a time. The typical usage is: Enable -> SetConfig -> Start -> (sampling) -> Stop -> decode APIs -> Disable. Start/Stop can be executed repeatedly, and samples of all windows accumulate in the same session.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingEnable(msptiPmSamplingEnableParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the msptiPmSamplingEnableParams type.|
|params.structSize|Input|Struct size, set to sizeof(msptiPmSamplingEnableParams).|
|params.pPriv|Input|Private pointer passed through as is. Set it to NULL.|
|params.deviceId|Input|Target device ID.|
|params.pPmSamplingObject|Output|Created PM sampling object, used as the handle by subsequent APIs.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_DEVICE_OFFLINE` is returned when the target device is offline, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED` is returned when a PM sampling session already exists on the device, indicating that the operation fails.
