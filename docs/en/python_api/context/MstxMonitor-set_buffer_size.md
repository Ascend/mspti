# MstxMonitor.set\_buffer\_size<a name="ZH-CN_TOPIC_0000002302580593"></a>

## Supported Products

>![](public_sys-resources/icon-note.gif) **Note:**
>For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training series products                         |    ×     |

## Function Usage <a name="section463019538153"></a>

Sets the size of the activity buffer before profiling starts to store the profile data.

During the profiling, the dynamic modification of the activity buffer size does not take effect. The modification takes effect only after the current profiling is complete and the next profiling starts.

## Function Prototype<a name="section759854510169"></a>

```python
def set_buffer_size(cls, size: int) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

<a name="zh-cn_topic_0122830089_table438764393513"></a>
<table><thead align="left"><tr id="zh-cn_topic_0122830089_row53871743113510"><th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.1"><p id="zh-cn_topic_0122830089_p1438834363520"><a name="zh-cn_topic_0122830089_p1438834363520"></a><a name="zh-cn_topic_0122830089_p1438834363520"></a> Parameter Name</p>
</th>
<th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.2"><p id="p1769255516412"><a name="p1769255516412"></a><a name="p1769255516412"></a> Input/Output</p>
</th>
<th class="cellrowborder" valign="top" width="72%" id="mcps1.1.4.1.3"><p id="zh-cn_topic_0122830089_p173881843143514"><a name="zh-cn_topic_0122830089_p173881843143514"></a><a name="zh-cn_topic_0122830089_p173881843143514"></a> Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p1991714716347"><a name="p1991714716347"></a><a name="p1991714716347"></a>size:int</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p4641190193411"><a name="p4641190193411"></a><a name="p4641190193411"></a>size: size of the activity buffer, in MB. The default value is 8 MB.</p>
The value of <p id="p10641802345"><a name="p10641802345"></a><a name="p10641802345"></a> must be a positive integer. If the value is invalid, a failure message is returned and the default activity buffer size is used.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section776014535188"></a>

If MsptiResult.MSPTI\_SUCCESS is returned, the setting is successful. If MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER is returned, the setting fails because the parameter is incorrect.
