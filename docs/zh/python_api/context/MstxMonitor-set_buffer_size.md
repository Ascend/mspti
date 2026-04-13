# MstxMonitor.set\_buffer\_size<a name="ZH-CN_TOPIC_0000002302580593"></a>

## 产品支持情况

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

在采集开始前设置Activity Buffer的大小，用来存放采集到的性能数据。

在采集过程中，动态修改Activity Buffer的大小是不生效的，直到本次采集结束，下次采集开始才会生效。

## 函数原型<a name="section759854510169"></a>

```python
def set_buffer_size(cls, size: int) -> MsptiResult:
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
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p1991714716347"><a name="p1991714716347"></a><a name="p1991714716347"></a>size:int</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a>输入</p>
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p4641190193411"><a name="p4641190193411"></a><a name="p4641190193411"></a>size：Activity Buffer的大小，单位MB，默认8MB。</p>
<p id="p10641802345"><a name="p10641802345"></a><a name="p10641802345"></a>仅支持配置为正整数，配置为其他非法值则返回失败，采集使用默认的Activity Buffer大小。</p>
</td>
</tr>
</tbody>
</table>

## 返回值说明<a name="section776014535188"></a>

返回MsptiResult.MSPTI\_SUCCESS表示设置成功；返回MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER，则表示参数设置不正确，设置失败。
