# msptiPmSamplingTriggerMode<a name="ZH-CN_TOPIC_0000002100000203"></a>

msptiPmSamplingTriggerMode is an enumeration for [msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md), indicating the trigger mode of PM sampling. The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL = 0,   // Periodic triggering at a fixed time interval
    MSPTI_PM_SAMPLING_TRIGGER_MODE_COUNT,
    MSPTI_PM_SAMPLING_TRIGGER_MODE_FORCE_INT = 0x7fffffff
} msptiPmSamplingTriggerMode;
```
