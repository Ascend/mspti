# msptiPmSamplingObjectPtr<a name="ZH-CN_TOPIC_0000002100000123"></a>

## Supported Products<a name="section8178181118225"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

| Product Type | Supported |
| ------------------------------------------- | :------: |
| Ascend 950 products | √ |
| Atlas A3 training products/Atlas A3 inference products | √ |
| Atlas A2 training products/Atlas A2 inference products | √ |
| Atlas 200I/500 A2 inference products | √ |
| Atlas inference products | × |
| Atlas training products | × |

## Description<a name="section20806203412478"></a>

Handle of a PM sampling session object. The handle is created by [msptiPmSamplingEnable](msptiPmSamplingEnable.md), and all subsequent PM Sampling APIs use it to specify the target session.

## Function Prototype<a name="section1121883194711"></a>

```cpp
typedef struct msptiPmSamplingObject_st *msptiPmSamplingObjectPtr
```

## Return Values<a name="section16621124213476"></a>

None
