# msptiPmSamplingSetConfig<a name="ZH-CN_TOPIC_0000002100000102"></a>

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

Configures the metric set, sampling interval, and hardware buffer of a PM sampling session.

This API can be called in the Created, Configured, or Stopped state; calling it in the Running state returns `MSPTI_ERROR_PM_SAMPLING_INVALID_STATE`. Reconfiguring a Stopped session clears its pending sample queue, since the metric set may change and old samples do not match the new entry layout.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

|Parameter|Input/Output|Description|
|--|--|--|
|params|Input|Pointer of the msptiPmSamplingSetConfigParams type.|
|params.structSize|Input|Struct size, set to sizeof(msptiPmSamplingSetConfigParams).|
|params.pPriv|Input|Private pointer passed through as is. Set it to NULL.|
|params.pPmSamplingObject|Input|PM sampling object.|
|params.numMetrics|Input|Number of metrics to collect.|
|params.pMetricIds|Input|Array of metric IDs. Set the elements to the enumerated values of msptiPmSamplingMetricId.|
|params.samplingInterval|Input|Sampling interval in ns, in the range [1ms, 1s].|
|params.triggerMode|Input|Trigger mode. Set it to the enumerated value of msptiPmSamplingTriggerMode.|
|params.hardwareBufferSize|Input|Internal sample queue capacity in bytes. Set it to **0** for the default value (10000 samples). A non-zero value must hold at least one sample and at most 512 MB; otherwise, **MSPTI_ERROR_INVALID_PARAMETER** is returned. The per-sample overhead in the queue depends on the metric set; size the queue from the per-sample size and the expected number of in-queue samples.|
|params.hwBufferAppendMode|Input|Append mode when the buffer is full. Set it to the enumerated value of msptiPmSamplingBufferAppendMode.|

## Returns<a name="section16621124213476"></a>

`MSPTI_SUCCESS` indicates that the operation is successful. `MSPTI_ERROR_INVALID_PARAMETER` is returned when `params` is a null pointer or a field is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED` is returned when `pPmSamplingObject` is invalid, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_INVALID_STATE` is returned when the session is in the Running state, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC` is returned when a metric ID is out of range, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_UNSUPPORTED_METRIC` is returned when a metric is unavailable on the current chip model, indicating that the operation fails. `MSPTI_ERROR_PM_SAMPLING_SAMPLING_INTERVAL_INVALID` is returned when the sampling interval is out of [1ms, 1s], indicating that the operation fails.
