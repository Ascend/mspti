# HcclMonitor.stop<a name="ZH-CN_TOPIC_0000002108243904"></a>

## Product Support <a name="zh-cn_topic_0000002111094444_section5889102116569"></a>

> [!NOTE]
>
> For details about Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002143882701_table38301303189"></a>

<!-- npu="950" id1 -->
- Ascend 950PR and 950DT products: supported
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 products: supported
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 products: supported
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 inference products: supported
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas inference products: not supported
<!-- end id5 -->
<!-- npu="910" id6 -->
- Atlas training products: not supported
<!-- end id6 -->

## Function Usage <a name="section463019538153"></a>

Marks the end of the communication operator profiling.

## Function Prototype<a name="section759854510169"></a>

```python
def stop(self) -> MsptiResult:
```

## Parameter Description<a name="section354791521716"></a>

None

## Returns<a name="section776014535188"></a>

If `MsptiResult.MSPTI_SUCCESS` is returned, the operation is successful. If `MsptiResult.MSPTI_ERROR_INVALID_PARAMETER` is returned, the operation fails.
