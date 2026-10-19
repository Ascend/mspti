# `msptiActivityAttribute`<a name="ZH-CN_TOPIC_0000002091015000"></a>

msptiActivityAttribute is an enumeration class invoked by [msptiActivitySetAttribute](./msptiActivitySetAttribute.md) and [msptiActivityGetAttribute](./msptiActivityGetAttribute.md), and it identifies the configurable attributes used during Activity collection. It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE = 0,   // Channel Buffer size, in bytes, data type uint32_t, default value 2 × 1024 × 1024, value range [2MB, 10MB]
    MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK = 1,   // Timestamp callback function, data type msptiTimestampCallbackFunc
    MSPTI_ACTIVITY_ATTR_FORCE_INT = 0x7fffffff     // Force type identifier, used to ensure that the enumeration class occupies the full 32 bits. It cannot be passed to interfaces for use
} msptiActivityAttribute;
```

`MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE` takes effect only on Channel Buffers created after the setting, and it has no effect on Channels that have already been created. You are advised to complete the setting before enabling Activity collection.
The Channel Buffer caches Activity Record data. Setting the Channel Buffer size appropriately balances collection performance and memory usage.
