# `MsptiResult`<a name="ZH-CN_TOPIC_0000002119450416"></a>

`MsptiResult` enumerates the error and result codes returned by msPTI. The definition is as follows:

```python
class MsptiResult(Enum):
    MSPTI_SUCCESS = 0 # msPTI is successfully executed and no error occurs
    MSPTI_ERROR_INVALID_PARAMETER = 1 # msPTI fails to be executed when the callback function is NULL
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2 # msPTI fails to be executed when there is already an msPTI user
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3 # msPTI fails to be executed when the activity buffer does not have more record data
    MSPTI_ERROR_DEVICE_OFFLINE = 4 # The information on the device side cannot be obtained
    MSPTI_ERROR_QUEUE_EMPTY = 5 # msPTI fails to be executed when the external correlation ID does not match
    MSPTI_ERROR_WITHOUT_LD_PRELOAD = 6 # msPTI fails to be executed when libmspti.so is not set in the LD_PRELOAD environment variable
    MSPTI_ERROR_NOT_INITIALIZED = 7 # msPTI fails to be executed when msPTI is not initialized
    MSPTI_ERROR_INVALID_KIND = 8 # msPTI fails to be executed when the Activity Kind is invalid
    MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT = 9 # msPTI fails to be executed when the size of the parameter buffer is insufficient
    MSPTI_ERROR_INNER = 999 # msPTI fails to be executed when an internal error occurs in msPTI
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
```
