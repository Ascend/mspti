# msptiResult<a name="ZH-CN_TOPIC_0000002060275809"></a>

`msptiResult` enumerates the error and result codes returned by msPTI. The definition is as follows:

```cpp
typedef enum {
    MSPTI_SUCCESS = 0, // msPTI is successfully executed and no error occurs
    MSPTI_ERROR_INVALID_PARAMETER = 1, // msPTI fails to be executed when funcBufferRequested or funcBufferCompleted is NULL
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2, // msPTI fails to be executed when there is already an msPTI user
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3, // msPTI fails to be executed when the activity buffer does not have more record data
    MSPTI_ERROR_DEVICE_OFFLINE = 4, // The information on the device side cannot be obtained
    MSPTI_ERROR_QUEUE_EMPTY = 5,    // Returned when External Correlation ID matching fails, indicating that msPTI fails to be executed
    MSPTI_ERROR_WITHOUT_LD_PRELOAD = 6,    // Returned when libmspti.so is not set in the LD_PRELOAD environment variable, indicating that msPTI fails to be executed
    MSPTI_ERROR_INNER = 999, // msPTI fails to be executed when msPTI cannot be initialized
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
} msptiResult;
```
