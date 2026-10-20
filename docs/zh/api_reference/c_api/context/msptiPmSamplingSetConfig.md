# msptiPmSamplingSetConfig<a name="ZH-CN_TOPIC_0000002100000102"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<!-- npu="950" id1 -->
- Ascend 950PR&950DT系列产品：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2推理产品：支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas推理系列产品：不支持
<!-- end id5 -->
<!-- npu="910" id6 -->
- Atlas训练系列产品：不支持
<!-- end id6 -->

## 功能说明<a name="section20806203412478"></a>

配置PM Sampling会话的指标集、采样周期与内部样本队列容量。

可在Created、Configured或Stopped状态下调用；Running状态下调用返回MSPTI_ERROR_PM_SAMPLING_INVALID_STATE。对Stopped会话重新配置会清空其未取走的样本队列（指标集可能变化，旧样本与新条目布局不匹配）。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

|参数名|输入/输出|说明|
|--|--|--|
|params|输入|msptiPmSamplingSetConfigParams类型的指针。|
|params.structSize|输入|结构体大小，配置为sizeof(msptiPmSamplingSetConfigParams)。|
|params.pPriv|输入|私有指针，msPTI原样透传，配置为NULL。|
|params.pPmSamplingObject|输入|PM Sampling对象。|
|params.numMetrics|输入|待采集指标数量。|
|params.pMetricIds|输入|指标ID数组，取值为msptiPmSamplingMetricId的枚举值。|
|params.samplingInterval|输入|采样周期，单位ns，取值范围[1ms, 1s]。|
|params.triggerMode|输入|触发模式，取值为msptiPmSamplingTriggerMode的枚举值。|
|params.hardwareBufferSize|输入|内部样本队列容量，单位字节。配置为0表示使用默认值（容纳10000个样本）；非0时最小需容纳单个样本，最大512MB，超出范围返回MSPTI_ERROR_INVALID_PARAMETER。队列内单样本开销视指标集而定，容量可按单样本大小与预期在队样本数估算。|
|params.hwBufferAppendMode|输入|缓冲区满时的追加模式，取值为msptiPmSamplingBufferAppendMode的枚举值。|

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI_SUCCESS表示成功；params为空或字段非法时返回MSPTI_ERROR_INVALID_PARAMETER、pPmSamplingObject非法时返回MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED、会话处于Running状态时返回MSPTI_ERROR_PM_SAMPLING_INVALID_STATE、指标ID超出范围时返回MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC、指标在当前芯片型号不可采集时返回MSPTI_ERROR_PM_SAMPLING_UNSUPPORTED_METRIC或采样周期超出[1ms, 1s]时返回MSPTI_ERROR_PM_SAMPLING_SAMPLING_INTERVAL_INVALID，表示失败。
