# msptiActivityGetAttribute<a name="ZH-CN_TOPIC_0000002091015002"></a>

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

获取指定Activity属性value的值，用于查询Activity采集的当前配置。当前支持获取的属性包括[msptiActivityAttribute](./msptiActivityAttribute.md)中的MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE和MSPTI\_ACTIVITY\_ATTR\_TIMESTAMP\_CALLBACK。

- 获取MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE时，返回当前Channel Buffer的大小，单位为Byte，数据类型为uint32\_t。
- 获取MSPTI\_ACTIVITY\_ATTR\_TIMESTAMP\_CALLBACK时，返回当前注册的时间戳回调函数，数据类型为[msptiTimestampCallbackFunc](./msptiTimestampCallbackFunc.md)。

## 函数原型<a name="section1121883194711"></a>

```cpp
msptiResult msptiActivityGetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
```

## 参数说明<a name="section11506138144714"></a>

**表 1**  参数说明

| 参数名 | 输入/输出 | 说明 |
| --- | --- | --- |
| attr | 输入 | 需要获取的Activity属性，配置为[msptiActivityAttribute](./msptiActivityAttribute.md)的枚举值。 |
| valueSize | 输入/输出 | 输入时指定value缓冲区的大小，单位为Byte；接口成功返回后，更新为实际写入value的属性值大小。 |
| value | 输出 | 指向接收属性值的缓冲区，缓冲区大小由valueSize指定。 |

## 返回值说明<a name="section16621124213476"></a>

返回MSPTI\_SUCCESS表示成功；valueSize或value为NULL时，返回MSPTI\_ERROR\_INVALID\_PARAMETER；value缓冲区大小小于属性所需大小时，返回MSPTI\_ERROR\_PARAMETER\_SIZE\_NOT\_SUFFICIENT，表示失败；attr非法时，返回MSPTI\_ERROR\_INVALID\_PARAMETER，表示失败。
