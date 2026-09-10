# msptiEnableAllDomains<a name="ZH-CN_TOPIC_0000002091012008"></a>

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

为订阅者开启或关闭所有domain的所有回调。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiEnableAllDomains(uint32_t enable, msptiSubscriberHandle subscriber)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| enable | 输入 | 回调的开关，非0表示开启所有回调，0表示关闭所有回调。 |
| subscriber | 输入 | 订阅者句柄。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；msPTI未初始化时返回MSPTI\_ERROR\_NOT\_INITIALIZED，表示失败；libmspti.so未设置到LD\_PRELOAD环境变量时返回MSPTI\_ERROR\_WITHOUT\_LD\_PRELOAD，表示失败；subscriber无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
