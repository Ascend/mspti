# msptiActivitySetAttribute<a name="ZH-CN_TOPIC_0000002091015001"></a>

## 产品支持情况<a name="section8178181118225"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413733_table38301303189"></a>

| 产品类型                                    | 是否支持 |
| ------------------------------------------- | :------: |
| 昇腾950PR&950DT系列产品                         |    √     |
| 昇腾A3系列产品 |    √     |
| 昇腾A2系列产品 |    √     |
| 昇腾310B系列产品                  |    √     |
| 昇腾310P系列产品                          |    ×     |
| 昇腾910系列产品                          |    ×     |

## 功能说明<a name="section20806203412478"></a>

设置指定Activity属性value的值，用于配置Activity采集行为。当前支持设置的属性包括[msptiActivityAttribute](./msptiActivityAttribute.md)中的MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE和MSPTI\_ACTIVITY\_ATTR\_TIMESTAMP\_CALLBACK。

- 设置MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE时，value指向的数据类型为uint32\_t，表示Channel Buffer的大小，单位为Byte，取值范围为\[2MB, 10MB\]，超出取值范围时设置失败。
- 设置MSPTI\_ACTIVITY\_ATTR\_TIMESTAMP\_CALLBACK时，value指向的数据类型为[msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md)，用于注册自定义时间戳回调函数，功能等同于[msptiActivityRegisterTimestampCallback](./msptiActivityRegisterTimestampCallback.md)接口。

> [!NOTE]
>
> MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE仅在设置之后创建的Channel Buffer上生效，对已创建的Channel无效。建议在启用Activity采集之前完成设置。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivitySetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| attr | 输入 | 需要设置的Activity属性，配置为[msptiActivityAttribute](./msptiActivityAttribute.md)的枚举值。 |
| valueSize | 输入 | 指定value缓冲区的大小，单位为Byte。值小于attr对应属性所需大小时，接口返回MSPTI\_ERROR\_PARAMETER\_SIZE\_NOT\_SUFFICIENT。 |
| value | 输入 | 指向待设置的属性值缓冲区，缓冲区大小由valueSize指定。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；valueSize或value为NULL时，返回MSPTI\_ERROR\_INVALID\_PARAMETER；value缓冲区大小小于属性所需大小时，返回MSPTI\_ERROR\_PARAMETER\_SIZE\_NOT\_SUFFICIENT，表示失败；attr非法或属性值非法（如Channel Buffer大小超出取值范围、时间戳回调函数为NULL）时，返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
