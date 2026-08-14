# msptiActivityAttribute<a name="ZH-CN_TOPIC_0000002091015000"></a>

msptiActivityAttribute为[msptiActivitySetAttribute](./msptiActivitySetAttribute.md)和[msptiActivityGetAttribute](./msptiActivityGetAttribute.md)调用的枚举类，用于标识Activity采集过程中的可配置属性。定义如下：

```cpp
typedef enum {
    MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE = 0,   // Channel Buffer大小，单位为Byte，数据类型为uint32_t，默认值为2*1024*1024，取值范围为[2MB, 10MB]
    MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK = 1,   // 时间戳回调函数，数据类型为msptiTimestampCallbackFunc
    MSPTI_ACTIVITY_ATTR_FORCE_INT = 0x7fffffff     // 强制类型标识，用于保证枚举类占满32位，不可传入接口使用。
} msptiActivityAttribute;
```

MSPTI\_ACTIVITY\_ATTR\_CHANNEL\_BUFFER\_SIZE仅在设置之后创建的Channel Buffer上生效，对已创建的Channel无效。建议在启用Activity采集之前完成设置。
Channel Buffer用于缓存Activity Record数据，合理配置Channel Buffer大小可以平衡采集性能与内存占用。
