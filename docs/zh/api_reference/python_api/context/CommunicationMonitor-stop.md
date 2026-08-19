# CommunicationMonitor.stop<a name="ZH-CN_TOPIC_0000002108243904"></a>

## 产品支持情况<a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
> 
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| 昇腾950PR&950DT系列产品                   |    √     |
| 昇腾A3系列产品 |    √     |
| 昇腾A2系列产品 |    √     |
| 昇腾310B系列产品                  |    √     |
| 昇腾310P系列产品                          |    ×     |
| 昇腾910系列产品                          |    ×     |

## 函数功能

标识通信算子性能数据采集的结束。

## 函数原型<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## 参数说明<a name="section354791521716"></a>

无

## 返回值说明<a name="section776014535188"></a>

返回MsptiResult.MSPTI\_SUCCESS表示成功；返回MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER表示失败。
