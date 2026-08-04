# msptiActivityMemoryKind<a name="ZH-CN_TOPIC_0000002155584861"></a>

请求的内存类型。msptiActivityMemoryKind为[msptiActivityMemory](./msptiActivityMemory.md)调用的枚举类，定义如下：

```cpp
typedef enum {
    MSPTI_ACTIVITY_MEMORY_UNKNOWN = 0,    // 未知的内存类型
    MSPTI_ACTIVITY_MEMORY_DEVICE = 1,    // 设备侧内存
    MSPTI_ACTIVITY_MEMORY_HOST = 2,    // 主机侧内存
    MSPTI_ACTIVITY_MEMORY_MANAGED = 3    // 统一管理内存（由系统自动迁移）
} msptiActivityMemoryKind;
```
