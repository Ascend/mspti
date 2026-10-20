# msptiPmSamplingDecodeStopReason<a name="ZH-CN_TOPIC_0000002100000204"></a>

msptiPmSamplingDecodeStopReason为[msptiPmSamplingDecodeData](msptiPmSamplingDecodeData.md)调用的枚举类，表示解码停止的原因。定义如下：

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER = 0,   // 其他原因
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL,   // counter data image没有空闲样本槽位
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS,   // 全部记录解码完成
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNT,
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_FORCE_INT = 0x7fffffff
} msptiPmSamplingDecodeStopReason;
```
