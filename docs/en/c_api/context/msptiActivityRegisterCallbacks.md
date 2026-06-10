# msptiActivityRegisterCallbacks<a name="ZH-CN_TOPIC_0000002014413733"></a>

## Supported Products<a name="section8178181118225"></a>

>![](public_sys-resources/icon-note.gif) **Note:**
>For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products                  |    √     |
| Atlas A3 training series/Atlas A3 inference series|    √     |
| Atlas A2 training series/Atlas A2 inference series|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference series                         |    ×     |
| Atlas training series products                         |    ×     |

## Description <a name="section20806203412478"></a>

Registers callback functions with MSPTI for activity buffer processing. When the activity buffer is insufficient, the **funcBufferRequested** function is called to allocate memory. When the activity buffer is full, the **funcBufferCompleted** function is called to notify the user to consume activity data and release the activity buffer.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityRegisterCallbacks(msptiBuffersCallbackRequestFunc funcBufferRequested, msptiBuffersCallbackCompleteFunc funcBufferCompleted)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>funcBufferRequested</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p168851111619"><a name="p168851111619"></a><a name="p168851111619"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a>Activity Buffer memory allocation function.</p>
</td>
</tr>
<tr id="row18118485118"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p17567995224"><a name="p17567995224"></a><a name="p17567995224"></a>funcBufferCompleted</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p18311931172217"><a name="p18311931172217"></a><a name="p18311931172217"></a> Input </p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p61315168385"><a name="p61315168385"></a><a name="p61315168385"></a>Activity Buffer is a function for data consumption and memory release.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

MSPTI_SUCCESS indicates success. If funcBufferRequested or funcBufferCompleted is NULL, MSPTI_ERROR_INVALID_PARAMETER is returned, indicating failure.
