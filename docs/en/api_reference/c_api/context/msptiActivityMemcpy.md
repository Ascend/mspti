# `msptiActivityMemcpy`<a name="ZH-CN_TOPIC_0000002155584865"></a>

msptiActivityMemcpy is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_MEMCPY](msptiActivityKind.md). It is used to report the memcpy activity information. The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind; // Type of the memory copy operation (such as HTOD/Host to Device and DTOH/Device to Host, see the msptiActivityMemcpyKind enumeration)
    uint64_t bytes; // Number of bytes transferred by the memory copy operation
    uint64_t start; // Start timestamp of the memory copy operation, in ns
    uint64_t end; // End timestamp of the memory copy operation, in ns
    uint32_t deviceId; // ID of the device where the memory copy operation is performed
    uint32_t streamId; // Stream ID of the memory copy operation
    uint64_t correlationId; // Correlation ID of the memory copy operation, currently a reserved field that has no effect
    uint8_t isAsync; // Whether the operation is performed using an asynchronous memcpy API (such as aclrtMemcpyAsync), where 1 indicates asynchronous and 0 indicates synchronous
} msptiActivityMemcpy;
```

Currently, only some CANN Runtime memory copy APIs are supported for collection, as listed in the following table:

| API | Description |
| --- | --- |
| aclrtMemcpy | Synchronous memory copy |
| aclrtMemcpyAsync | Asynchronous memory copy |
| aclrtMemcpyAsyncWithCondition | Asynchronous memory copy with conditions |
| aclrtMemcpyBatch | Batched synchronous memory copy |
| aclrtMemcpyBatchAsync | Batched asynchronous memory copy |
| aclrtMemcpy2d | 2D synchronous memory copy |
| aclrtMemcpy2dAsync | 2D asynchronous memory copy |
| aclrtMemcpyAsyncWithOffset | Asynchronous memory copy with offset |
| aclrtMemcpyBatchV2 | Batched synchronous memory copy (V2) |
| aclrtMemcpyBatchAsyncV2 | Batched asynchronous memory copy (V2) |
