# msptiPmSamplingBufferAppendMode<a name="ZH-CN_TOPIC_0000002100000202"></a>

msptiPmSamplingBufferAppendMode为[msptiPmSamplingSetConfig](msptiPmSamplingSetConfig.md)调用的枚举类，表示硬件缓冲区满时新样本的追加模式。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST = 0,   // 保留最旧样本，丢弃新样本
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST = 1,   // 保留最新样本，覆盖最旧样本
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_COUNT,
    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_FORCE_INT = 0x7fffffff
} msptiPmSamplingBufferAppendMode;
```
