# `msptiActivityOverhead`<a name="ZH-CN_TOPIC_0000002120186622"></a>

msptiActivityOverhead is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_OVERHEAD](msptiActivityKind.md). It is used to report the additional overhead that msPTI itself generates during collection, for example msPTI internal resource creation and destruction or Activity Buffer requests and flushes. The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;    // Activity Record type MSPTI_ACTIVITY_KIND_OVERHEAD
    msptiActivityOverheadKind overheadKind;    // Type of overhead included, indicating the phase in which the overhead is generated
    msptiObjectId objectId;    // Identifier of the Activity object, whose valid fields are determined by objectKind
    uint64_t start;    // Start timestamp of the overhead operation, in ns
    uint64_t end;    // End timestamp of the overhead operation, in ns
    uint64_t correlationId;    // Correlation ID of the overhead operation, currently a reserved field that has no effect
    void *overheadData;    // Pointer to additional information about the overhead operation, currently a reserved field that has no effect
    msptiActivityObjectKind objectKind;    // Type of Activity object associated with the overhead
} msptiActivityOverhead;
```

The overhead types currently recorded are as follows:

| overheadKind | Description |
| --- | --- |
| MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE | Overhead of creating and destroying msPTI internal resources (for example, enabling or disabling a collection task) |
| MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST | Overhead of requesting an Activity Buffer (the time taken to invoke the Request callback to request a buffer) |
| MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH | Overhead of flushing an Activity Buffer (the time taken to invoke the Complete callback to return a buffer) |
