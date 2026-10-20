# msptiPmSamplingDecodeData<a name="ZH-CN_TOPIC_0000002100000110"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type| Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products|    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products|    √     |
| Atlas inference products|    ×     |
| Atlas training products|    ×     |

## Description<a name="section20806203412478"></a>

Decodes samples from the internal sample queue and writes them into a counter data image.

This API can be called in the **Running** or **Stopped** state. Decoding is incremental and consumes the internal sample queue: each call moves the samples that have not been decoded yet from the queue into the image (the image is rewritten starting from its first sample slot), and the decoded samples are removed from the queue. Therefore, repeated calls never decode the same batch of samples twice. When the image is full, decoding stops and `pDecodeStopReason` is set to `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL`; when all samples in the queue have been decoded, `pDecodeStopReason` is set to `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS`.

The `pOverflow` output parameter reports whether the internal sample queue dropped samples due to overflow since the last call (the flag is cleared upon reading). If `MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW` is returned because the image capacity is insufficient, samples in the queue are not lost; adjust the image capacity and decode again.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the **msptiPmSamplingDecodeDataParams** type.|
|params.structSize|Input|Structure size, set to **sizeof(msptiPmSamplingDecodeDataParams)**.|
|params.pPriv|Input|Private pointer, which is passed through by msPTI as is. Set it to **NULL**.|
|params.pPmSamplingObject|Input|PM Sampling object.|
|params.pCounterDataImage|Input|Initialized counter data image.|
|params.counterDataImageSize|Input|Image size, in bytes.|
|params.pOverflow|Output|Whether samples were dropped due to buffer overflow.|
|params.pDecodeStopReason|Output|Reason why decoding stopped. Set it to an enumerated value of [msptiPmSamplingDecodeStopReason](msptiPmSamplingDecodeStopReason.md).|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW` is returned when the image has no free slot for new samples, indicating that the operation fails.
