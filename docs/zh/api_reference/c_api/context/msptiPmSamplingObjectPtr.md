# msptiPmSamplingObjectPtr<a name="ZH-CN_TOPIC_0000002100000123"></a>

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

PM Sampling会话对象的句柄，由[msptiPmSamplingEnable](msptiPmSamplingEnable.md)创建，后续所有PM Sampling接口均通过该句柄指定目标会话。

## 函数原型<a name="section1121883194711"></a>

```cpp
typedef struct msptiPmSamplingObject_st *msptiPmSamplingObjectPtr
```

## 返回值说明<a name="section16621124213476"></a>

无
