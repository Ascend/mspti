# msptiPmSamplingStop<a name="ZH-CN_TOPIC_0000002100000104"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type | Supported |
| ------------------------------------------- | :------: |
| Ascend 950 products | √ |
| Atlas A3 training products/Atlas A3 inference products | √ |
| Atlas A2 training products/Atlas A2 inference products | √ |
| Atlas 200I/500 A2 inference products | √ |
| Atlas inference products | × |
| Atlas training products | × |

## Description<a name="section20806203412478"></a>

Stops the PM sampling window.

This API can be called only in the Running state. Stopping keeps the object, configuration, and already-collected samples, so the session can be restarted or decoded afterwards.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingStop(msptiPmSamplingStopParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the msptiPmSamplingStopParams type.|
|params.structSize|Input|Struct size, set to sizeof(msptiPmSamplingStopParams).|
|params.pPriv|Input|Private pointer passed through as is. Set it to NULL.|
|params.pPmSamplingObject|Input|PM sampling object.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_INVALID_STATE` is returned when the session is not in the Running state, indicating that the operation fails.
