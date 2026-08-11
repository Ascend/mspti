# msptiActivityGetNumDroppedRecords<a name="ZH-CN_TOPIC_0000002091012004"></a>

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

获取因Activity缓冲区空间不足而丢弃的Activity Record数量。

丢弃数量包括：由于msPTI没有可用的Activity缓冲区空间（例如msptiBuffersCallbackRequestFunc回调未返回足够大小的空缓冲区）而无法记录的Record数量。调用该接口后，丢弃计数会被重置为0。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetNumDroppedRecords(void *context, uint32_t streamId, size_t *dropped)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| context | 输入 | Context指针。 |
| streamId | 输入 | Stream ID。 |
| dropped | 输出 | 上次调用该接口以来被丢弃的Record数量。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；dropped为空时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
