# msptiPmSamplingGetSampleEntries<a name="ZH-CN_TOPIC_0000002100000114"></a>

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

按索引获取样本的自描述值条目列表。

样本由自描述条目组成，每条携带指标ID、硬件实例ID与值，因此样本的条目数不固定，通常大于等于配置的指标数（逐实例指标会为每个实例产出一条）。pCounterDataImage配置为NULL时查询内部样本队列。pEntries配置为NULL时只填充pNumEntries，用于查询条目数（两段式取数）。

与msptiPmSamplingGetSampleInfo配合使用：本接口取值条目，后者取时间窗口。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

|参数名|输入/输出|说明|
|--|--|--|
|params|输入|msptiPmSamplingGetSampleEntriesParams类型的指针。|
|params.structSize|输入|结构体大小，配置为sizeof(msptiPmSamplingGetSampleEntriesParams)。|
|params.pPriv|输入|私有指针，msPTI原样透传，配置为NULL。|
|params.pPmSamplingObject|输入|PM Sampling对象。|
|params.pCounterDataImage|输入|可选；为NULL时查询内部样本队列。|
|params.sampleIndex|输入|样本索引。|
|params.maxEntries|输入|pEntries的容量，pEntries为NULL时忽略。|
|params.pEntries|输出|值条目数组，类型为msptiPmSamplingSampleEntry；为NULL时仅查询条目数。|
|params.pNumEntries|输出|样本的实际条目数。|

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI_SUCCESS表示成功；params为空或字段非法时返回MSPTI_ERROR_INVALID_PARAMETER或pPmSamplingObject非法时返回MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED，表示失败。
