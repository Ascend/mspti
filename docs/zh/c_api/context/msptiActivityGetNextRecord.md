# msptiActivityGetNextRecord<a name="ZH-CN_TOPIC_0000002014573217"></a>

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

依次从Activity Buffer中取出数据，每次读取一条Activity数据。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetNextRecord(uint8_t *buffer, size_t validBufferSizeBytes, msptiActivity **record)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>buffer</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1043011591665"><a name="p1043011591665"></a><a name="p1043011591665"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a>设置Activity Buffer的地址。</p>
</td>
</tr>
<tr id="row18118485118"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p17567995224"><a name="p17567995224"></a><a name="p17567995224"></a>validBufferSizeBytes</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p164321599616"><a name="p164321599616"></a><a name="p164321599616"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p61315168385"><a name="p61315168385"></a><a name="p61315168385"></a>Activity Buffer中记录数据的大小。</p>
</td>
</tr>
<tr id="row208099610258"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p16810567252"><a name="p16810567252"></a><a name="p16810567252"></a>record</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p114341059162"><a name="p114341059162"></a><a name="p114341059162"></a>输出</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p8810126142514"><a name="p8810126142514"></a><a name="p8810126142514"></a>记录Record数据的地址。</p>
</td>
</tr>
</tbody>
</table>

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；Activity Buffer没有更多的Record数据时返回MSPTI\_ERROR\_MAX\_LIMIT\_REACHED（表示已取完Activity Buffer中数据），表示失败；Activity Buffer为空时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
