# msptiActivityMemset<a name="ZH-CN_TOPIC_0000002155706481"></a>

msptiActivityMemset为Activity Record类型[MSPTI\_ACTIVITY\_KIND\_MEMSET](msptiActivityKind.md)对应的结构体，用于上报Memset Activity信息，定义如下：

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record类型MSPTI_ACTIVITY_KIND_MEMSET
    uint32_t value;    // Memset操作设置的目标值
    uint64_t bytes;    // Memset设置的字节数
    uint64_t start;    //  Memset操作的开始时间戳，单位ns
    uint64_t end;    //  Memset操作的结束时间戳，单位ns
    uint32_t deviceId;    // Memset操作所在的设备ID
    uint32_t streamId;    // Memset操作的流ID
    uint64_t correlationId;    // Memset操作的关联ID，当前为预留字段，暂未生效
    uint8_t isAsync;    // 是否通过异步Memset API（如aclrtMemsetAsync）进行操作，1表示异步，0表示同步
} msptiActivityMemset;
```

当前仅支持采集部分 CANN Runtime 内存设置 API，列表如下：

| API | 说明 |
| --- | --- |
| `aclrtMemset` | 同步内存设置 |
| `aclrtMemsetAsync` | 异步内存设置 |
| `aclrtMemsetD32` | 以32位整数进行内存设置 |
| `aclrtMemsetD32Async` | 以32位整数异步内存设置 |
