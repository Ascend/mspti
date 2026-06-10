# msptiEnableCallback<a name="ZH-CN_TOPIC_0000001977813684"></a>

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

Enables or disables callbacks for subscribers of specific **domain** and **CallbackId**.

When the event is triggered at the location of **CallbackId**, MSPTI calls the callback function registered through the **msptiSubscribe** API.

## Function Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiEnableCallback(uint32_t enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain, msptiCallbackId cbid)
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
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>enable</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p9990142551817"><a name="p9990142551817"></a><a name="p9990142551817"></a> input. </p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p131994242276"><a name="p131994242276"></a><a name="p131994242276"></a> callback switch. If this parameter is configured, the callback is enabled. If this parameter is not configured, the callback is disabled.</p>
</td>
</tr>
<tr id="row42351156185717"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p9235185665713"><a name="p9235185665713"></a><a name="p9235185665713"></a>subscriber</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p129931625131815"><a name="p129931625131815"></a><a name="p129931625131815"></a> Input. </p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p223511561579"><a name="p223511561579"></a><a name="p223511561579"></a> Subscriber handle.</p>
</td>
</tr>
<tr id="row154052053135718"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p740513532574"><a name="p740513532574"></a><a name="p740513532574"></a>domain</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p1599514253189"><a name="p1599514253189"></a><a name="p1599514253189"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p1840545320573"><a name="p1840545320573"></a><a name="p1840545320573"></a> component. Currently, only Runtime is supported.</p>
</td>
</tr>
<tr id="row255615065712"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p1655711508571"><a name="p1655711508571"></a><a name="p1655711508571"></a>cbid</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p499642531816"><a name="p499642531816"></a><a name="p499642531816"></a>Enter </p>.
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p6557195055719"><a name="p6557195055719"></a><a name="p6557195055719"></a> callback ID, which identifies the calling point of the domain. The <a href="msptiCallbackIdRuntime.md">msptiCallbackIdRuntime</a> and <a href="msptiCallbackIdHccl.md">msptiCallbackIdHccl</a> functions can be called.</p>
</td>
</tr>
</tbody>
</table>

## Return Values<a name="section16621124213476"></a>

MSPTI_SUCCESS is returned if the operation is successful. MSPTI_ERROR_INVALID_PARAMETER is returned if the user, domain, or *cbid is invalid, indicating that the operation fails.
