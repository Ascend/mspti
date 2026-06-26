# HcclMonitor.flush\_all<a name="ZH-CN_TOPIC_0000002108084152"></a>

## Supported Products<a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
> 
> For details about Ascend product models, see [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

| Product Type                                    | Supported |
| ------------------------------------------- | :------: |
| Atlas 350 accelerator cards                   |    √     |
| Atlas A3 training products/Atlas A3 inference products |    √     |
| Atlas A2 training products/Atlas A2 inference products |    √     |
| Atlas 200I/500 A2 inference products                  |    √     |
| Atlas inference products                          |    ×     |
| Atlas training products                          |    ×     |

## Function<a name="section463019538153"></a>

The user (subscriber) calls the callback function to write all activity data (including communication, kernel, and msTX data) in the buffer to the user memory.

## Prototype<a name="section759854510169"></a>

```python
def flush_all(cls) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Returns<a name="section776014535188"></a>

None
