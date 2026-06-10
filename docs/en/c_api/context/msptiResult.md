# msptiResult<a name="ZH-CN_TOPIC_0000002060275809"></a>

**msptiResult** enumerates the error and result codes returned by MSPTI. The definition is as follows:

```cpp
typedef enum {
    MSPTI_SUCCESS = 0, // The MSPTI is successfully executed and no error occurs.
    MSPTI_ERROR_INVALID_PARAMETER = 1, // The MSPTI fails to be executed when funcBufferRequested or funcBufferCompleted is NULL.
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2, // The MSPTI fails to be executed when there is already an MSPTI user.
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3, // The MSPTI fails to be executed when the activity buffer does not have more record data.
    MSPTI_ERROR_DEVICE_OFFLINE = 4, // The information on the device side cannot be obtained.
    MSPTI_ERROR_INNER = 999, // The MSPTI fails to be executed when the MSPTI cannot be initialized.
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
} msptiResult;
```
