# msptiPmSamplingGetCounterDataSize<a name="ZH-CN_TOPIC_0000002100000107"></a>

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

获取counter data image所需的字节数。

counter data image是对用户不透明的缓冲区，由用户按本接口返回的大小分配内存，只能通过本头文件中的接口访问。单样本的值条目容量由计划写入镜像的指标集在接口内部推导（逐实例指标按其上报的实例数展开），用户只需通过maxSamples指定镜像可容纳的样本数。maxSamples常规配置10000即可（与样本队列默认容量一致）；上限受镜像总大小512MB预算约束，即maxSamples ≤ (512MB - 镜像目录区大小) / 单样本大小，超出时返回MSPTI_ERROR_INVALID_PARAMETER，需减小maxSamples后重试。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

|参数名|输入/输出|说明|
|--|--|--|
|params|输入|msptiPmSamplingGetCounterDataSizeParams类型的指针。|
|params.structSize|输入|结构体大小，配置为sizeof(msptiPmSamplingGetCounterDataSizeParams)。|
|params.pPriv|输入|私有指针，msPTI原样透传，配置为NULL。|
|params.pPmSamplingObject|输入|PM Sampling对象。|
|params.numMetrics|输入|计划写入镜像的指标数量。|
|params.pMetricIds|输入|指标ID数组。|
|params.maxSamples|输入|镜像可容纳的最大样本数。镜像大小受单镜像预算限制，超出预算时返回MSPTI_ERROR_INVALID_PARAMETER，需减小maxSamples后重试。|
|params.pCounterDataSize|输出|镜像所需字节数。|

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI_SUCCESS表示成功；params为空或字段非法时返回MSPTI_ERROR_INVALID_PARAMETER或pPmSamplingObject非法时返回MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED，表示失败。
