# msptiActivityOverheadKind<a name="ZH-CN_TOPIC_0000002155584861"></a>

开销类型。msptiActivityOverheadKind为[msptiActivityOverhead](./msptiActivityOverhead.md)调用的枚举类，用于标识msPTI开销产生的环节，定义如下：

```cpp
typedef enum {
    MSPTI_ACTIVITY_OVERHEAD_UNKNOWN = 0,    // 未知的开销类型
    MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE = 1,    // msPTI内部资源的创建与销毁开销
    MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST = 2,    // Activity Buffer申请开销
    MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH = 3,    // Activity Buffer刷新开销
} msptiActivityOverheadKind;
```
