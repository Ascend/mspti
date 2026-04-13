# HcclMonitor.stop<a name="ZH-CN_TOPIC_0000002108243904"></a>

## 产品支持情况<a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

>![](public_sys-resources/icon-note.gif) **说明：** 
>昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| Atlas 350 加速卡                   |    √     |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 |    √     |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 |    √     |
| Atlas 200I/500 A2 推理产品                  |    √     |
| Atlas 推理系列产品                          |    ×     |
| Atlas 训练系列产品                          |    ×     |

## 函数功能<a name="section463019538153"></a>

标识通信算子性能数据采集的结束。

## 函数原型<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## 参数说明<a name="section354791521716"></a>

无

## 返回值说明<a name="section776014535188"></a>

返回MsptiResult.MSPTI\_SUCCESS表示成功；返回MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER表示失败。
