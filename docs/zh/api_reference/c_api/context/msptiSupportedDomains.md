# msptiSupportedDomains<a name="ZH-CN_TOPIC_0000002091012011"></a>

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

获取可用的回调domain列表。

通过domainTable返回一个大小为domainCount的数组，包含所有可用的回调domain。

## 函数原型<a name="section11218831947"></a>

```cpp
msptiResult msptiSupportedDomains(size_t *domainCount, msptiDomainTable *domainTable)
```

## 参数说明<a name="section8220926"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| domainCount | 输出 | 返回回调域的数量。 |
| domainTable | 输出 | 返回指向可用回调域数组的指针。 |

## 返回值说明<a name="T38657770"></a>

返回MSPTI\_SUCCESS表示成功；domainCount或domainTable为NULL时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
