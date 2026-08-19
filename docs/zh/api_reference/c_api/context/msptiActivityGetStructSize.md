# msptiActivityGetStructSize<a name="ZH-CN_TOPIC_0000002091012002"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| 昇腾950PR&950DT系列产品                         |    √     |
| 昇腾A3系列产品 |    √     |
| 昇腾A2系列产品 |    √     |
| 昇腾310B系列产品                  |    √     |
| 昇腾310P系列产品                          |    ×     |
| 昇腾910系列产品                          |    ×     |

## 功能说明<a name="section20806203412478"></a>

获取指定Activity Kind对应的Activity结构体的大小。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetStructSize(msptiActivityKind activityKind, uint32_t version, size_t *activityStructSize)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| activityKind | 输入 | 需要获取结构体大小的Activity Kind，配置为[msptiActivityKind](msptiActivityKind.md)的枚举值。 |
| version | 输入 | msPTI API的版本号，用于兼容不同版本的Activity结构体。 |
| activityStructSize | 输出 | 返回指向存储指定Activity结构体大小的指针。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；activityStructSize为NULL时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败；activityKind无效时返回MSPTI\_ERROR\_INVALID\_KIND，表示失败。
