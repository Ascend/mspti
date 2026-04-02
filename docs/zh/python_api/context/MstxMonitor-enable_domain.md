# MstxMonitor.enable\_domain<a name="ZH-CN_TOPIC_0000002267876058"></a>

## 产品支持情况<a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

>![](public_sys-resources/icon-note.gif) **说明：** 
>昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| 昇腾950代际产品                   |    √     |
| Atlas A3 训练系列产品/Atlas A3 推理系列产品 |    √     |
| Atlas A2 训练系列产品/Atlas A2 推理系列产品 |    √     |
| Atlas 200I/500 A2 推理产品                  |    √     |
| Atlas 推理系列产品                          |    ×     |
| Atlas 训练系列产品                          |    ×     |

## 函数功能<a name="section463019538153"></a>

开启对应域打点的采集。

## 函数原型<a name="section759854510169"></a>

```python
def enable_domain(self, domain_name: str):
```

## 参数说明<a name="section354791521716"></a>

<a name="zh-cn_topic_0122830089_table438764393513"></a>
<table><thead align="left"><tr id="zh-cn_topic_0122830089_row53871743113510"><th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.1"><p id="zh-cn_topic_0122830089_p1438834363520"><a name="zh-cn_topic_0122830089_p1438834363520"></a><a name="zh-cn_topic_0122830089_p1438834363520"></a>参数名</p>
</th>
<th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.2"><p id="p1769255516412"><a name="p1769255516412"></a><a name="p1769255516412"></a>输入/输出</p>
</th>
<th class="cellrowborder" valign="top" width="72%" id="mcps1.1.4.1.3"><p id="zh-cn_topic_0122830089_p173881843143514"><a name="zh-cn_topic_0122830089_p173881843143514"></a><a name="zh-cn_topic_0122830089_p173881843143514"></a>说明</p>
</th>
</tr>
</thead>
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p1953881145514"><a name="p1953881145514"></a><a name="p1953881145514"></a>domain_name：str</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p4641190193411"><a name="p4641190193411"></a><a name="p4641190193411"></a>对应打点域的名称。</p>
<p id="p10641802345"><a name="p10641802345"></a><a name="p10641802345"></a>可以通过多次调用接口来开启多个域的采集。默认所有域的采集均已开启。</p>
</td>
</tr>
</tbody>
</table>

## 返回值说明<a name="section776014535188"></a>

返回MsptiResult.MSPTI\_SUCCESS则表示成功；domain\_name为空字符串时，返回MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
