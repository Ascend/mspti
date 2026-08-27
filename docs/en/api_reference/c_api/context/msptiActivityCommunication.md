# msptiActivityCommunication<a name="ZH-CN_TOPIC_0000002305037540"></a>

msptiActivityCommunication is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_COMMUNICATION](msptiActivityKind.md). It is used to associate Activity Records and is defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_COMMUNICATION
    msptiCommunicationDataType dataType; // Data type of the communication operator
    uint64_t count; // Data volume of the communication operator
    struct {
        uint32_t deviceId; // Device ID of the device where the communication operator runs
        uint32_t streamId; // Stream ID of the stream where the communication operator runs
    } ds;
    uint64_t start; // Start timestamp of the communication operator execution on the NPU, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    uint64_t end; // End timestamp of the communication operator execution, in ns. If both the start and end timestamps are 0, the timestamp information of the communication operator cannot be collected.
    const char* algType; // Algorithm used by the communication operator
    const char* name; // Name of the communication operator
    const char* commName; // Name of the communicator where the communication operator is located
    uint64_t correlationId; // Unique ID generated during the execution of the communication operator. Other activities can be associated with the communication operator using this value.
} msptiActivityCommunication;
```
