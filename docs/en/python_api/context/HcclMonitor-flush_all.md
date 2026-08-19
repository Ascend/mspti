# HcclMonitor.flush\_all<a name="ZH-CN_TOPIC_0000002108084152"></a>

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

The user (subscriber) calls the callback function to transfer all activity data (including communication, kernel, and mstx data) from the buffer to the user memory.

## Function Prototype<a name="section759854510169"></a>

```python
def flush_all(cls) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Return Values<a name="section776014535188"></a>

None
