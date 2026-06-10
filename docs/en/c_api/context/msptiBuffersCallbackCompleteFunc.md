# msptiBuffersCallbackCompleteFunc<a name="ZH-CN_TOPIC_0000001977973396"></a>

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

Registers the callback function with the MSPTI to release the data in the Activity Buffer. When using the activity API, a user (subscriber) needs to customize this function and register it with MSPTI. When the storage space of the activity buffer is full, MSPTI calls this function to notify the user to consume data in the activity buffer and release the memory space.

## Function Prototype<a name="section1121883194711"></a>

```cpp
typedef void(*msptiBuffersCallbackCompleteFunc)(uint8_t *buffer, size_t size, size_t validSize)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 2** Command-line options

<a name="table827101275518"></a>
<table><thead align="left"><tr id="row429121265517"><th class="cellrowborder" valign="top" width="28.65286528652865%" id="mcps1.2.4.1.1"><p id="p1329121214558"><a name="p1329121214558"></a><a name="p1329121214558"></a> Parameter Name</p>
</th>
<th class="cellrowborder" valign="top" width="13.661366136613662%" id="mcps1.2.4.1.2"><p id="p10230141454318"><a name="p10230141454318"></a><a name="p10230141454318"></a> Input/Output</p>
</th>
<th class="cellrowborder" valign="top" width="57.68576857685769%" id="mcps1.2.4.1.3"><p id="p83121275519"><a name="p83121275519"></a><a name="p83121275519"></a> Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>buffer</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p599212241261"><a name="p599212241261"></a><a name="p599212241261"></a>Enter </p>.
</td>
Address of the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a>Activity Buffer.</p>
</td>
</tr>
<tr id="row18118485118"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p17567995224"><a name="p17567995224"></a><a name="p17567995224"></a>size</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1399417247617"><a name="p1399417247617"></a><a name="p1399417247617"></a>Enter </p>.
</td>
Size of the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p61315168385"><a name="p61315168385"></a><a name="p61315168385"></a>Activity Buffer, in bytes.</p>
</td>
</tr>
<tr id="row208099610258"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p16810567252"><a name="p16810567252"></a><a name="p16810567252"></a>validSize</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p99964242613"><a name="p99964242613"></a><a name="p99964242613"></a> Input</p>
</td>
Size of the data recorded by the <td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p8810126142514"><a name="p8810126142514"></a><a name="p8810126142514"></a>Activity Buffer, in bytes.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

None
