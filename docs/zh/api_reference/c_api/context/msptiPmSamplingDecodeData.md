# msptiPmSamplingDecodeData<a name="ZH-CN_TOPIC_0000002100000110"></a>

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

将内部样本队列中的样本解码写入counter data image。

可在Running或Stopped状态下调用。解码采用消费式语义：每次调用将内部样本队列中尚未解码的样本取出并写入镜像（镜像从第一个样本槽位起重写），写入后样本从队列中移除，因此增量调用不会重复解码同一批样本。镜像写满时解码停止，pDecodeStopReason为MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL；队列中全部样本解码完毕时，pDecodeStopReason为MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS。

pOverflow输出参数反馈自上次调用以来内部样本队列是否因溢出丢弃过样本（读取后自动清除）；镜像容量不足返回MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW时，队列中的样本不丢失，调整镜像容量后重新解码即可。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

|参数名|输入/输出|说明|
|--|--|--|
|params|输入|msptiPmSamplingDecodeDataParams类型的指针。|
|params.structSize|输入|结构体大小，配置为sizeof(msptiPmSamplingDecodeDataParams)。|
|params.pPriv|输入|私有指针，msPTI原样透传，配置为NULL。|
|params.pPmSamplingObject|输入|PM Sampling对象。|
|params.pCounterDataImage|输入|已初始化的counter data image。|
|params.counterDataImageSize|输入|镜像大小，单位字节。|
|params.pOverflow|输出|是否因缓冲区溢出丢弃了样本。|
|params.pDecodeStopReason|输出|解码停止原因，取值为msptiPmSamplingDecodeStopReason的枚举值。|

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI_SUCCESS表示成功；params为空或字段非法时返回MSPTI_ERROR_INVALID_PARAMETER、pPmSamplingObject非法时返回MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED或镜像没有空闲槽位容纳新样本时返回MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW，表示失败。
