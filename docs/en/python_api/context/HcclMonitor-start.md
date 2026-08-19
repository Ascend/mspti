# HcclMonitor.start<a name="ZH-CN_TOPIC_0000002143882701"></a>

## Product Support <a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
> 
> For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950PR&950DT Products                  |    √     |
| Ascend A3 Products|    √     |
| Ascend A2 Products|    √     |
| Ascend 310B Products                 |    √     |
| Ascend 310P Products                         |    ×     |
| Ascend 910 Products                         |    ×     |

## Function Usage <a name="section463019538153"></a>

Marks the start of the communication operator profiling.

## Function Prototype<a name="section759854510169"></a>

```python
def start(self, cb: Callable[[HcclData], None]) -> MsptiResult:
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
<tbody><tr id="row10379818172019"><td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.1 "><p id="p10791153923311"><a name="p10791153923311"></a><a name="p10791153923311"></a>cb: Callable</p>
</td>
<td class="cellrowborder" valign="top" width="14.000000000000002%" headers="mcps1.1.4.1.2 "><p id="p187904392335"><a name="p187904392335"></a><a name="p187904392335"></a> Input </p>
</td>
<td class="cellrowborder" valign="top" width="72%" headers="mcps1.1.4.1.3 "><p id="p1871112011815"><a name="p1871112011815"></a><a name="p1871112011815"></a> is used to transfer the collected communication data. The <a href="HcclData.md">HcclData</a> structure is called.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section776014535188"></a>

If MsptiResult.MSPTI\_SUCCESS is returned, the operation is successful. If MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER is returned, the callback function type is incorrect, indicating that the operation fails.
