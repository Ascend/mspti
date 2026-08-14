# MsptiResult<a name="ZH-CN_TOPIC_0000002119450416"></a>

MsptiResult是msPTI返回的错误和结果代码，为枚举类。定义如下：

```python
class MsptiResult(Enum):
    MSPTI_SUCCESS = 0    # msPTI执行成功，无错误
    MSPTI_ERROR_INVALID_PARAMETER = 1    # 回调函数为NULL时返回，表示msPTI执行失败
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2    # 已存在msPTI用户时返回，表示msPTI执行失败
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3    # Activity Buffer没有更多的Record数据时返回，表示msPTI执行失败
    MSPTI_ERROR_DEVICE_OFFLINE = 4    # 无法获取DEVICE侧信息
    MSPTI_ERROR_QUEUE_EMPTY = 5    # External Correlation ID 匹配失败时返回，表示msPTI执行失败
    MSPTI_ERROR_WITHOUT_LD_PRELOAD = 6    # libmspti.so未设置到LD_PRELOAD环境变量时返回，表示msPTI执行失败
    MSPTI_ERROR_NOT_INITIALIZED = 7    # msPTI未初始化时返回，表示msPTI执行失败
    MSPTI_ERROR_INVALID_KIND = 8    # Activity Kind无效时返回，表示msPTI执行失败
    MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT = 9    # 参数缓冲区大小不足时返回，表示msPTI执行失败
    MSPTI_ERROR_INNER = 999    # msPTI内部错误时返回，表示msPTI执行失败
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
```
