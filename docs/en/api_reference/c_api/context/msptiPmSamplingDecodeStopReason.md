# msptiPmSamplingDecodeStopReason<a name="ZH-CN_TOPIC_0000002100000204"></a>

msptiPmSamplingDecodeStopReason is an enumeration for [msptiPmSamplingDecodeData](msptiPmSamplingDecodeData.md), indicating the reason why decoding stopped. The definition is as follows:

```cpp
typedef enum {
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER = 0,   // Other reasons
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL,   // The counter data image has no free sample slot
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS,   // All records have been decoded
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNT,
    MSPTI_PM_SAMPLING_DECODE_STOP_REASON_FORCE_INT = 0x7fffffff
} msptiPmSamplingDecodeStopReason;
```
