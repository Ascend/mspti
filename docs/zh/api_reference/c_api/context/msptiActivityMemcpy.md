# msptiActivityMemcpy<a name="ZH-CN_TOPIC_0000002155584865"></a>

msptiActivityMemcpy为Activity Record类型[MSPTI\_ACTIVITY\_KIND\_MEMCPY](msptiActivityKind.md)对应的结构体，用于上报Memcpy Activity信息，定义如下：

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record类型MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind;    // 内存拷贝操作的类型（如HTOD/Host到Device、DTOH/Device到Host等，详见msptiActivityMemcpyKind枚举）
    uint64_t bytes;    // 内存拷贝操作传输的字节数
    uint64_t start;    // 内存拷贝操作的开始时间戳，单位ns
    uint64_t end;    // 内存拷贝操作的结束时间戳，单位ns
    uint32_t deviceId;    // 内存拷贝操作所在的设备ID
    uint32_t streamId;    // 内存拷贝操作的流ID
    uint64_t correlationId;    // 内存拷贝操作的关联ID，当前为预留字段，暂未生效
    uint8_t isAsync;    // 是否通过异步Memcpy API（如aclrtMemcpyAsync）进行操作，1表示异步，0表示同步
} msptiActivityMemcpy;
```

当前仅支持采集部分 CANN Runtime 内存拷贝 API，列表如下：

| API | 说明 |
| --- | --- |
| `aclrtMemcpy` | 同步内存拷贝 |
| `aclrtMemcpyAsync` | 异步内存拷贝 |
| `aclrtMemcpyAsyncWithCondition` | 带条件的异步内存拷贝 |
| `aclrtMemcpyBatch` | 批量同步内存拷贝 |
| `aclrtMemcpyBatchAsync` | 批量异步内存拷贝 |
| `aclrtMemcpy2d` | 2D同步内存拷贝 |
| `aclrtMemcpy2dAsync` | 2D异步内存拷贝 |
| `aclrtMemcpyAsyncWithOffset` | 带偏移的异步内存拷贝 |
| `aclrtMemcpyBatchV2` | 批量同步内存拷贝（V2版本） |
| `aclrtMemcpyBatchAsyncV2` | 批量异步内存拷贝（V2版本） |
