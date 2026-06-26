# msptiActivityHccl<a name="ZH-CN_TOPIC_0000002086158432"></a>

msptiActivityHccl is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_HCCL](msptiActivityKind.md), defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;   // Activity record type MSPTI_ACTIVITY_KIND_HCCL
    uint64_t start;   // Start timestamp of the communication operator execution on the NPU device, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected
    uint64_t end;   // End timestamp of the communication operator execution, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected
    struct {
        uint32_t deviceId;   // Device ID of the device on which the communication operator runs
        uint32_t streamId;   // Stream ID of the communication operator execution stream
    } ds;
    uint64_t bandWidth;   // Bandwidth during communication operator execution, in GB/s
    const char *name;   // Name of the communication operator
    const char *commName;   // Name of the communication domain
} msptiActivityHccl;
```
