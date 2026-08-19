# msptiGetEnabledCallbacks<a name="ZH-CN_TOPIC_0000002091012012"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/Productform/hardwaredesc_0001.html)》。

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

获取订阅者在指定domain下已开启的回调列表。

如果提供的buffer空间不足以存储所有已开启的回调，则尽可能多地填充buffer，同时通过enabledCallbacksCount返回已开启的回调的真实数量。

## 函数原型<a name="section11218831947"></a>

```cpp
msptiResult msptiGetEnabledCallbacks(msptiSubscriberHandle subscriber, msptiCallbackDomain domain, msptiCallbackId *buffer, uint32_t *bufferSize, uint32_t *enabledCallbacksCount)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| subscriber | 输入 | 订阅者句柄。 |
| domain | 输入 | 需要获取回调的domain。 |
| buffer | 输出 | 用于存储已开启回调的缓冲区。如果为NULL，则仅通过enabledCallbacksCount返回已开启的回调数量。 |
| bufferSize | 输入 | buffer的大小。如果为NULL，则仅通过enabledCallbacksCount返回已开启的回调数量。如果为NULL且buffer不为NULL，返回MSPTI\_ERROR\_INVALID\_PARAMETER。 |
| enabledCallbacksCount | 输出 | 已开启的回调数量。如果为NULL，返回MSPTI\_ERROR\_INVALID\_PARAMETER。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；subscriber、domain、buffer、bufferSize或enabledCallbacksCount无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败；msPTI未初始化时返回MSPTI\_ERROR\_NOT\_INITIALIZED，表示失败。
