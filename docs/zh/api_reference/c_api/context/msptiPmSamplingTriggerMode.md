# msptiPmSamplingTriggerMode<a name="ZH-CN_TOPIC_0000002100000203"></a>

msptiPmSamplingTriggerMode为[msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md)调用的枚举类，表示PM采样的触发模式。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL = 0,   // 固定时间间隔周期触发
    MSPTI_PM_SAMPLING_TRIGGER_MODE_COUNT,
    MSPTI_PM_SAMPLING_TRIGGER_MODE_FORCE_INT = 0x7fffffff
} msptiPmSamplingTriggerMode;
```
