# msptiActivityOverhead<a name="ZH-CN_TOPIC_0000002120186622"></a>

msptiActivityOverhead为Activity Record类型[MSPTI\_ACTIVITY\_KIND\_OVERHEAD](msptiActivityKind.md)对应的结构体，用于上报msPTI自身在采集过程中产生的额外开销，如msPTI内部资源创建/销毁、Activity Buffer请求/刷新等，定义如下：

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record类型MSPTI_ACTIVITY_KIND_OVERHEAD
    msptiActivityOverheadKind overheadKind;    // 纳入开销的类型，表明开销产生的环节
    msptiObjectId objectId;    // Activity对象的标识，取值字段由objectKind决定
    uint64_t start;    // 开销操作的开始时间戳，单位ns
    uint64_t end;    // 开销操作的结束时间戳，单位ns
    uint64_t correlationId;    // 开销操作的关联ID，当前为预留字段，暂未生效
    void *overheadData;    // 开销操作附加信息的指针，当前为预留字段，暂未生效
    msptiActivityObjectKind objectKind;    // 开销关联的Activity对象类型
} msptiActivityOverhead;
```

当前记录的开销类型如下：

| overheadKind | 说明 |
| --- | --- |
| `MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE` | msPTI内部资源的创建与销毁开销（如开启/关闭采集任务） |
| `MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST` | Activity Buffer申请开销（调用Request回调申请缓冲区的耗时） |
| `MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH` | Activity Buffer刷新开销（调用Complete回调返回缓冲区的耗时） |
