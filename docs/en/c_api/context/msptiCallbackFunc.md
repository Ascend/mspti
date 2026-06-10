# msptiCallbackFunc<a name="ZH-CN_TOPIC_0000002014573221"></a>

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

Callback function type.

## Function Prototype<a name="section1121883194711"></a>

```cpp
typedef void (*msptiCallbackFunc)(void *userdata, msptiCallbackDomain domain, msptiCallbackId cbid, const msptiCallbackData *cbdata)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>userdata</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p98288114171"><a name="p98288114171"></a><a name="p98288114171"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a> user-side data address.</p>
</td>
</tr>
<tr id="row1940614612417"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p3406206194111"><a name="p3406206194111"></a><a name="p3406206194111"></a>domain</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p11831211181716"><a name="p11831211181716"></a><a name="p11831211181716"></a> Input. </p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p540710654112"><a name="p540710654112"></a><a name="p540710654112"></a> Domain to which the current callback belongs.</p>
</td>
</tr>
<tr id="row56118904112"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p1614944111"><a name="p1614944111"></a><a name="p1614944111"></a>cbid</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p19833161113174"><a name="p19833161113174"></a><a name="p19833161113174"></a> Input. </p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p186110911418"><a name="p186110911418"></a><a name="p186110911418"></a> ID of the current callback.</p>
</td>
</tr>
<tr id="row4110294513"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p414244516"><a name="p414244516"></a><a name="p414244516"></a>cbdata</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1383541171719"><a name="p1383541171719"></a><a name="p1383541171719"></a> Input. </p>
</td>
Indicates the additional information triggered by the current callback of the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p1624118151355"><a name="p1624118151355"></a><a name="p1624118151355"></a>. When domain is MSPTI_CB_DOMAIN_RUNTIME, the cbdata type is <a href="msptiCallbackData.md">msptiCallbackData</a>.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

None
