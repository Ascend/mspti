# msptiPmSamplingObjectPtr<a name="ZH-CN_TOPIC_0000002100000123"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<!-- npu="950" id1 -->
- Ascend 950PR/Ascend 950DT products: supported
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

## Description<a name="section20806203412478"></a>

Handle of a PM sampling session object. The handle is created by [msptiPmSamplingEnable](msptiPmSamplingEnable.md), and all subsequent PM Sampling APIs use it to specify the target session.

## Function Prototype<a name="section1121883194711"></a>

```cpp
typedef struct msptiPmSamplingObject_st *msptiPmSamplingObjectPtr
```

## Return Values<a name="section16621124213476"></a>

None
