# msptiActivityPopExternalCorrelationId<a name="ZH-CN_TOPIC_0000002157283965"></a>

## Supported Products<a name="section8178181118225"></a>

>![](public_sys-resources/icon-note.gif) **Note:**
>For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training series products                         |    ×     |

## Description <a name="section20806203412478"></a>

Pops the external correlation ID for the calling thread.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityPopExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t *lastId)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Command-line options

<a name="table827101275518"></a>
<table><thead align="left"><tr id="row429121265517"><th class="cellrowborder" valign="top" width="28.65286528652865%" id="mcps1.2.4.1.1"><p id="p1329121214558"><a name="p1329121214558"></a><a name="p1329121214558"></a> Parameter Name</p>
</th>
<th class="cellrowborder" valign="top" width="13.661366136613662%" id="mcps1.2.4.1.2"><p id="p10230141454318"><a name="p10230141454318"></a><a name="p10230141454318"></a> Input/Output</p>
</th>
<th class="cellrowborder" valign="top" width="57.68576857685769%" id="mcps1.2.4.1.3"><p id="p83121275519"><a name="p83121275519"></a><a name="p83121275519"></a> Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>kind</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p18311931172217"><a name="p18311931172217"></a><a name="p18311931172217"></a> input. </p>
</td>
Type of the external API associated with the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p712111021"><a name="p712111021"></a><a name="p712111021"></a>. Currently, the valid value is xxx_CUSTOM0.</p>
</td>
</tr>
<tr id="row12929154417467"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p10930104415467"><a name="p10930104415467"></a><a name="p10930104415467"></a>lastId</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p493014448469"><a name="p493014448469"></a><a name="p493014448469"></a> Input </p>
</td>
The <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p119301744204613"><a name="p119301744204613"></a><a name="p119301744204613"></a>MSPTI pops the ID from the stack and writes it to lastId.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

MSPTI_SUCCESS indicates that the operation is successful. MSPTI_ERROR_INVALID_PARAMETER indicates that the operation fails because the external association type is invalid.
