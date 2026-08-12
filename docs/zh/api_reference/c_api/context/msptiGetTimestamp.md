# msptiGetTimestamp<a name="ZH-CN_TOPIC_0000002091012005"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| Ascend 950 系列产品                         |    √     |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 |    √     |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 |    √     |
| Atlas 200I/500 A2 推理产品                  |    √     |
| Atlas 推理系列产品                          |    ×     |
| Atlas 训练系列产品                          |    ×     |

## 功能说明<a name="section20806203412478"></a>

获取 msPTI 时间戳。

返回与Activity Record中start和end时间戳对应的规范化时间戳，单位为纳秒（ns）。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetTimestamp(uint64_t *timestamp)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| timestamp | 输出 | 返回指向存储 msPTI 时间戳的指针。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；timestamp为空时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
