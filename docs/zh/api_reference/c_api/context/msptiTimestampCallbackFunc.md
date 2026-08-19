# msptiTimestampCallbackFunc<a name="ZH-CN_TOPIC_0000002091012007"></a>

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

时间戳回调函数类型。用户通过[msptiActivityRegisterTimestampCallback](./msptiActivityRegisterTimestampCallback.md)接口将该类型函数注册到msPTI。msPTI需要时间戳时会调用该回调函数获取归一化时间戳，该时间戳可用于存储Activity Record中的start和end时间戳等场景。返回的时间戳必须为纳秒（ns）。

## 函数原型<a name="section1121883194711"></a>

```cpp
typedef uint64_t (*msptiTimestampCallbackFunc)()
```

## 参数说明<a name="section11506138144714"></a>

无。

## 返回值说明<a name="section16621124213476"></a>

返回 msPTI 需要使用的归一化时间戳，单位为纳秒（ns）。
