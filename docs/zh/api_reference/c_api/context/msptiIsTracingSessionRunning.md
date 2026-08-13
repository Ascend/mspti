# msptiIsTracingSessionRunning<a name="ZH-CN_TOPIC_0000002091012013"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/Productform/hardwaredesc_0001.html)》。

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

查询 msPTI 数据采集是否仍在运行。

如果其他线程已通过调用任意msPTI API加载了msPTI库，执行数据采集，该接口通过isRunning参数返回1；当msPTI采集流程已结束且当前没有msPTI库被加载时，该接口通过isRunning参数返回0。

该接口可用于判断是否可以安全卸载基于msPTI的工具。注意：该接口本身不会加载msPTI库，仅检查msPTI库是否已被加载。

## 函数原型<a name="section11218831947"></a>

```cpp
msptiResult msptiIsTracingSessionRunning(uint8_t *isRunning)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| isRunning | 输出 | 返回数据采集是否仍在运行。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；isRunning为NULL时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
