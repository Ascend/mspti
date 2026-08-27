# msptiActivityHccl<a name="ZH-CN_TOPIC_0000002086158432"></a>

msptiActivityHccl is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_HCCL](msptiActivityKind.md). The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_HCCL
    uint64_t start; // Start timestamp of the communication operator execution on the NPU, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    uint64_t end; // End timestamp of the communication operator execution, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    struct {
        uint32_t deviceId; // Device ID of the device where the communication operator runs
        uint32_t streamId; // Stream ID of the stream where the communication operator runs
    } ds;
    uint64_t bandWidth; // Bandwidth of the communication operator, in GB/s
    const char *name; // Name of the communication operator
    const char *commName; // Name of the communicator domain.
} msptiActivityHccl;
```
