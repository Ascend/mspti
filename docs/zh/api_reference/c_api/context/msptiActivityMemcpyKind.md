# msptiActivityMemcpyKind<a name="ZH-CN_TOPIC_0000002120344714"></a>

内存拷贝类型。msptiActivityMemcpyKind为[msptiActivityMemcpy](msptiActivityMemcpy.md)调用的枚举类，定义如下：

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMCPY_KIND_UNKNOWN = 0,    // 未知的内存拷贝类型
    MSPTI_ACTIVITY_MEMCPY_KIND_HTOH = 1,    // Host到Host的内存拷贝
    MSPTI_ACTIVITY_MEMCPY_KIND_HTOD = 2,    // Host到Device的内存拷贝
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOH = 3,    // Device到Host的内存拷贝
    MSPTI_ACTIVITY_MEMCPY_KIND_DTOD = 4,    // Device到Device的内存拷贝
    MSPTI_ACTIVITY_MEMCPY_KIND_DEFAULT = 5    // 系统根据地址自动推断的拷贝方向
} msptiActivityMemcpyKind;
```
