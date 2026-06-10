# msptiSubscribe<a name="ZH-CN_TOPIC_0000002014413741"></a>

## Supported Products<a name="section8178181118225"></a>

>![](public_sys-resources/icon-note.gif) **Note:**
>For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training series/Atlas A3 inference series|    √     |
| Atlas A2 training series/Atlas A2 inference series|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference series                         |    ×     |
| Atlas training series products                         |    ×     |

## Description <a name="section20806203412478"></a>

Registers callback functions with MSPTI. A user (subscriber) needs to call this API before calling the MSPTI APIs. Only one subscriber is supported at a time.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiSubscribe(msptiSubscriberHandle *subscriber, msptiCallbackFunc callback, void *userdata)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>subscriber</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p9342183212166"><a name="p9342183212166"></a><a name="p9342183212166"></a>Enter </p>.
</td>
Handle address of the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a> subscriber.</p>
</td>
</tr>
<tr id="row1940614612417"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p3406206194111"><a name="p3406206194111"></a><a name="p3406206194111"></a>callback</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1734517328167"><a name="p1734517328167"></a><a name="p1734517328167"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p540710654112"><a name="p540710654112"></a><a name="p540710654112"></a> callback function.</p>
</td>
</tr>
<tr id="row56118904112"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p1614944111"><a name="p1614944111"></a><a name="p1614944111"></a>userdata</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1347173215168"><a name="p1347173215168"></a><a name="p1347173215168"></a> Input </p>
</td>
User-defined data address of the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p186110911418"><a name="p186110911418"></a><a name="p186110911418"></a> subscriber. The subscriber data is transferred to the callback function through this parameter.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

MSPTI_SUCCESS indicates that the initialization is successful. MSPTI_ERROR_INNER is returned if the MSPTI cannot be initialized, MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED is returned if there are already MSPTI subscribers, or MSPTI_ERROR_INVALID_PARAMETER is returned if the subscriber is empty. In this case, the initialization fails.
