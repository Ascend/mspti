# MstxMonitor.start<a name="ZH-CN_TOPIC_0000002111026812"></a>

## Supported Products<a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
> 
> For details about Ascend product models, see [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Atlas 350 accelerator cards                  |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training products                         |    ×     |

## Function<a name="section463019538153"></a>

Marks the start of msTX data profiling.

## Prototype<a name="section759854510169"></a>

```python
def start(self, mark_cb: Callable[[MarkerData], None] = empty_callback, range_cb: Callable[[RangeMarkerData], None] = empty_callback) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

<a name="zh-cn_topic_0122830089_table438764393513"></a>
<table><thead align="left"><tr id="zh-cn_topic_0122830089_row53871743113510"><th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.1"><p id="zh-cn_topic_0122830089_p1438834363520"><a name="zh-cn_topic_0122830089_p1438834363520"></a><a name="zh-cn_topic_0122830089_p1438834363520"></a>Parameter</p>
</th>
<th class="cellrowborder" valign="top" width="14.000000000000002%" id="mcps1.1.4.1.2"><p id="p1769255516412"><a name="p1769255516412"></a><a name="p1769255516412"></a>Input/Output</p>
</th>
<th class="cellrowborder" valign="top" width="72%" id="mcps1.1.4.1.3"><p id="zh-cn_topic_0122830089_p173881843143514"><a name="zh-cn_topic_0122830089_p173881843143514"></a><a name="zh-cn_topic_0122830089_p173881843143514"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p1991714716347"><a name="p1991714716347"></a><a name="p1991714716347"></a>mark_cb: Callable</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a>Input</p>
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p10230442103813"><a name="p10230442103813"></a><a name="p10230442103813"></a>Used to pass the profiled msTX instantaneous marker data. Calls the <a href="MarkerData.md">MarkerData</a> struct.</p>
</td>
</tr>
<tr id="row160312818386"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p16031928153815"><a name="p16031928153815"></a><a name="p16031928153815"></a>range_cb: Callable</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p86031728153812"><a name="p86031728153812"></a><a name="p86031728153812"></a>Input</p>
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p126031228143811"><a name="p126031228143811"></a><a name="p126031228143811"></a>Used to pass the profiled msTX range marker data. Calls the <a href="RangeMarkerData.md">RangeMarkerData</a> struct.</p>
</td>
</tr>
</tbody>
</table>

## Returns<a name="section776014535188"></a>

Returns `MsptiResult.MSPTI_SUCCESS` on success, or `MsptiResult.MSPTI_ERROR_INVALID_PARAMETER` if the callback function type is incorrect.
