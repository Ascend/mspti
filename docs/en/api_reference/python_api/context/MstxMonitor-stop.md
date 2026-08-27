# MstxMonitor.stop<a name="ZH-CN_TOPIC_0000002110867024"></a>

## Product Support <a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
>
> For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950 products           |    √     |
| Atlas A3 training products/Atlas A3 inference products|    √     |
| Atlas A2 training products/Atlas A2 inference products|    √     |
| Atlas 200I/500 A2 inference products                 |    √     |
| Atlas inference products                         |    ×     |
| Atlas training products                         |    ×     |

## Function Usage <a name="section463019538153"></a>

Marks the end of the msTX marker for profiling.

## Function Prototype<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Returns<a name="section776014535188"></a>

If `MsptiResult.MSPTI_SUCCESS` is returned, the operation is successful. If `MsptiResult.MSPTI_ERROR_INVALID_PARAMETER` is returned, the operation fails.
