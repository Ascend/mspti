# msptiActivityEnableMarkerDomain<a name="ZH-CN_TOPIC_0000002188924710"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
> 
> For details about Ascend product models, see [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| Product Type                                    | Supported |
| ------------------------------------------- | :------: |
| Atlas 350 accelerator cards                   |    √     |
| Atlas A3 training products/Atlas A3 inference products |    √     |
| Atlas A2 training products/Atlas A2 inference products |    √     |
| Atlas 200I/500 A2 inference products                  |    √     |
| Atlas inference products                          |    ×     |
| Atlas training products                          |    ×     |

## Function<a name="section20806203412478"></a>

Enables the profiling of a marked domain.

You can call this API multiple times to enable profiling for multiple domains. By default, profiling is enabled for all domains.

## Prototype<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityEnableMarkerDomain(const char* name)
```

## Parameter Description<a name="section11506138144714"></a>

**Table 1** Parameter Description

<a name="table827101275518"></a>
<table><thead align="left"><tr id="row429121265517"><th class="cellrowborder" valign="top" width="28.65286528652865%" id="mcps1.2.4.1.1"><p id="p1329121214558"><a name="p1329121214558"></a><a name="p1329121214558"></a>Parameter</p>
</th>
<th class="cellrowborder" valign="top" width="13.661366136613662%" id="mcps1.2.4.1.2"><p id="p10230141454318"><a name="p10230141454318"></a><a name="p10230141454318"></a>Input/Output</p>
</th>
<th class="cellrowborder" valign="top" width="57.68576857685769%" id="mcps1.2.4.1.3"><p id="p83121275519"><a name="p83121275519"></a><a name="p83121275519"></a>Description</p>
</th>
</tr>
</thead>
<tbody><tr id="row1131131265511"><td class="cellrowborder" valign="top" width="28.65286528652865%" headers="mcps1.2.4.1.1 "><p id="p7669321185110"><a name="p7669321185110"></a><a name="p7669321185110"></a>name</p>
</td>
<td class="cellrowborder" valign="top" width="13.661366136613662%" headers="mcps1.2.4.1.2 "><p id="p18311931172217"><a name="p18311931172217"></a><a name="p18311931172217"></a>Input</p>
</td>
<td class="cellrowborder" valign="top" width="57.68576857685769%" headers="mcps1.2.4.1.3 "><p id="p712111021"><a name="p712111021"></a><a name="p712111021"></a>Name of the corresponding marked domain.</p>
</td>
</tr>
</tbody>
</table>

## Returns<a name="section16621124213476"></a>

Returns `MSPTI_SUCCESS` on success, or `MSPTI_ERROR_INVALID_PARAMETER` if name is an empty string, indicating failure.
