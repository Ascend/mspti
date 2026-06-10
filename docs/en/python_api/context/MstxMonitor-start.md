# MstxMonitor.start<a name="ZH-CN_TOPIC_0000002111026812"></a>

## Product Support <a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

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

Marks the start of the mstx profiling.

## Function Prototype<a name="section759854510169"></a>

```python
def start(self, mark_cb : Callable[[MarkerData], None] = empty_callback, range_cb : Callable[[RangeMarkerData], None] = empty_callback) -> MsptiResult:
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
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p1991714716347"><a name="p1991714716347"></a><a name="p1991714716347"></a>mark_cb: Callable</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p10230442103813"><a name="p10230442103813"></a><a name="p10230442103813"></a> is used to transfer the collected mstx instantaneous dotting data. The <a href="MarkerData.md">MarkerData</a> structure is invoked.</p>
</td>
</tr>
<tr id="row160312818386"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p16031928153815"><a name="p16031928153815"></a><a name="p16031928153815"></a>range_cb: Callable</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p86031728153812"><a name="p86031728153812"></a><a name="p86031728153812"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p126031228143811"><a name="p126031228143811"></a><a name="p126031228143811"></a> is used to transfer the collected mstx range data. The <a href="RangeMarkerData.md">RangeMarkerData</a> structure is called.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section776014535188"></a>

If MsptiResult.MSPTI\_SUCCESS is returned, the operation is successful. If MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER is returned, the callback function type is incorrect and the operation fails.
