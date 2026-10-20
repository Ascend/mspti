# msptiPmSamplingBufferAppendMode<a name="ZH-CN_TOPIC_0000002100000202"></a>

msptiPmSamplingBufferAppendMode is an enumeration for [msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md), indicating the append mode for new samples when the hardware buffer is full. The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST = 0,   // Keep the oldest samples and drop new samples
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST = 1,   // Keep the latest samples and overwrite the oldest samples
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_COUNT,
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_FORCE_INT = 0x7fffffff
} msptiPmSamplingBufferAppendMode;
```
