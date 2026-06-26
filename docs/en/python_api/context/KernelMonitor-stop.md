# KernelMonitor.stop<a name="ZH-CN_TOPIC_0000002108084160"></a>

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

Marks the end of Kernel performance data collection.

## Prototype<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Returns<a name="section776014535188"></a>

Returns `MsptiResult.MSPTI_SUCCESS` on success, or `MsptiResult.MSPTI_ERROR_INVALID_PARAMETER` on failure.
