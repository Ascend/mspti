# MsptiResult<a name="ZH-CN_TOPIC_0000002119450416"></a>

MsptiResult is the error and result codes returned by MSPTI, enumeration class. It is defined as follows:

```python
class MsptiResult(Enum):
    MSPTI_SUCCESS = 0    # MSPTI executed successfully, no error
    MSPTI_ERROR_INVALID_PARAMETER = 1    # Returned when the callback function is NULL, indicating MSPTI execution failure
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2    # Returned when an MSPTI user already exists, indicating MSPTI execution failure
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3    # Returned when the activity buffer has no more record data, indicating MSPTI execution failure
    MSPTI_ERROR_DEVICE_OFFLINE = 4    # Unable to obtain device-side information
    MSPTI_ERROR_INNER = 999    # Returned when MSPTI cannot be initialized, indicating MSPTI execution failure
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
```
