# msptiGetResultString<a name="ZH-CN_TOPIC_0000002091012063"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

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

获取msPTI返回码对应的结果描述字符串（人类可读的错误/结果信息）。

该接口为纯查询接口，线程安全，可在任意时刻调用，不要求msPTI已完成初始化。返回的字符串为静态常量，调用方无需释放。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetResultString(msptiResult result, const char **str)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| result | 输入 | 待翻译的msPTI返回码。 |
| str | 输出 | 成功时返回指向静态结果描述字符串的指针；失败时不修改该指针指向的值。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功，str指向结果描述字符串；result为非法返回码（含MSPTI\_ERROR\_FORCE\_INT及未知数值）或str为NULL时，返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
