# msptiResult<a name="ZH-CN_TOPIC_0000002060275809"></a>

msptiResult is an enumeration class for error and result codes returned by MSPTI. It is defined as follows:

```cpp
typedef enum {
    MSPTI_SUCCESS = 0,    // MSPTI executed successfully, no error
    MSPTI_ERROR_INVALID_PARAMETER = 1,    // Returned when funcBufferRequested or funcBufferCompleted is NULL, indicating MSPTI execution failure
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2,    // Returned when an MSPTI user already exists, indicating MSPTI execution failure
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3,    // Returned when the Activity Buffer has no more record data, indicating that MSPTI execution failed
    MSPTI_ERROR_DEVICE_OFFLINE = 4,    // Unable to obtain device-side information
    MSPTI_ERROR_INNER = 999,    // Returned when MSPTI cannot be initialized, indicating that MSPTI execution failed
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
} msptiResult;
```
