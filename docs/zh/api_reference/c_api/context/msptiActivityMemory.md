# msptiActivityMemory<a name="ZH-CN_TOPIC_0000002120186622"></a>

msptiActivityMemory为Activity Record类型[MSPTI\_ACTIVITY\_KIND\_MEMORY](msptiActivityKind.md)对应的结构体，用于上报Memory Activity信息，定义如下：

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record类型MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType;    // 用户请求（分配或释放）内存操作
    msptiActivityMemoryKind memoryKind;    // 请求的内存类型
    uint64_t correlationId;    // 内存请求操作的关联ID，当前为预留字段，暂未生效
    uint64_t start;    // 内存请求操作的开始时间戳，单位ns
    uint64_t end;    // 内存请求操作的结束时间戳，单位ns
    uint64_t address;    // 请求的内存地址
    uint64_t bytes;    // 内存请求操作申请的内存字节数
    uint32_t processId;    // 内存请求操作所属的进程ID
    uint32_t deviceId;    // 内存请求操作所在的设备ID
    uint32_t streamId;    // 内存请求操作的流ID，若内存请求操作为同步，则流ID设置为MSPTI_INVALID_STREAM_ID
} msptiActivityMemory;
```

当前仅支持采集部分 CANN Runtime 内存管理 API，列表如下：

| API | 说明 |
| --- | --- |
| `aclrtMalloc` | 分配Device内存 |
| `aclrtMallocAlign32` | 分配32字节对齐的Device内存 |
| `aclrtMallocCached` | 分配带缓存的Device内存 |
| `aclrtMallocWithCfg` | 按配置分配Device内存 |
| `aclrtMallocForTaskScheduler` | 为任务调度器分配内存 |
| `aclrtFree` | 释放Device内存 |
| `aclrtFreeWithDevSync` | 同步后释放Device内存 |
| `aclrtMallocHost` | 分配Host内存 |
| `aclrtFreeHost` | 释放Host内存 |
| `aclrtMallocHostWithCfg` | 按配置分配Host内存 |
| `aclrtFreeHostWithDevSync` | 同步后释放Host内存 |
| `aclrtReserveMemAddress` | 预留内存地址 |
| `aclrtReserveMemAddressNoUCMemory` | 预留内存地址（不开启统一内存） |
| `aclrtReleaseMemAddress` | 释放预留的内存地址 |
| `aclrtMemAllocManaged` | 分配统一管理内存 |
| `aclrtMallocPhysical` | 分配物理内存 |
| `aclrtFreePhysical` | 释放物理内存 |
