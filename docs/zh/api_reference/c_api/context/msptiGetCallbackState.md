# msptiGetCallbackState<a name="ZH-CN_TOPIC_0000002091012010"></a>

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

获取指定domain和指定函数ID对应的回调当前的开启/关闭状态。

若回调已开启，则通过enable参数返回非0值；若未开启，则通过enable参数返回0值。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiGetCallbackState(uint32_t *enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain, msptiCallbackId cbid)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| enable | 输出 | 回调已开启时返回非0值，未开启时返回0值。 |
| subscriber | 输入 | 订阅者句柄。 |
| domain | 输入 | 回调所在的domain。 |
| cbid | 输入 | 回调的ID。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；enable、subscriber、domain或cbid无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败；msPTI未初始化时返回MSPTI\_ERROR\_NOT\_INITIALIZED，表示失败。
