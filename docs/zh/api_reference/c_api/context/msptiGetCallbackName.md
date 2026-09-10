# msptiGetCallbackName<a name="ZH-CN_TOPIC_0000002091012009"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

<!-- npu="950" id1 -->
- 昇腾950PR&950DT系列产品：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- 昇腾A3系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- 昇腾A2系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- 昇腾310B系列产品：支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- 昇腾310P系列产品：不支持
<!-- end id5 -->
<!-- npu="910" id6 -->
- 昇腾910系列产品：不支持
<!-- end id6 -->

## 功能说明<a name="section20806203412478"></a>

获取指定domain和callback ID对应的回调函数名称。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetCallbackName(msptiCallbackDomain domain, uint32_t cbid, const char **name)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| domain | 输入 | 回调所在的domain域。 |
| cbid | 输入 | 回调的ID。 |
| name | 输出 | 返回回调名称字符串的指针，失败时返回NULL。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；name为NULL，或domain、cbid无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
