# msptiActivityPushExternalCorrelationId<a name="ZH-CN_TOPIC_0000002121845788"></a>

## 产品支持情况<a name="section8178181118225"></a>

>![](public_sys-resources/icon-note.gif) **说明：** 
>昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| Atlas 350 加速卡                   |    √     |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 |    √     |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 |    √     |
| Atlas 200I/500 A2 推理产品                  |    √     |
| Atlas 推理系列产品                          |    ×     |
| Atlas 训练系列产品                          |    ×     |

## 功能说明<a name="section20806203412478"></a>

此函数为当前线程创建push-pop调用栈。如果启用了[MSPTI\_ACTIVITY\_KIND\_EXTERNAL\_CORRELATION](msptiActivityKind.md)功能，在push-pop栈内创建[msptiActivityAPI](msptiActivityApi.md)时，会为每一种[msptiExternalCorrelationKind](msptiExternalCorrelationKind.md)创建一个与[msptiActivityAPI](msptiActivityApi.md)记录对应的[msptiActivityExternalCorrelation](msptiActivityExternalCorrelation.md)。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityPushExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t id)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

<a name="table827101275518"></a>
<table><thead align="left"><tr id="row429121265517"><th class="cellrowborder" valign="top" width="28.65286528652865%" id="mcps1.2.4.1.1"><p id="p1329121214558"><a name="p1329121214558"></a><a name="p1329121214558"></a>参数名</p>
</th>
<th class="cellrowborder" valign="top" width="13.661366136613662%" id="mcps1.2.4.1.2"><p id="p10230141454318"><a name="p10230141454318"></a><a name="p10230141454318"></a>输入/输出</p>
</th>
<th class="cellrowborder" valign="top" width="57.68576857685769%" id="mcps1.2.4.1.3"><p id="p83121275519"><a name="p83121275519"></a><a name="p83121275519"></a>说明</p>
</th>
</tr>
</thead>
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>kind</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p18311931172217"><a name="p18311931172217"></a><a name="p18311931172217"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p712111021"><a name="p712111021"></a><a name="p712111021"></a>关联的外部API活动类型，当前有效kind为xxx_CUSTOM0。调用枚举类<a href="msptiExternalCorrelationKind.md">msptiExternalCorrelationKind</a>。</p>
</td>
</tr>
<tr id="row12929154417467"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p10930104415467"><a name="p10930104415467"></a><a name="p10930104415467"></a>id</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p493014448469"><a name="p493014448469"></a><a name="p493014448469"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p119301744204613"><a name="p119301744204613"></a><a name="p119301744204613"></a>由外部组件生成的关联ID，用于push到MSPTI指定的stack里。</p>
</td>
</tr>
</tbody>
</table>

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；外部关联类型无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER和外部关联ID栈空时出栈返回MSPTI\_ERROR\_QUEUE\_EMPTY，表示失败。
