# MstxMonitor.stop<a name="ZH-CN_TOPIC_0000002110867024"></a>

## Product Support <a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
> 
> For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| Product Type                                   | Supported|
| ------------------------------------------- | :------: |
| Ascend 950PR&950DT Products                  |    √     |
| Ascend A3 Products|    √     |
| Ascend A2 Products|    √     |
| Ascend 310B Products                 |    √     |
| Ascend 310P Products                         |    ×     |
| Ascend 910 Products                         |    ×     |

## Function Usage <a name="section463019538153"></a>

Marks the end of the mstx marker for profiling.

## Function Prototype<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Return Values<a name="section776014535188"></a>

If MsptiResult.MSPTI\_SUCCESS is returned, the operation is successful. If MsptiResult.MSPTI\_ERROR\_INVALID\_PARAMETER is returned, the operation fails.
