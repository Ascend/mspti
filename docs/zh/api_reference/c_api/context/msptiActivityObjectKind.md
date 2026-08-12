# msptiActivityObjectKind<a name="ZH-CN_TOPIC_0000002155584861"></a>

Activity对象类型。msptiActivityObjectKind为[msptiActivityOverhead](./msptiActivityOverhead.md)调用的枚举类，用于标识Activity记录关联的对象类型，并指示msptiObjectId中有效的字段，定义如下：

```cpp
typedef enum {
    MSPTI_ACTIVITY_OBJECT_UNKNOWN = 0,    // 未知的对象类型
    MSPTI_ACTIVITY_OBJECT_PROCESS = 1,    // 对象为进程，msptiObjectId的pt成员有效
    MSPTI_ACTIVITY_OBJECT_THREAD = 2,    // 对象为线程，msptiObjectId的pt成员有效
    MSPTI_ACTIVITY_OBJECT_DEVICE = 3,    // 对象为设备，msptiObjectId的ds成员有效
    MSPTI_ACTIVITY_OBJECT_FORCE_INT = 0x7fffffff
} msptiActivityObjectKind;
```
