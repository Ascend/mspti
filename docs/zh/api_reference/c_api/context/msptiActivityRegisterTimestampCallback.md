# msptiActivityRegisterTimestampCallback<a name="ZH-CN_TOPIC_0000002091012006"></a>

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

向msPTI注册自定义时间戳回调函数。注册后，msPTI在需要获取时间戳（如记录Activity Record的start和end时间戳）时，会调用该回调函数获取归一化时间戳，不再使用msPTI默认的时间戳获取方式，用于实现分布式场景下的时间对齐。

默认情况下，msPTI在Linux（x86_64、aarch64）平台使用`clock_gettime(CLOCK_REALTIME)`获取时间戳。对于Kernel、通信等直接在NPU上记录时间戳的Activity，msPTI在后处理阶段将NPU时间戳转换为CPU时间戳时调用该时间戳回调函数；对于直接在CPU上记录时间戳的Activity，msPTI在Activity发生时立即调用该时间戳回调函数。

建议在启用任意msPTI Activity Kind之前完成时间戳回调函数的注册，以确保所有Activity Record均使用通过该接口注册的回调函数上报时间戳。在采集过程中通过该接口更换时间戳回调函数，会导致更换前产生的Activity Record仍使用更换前的时间戳获取方式上报时间戳。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityRegisterTimestampCallback(msptiTimestampCallbackFunc funcTimestamp)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| funcTimestamp | 输入 | 时间戳回调函数，类型为[msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md)，返回的时间戳需要以纳秒（ns）为单位。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；funcTimestamp为NULL时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
