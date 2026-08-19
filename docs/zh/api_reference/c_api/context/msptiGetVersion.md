# msptiGetVersion<a name="ZH-CN_TOPIC_0000002091012001"></a>

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

获取msPTI API的版本号。

msPTI API版本号使用**xxyyzz**格式（例如："26.2.0" -> 260200），其中：

- **xx**：msPTI API的主版本号（例如，msPTI 26.x对应26）。
- **yy**：msPTI API的次版本号（例如，26.0对应00，26.1对应01）。
- **zz**：msPTI 特定的更新或补丁版本号。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetVersion(uint32_t *version)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| version | 输出 | 指向存储 msPTI API版本号的指针。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；version为NULL时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败；获取版本号失败时返回MSPTI\_ERROR\_INNER，表示失败。
