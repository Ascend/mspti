# msptiEnableDomain<a name="ZH-CN_TOPIC_0000002014413745"></a>

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

为特定domain的订阅者开启或关闭所有回调。

当这个CallbackId所在位置触发时，msPTI会主动调用msptiSubscribe接口注册的回调函数。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiEnableDomain(uint32_t enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>enable</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p2286931131811"><a name="p2286931131811"></a><a name="p2286931131811"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a>回调的开关，配置该参数表示开启，未配置表示关闭。</p>
</td>
</tr>
<tr id="row42351156185717"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p9235185665713"><a name="p9235185665713"></a><a name="p9235185665713"></a>subscriber</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p10288231111811"><a name="p10288231111811"></a><a name="p10288231111811"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p223511561579"><a name="p223511561579"></a><a name="p223511561579"></a>订阅者句柄。</p>
</td>
</tr>
<tr id="row154052053135718"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p740513532574"><a name="p740513532574"></a><a name="p740513532574"></a>domain</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p129013112186"><a name="p129013112186"></a><a name="p129013112186"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p1840545320573"><a name="p1840545320573"></a><a name="p1840545320573"></a>组件，当前仅支持Runtime。</p>
</td>
</tr>
</tbody>
</table>

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；用户或域无效时返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
