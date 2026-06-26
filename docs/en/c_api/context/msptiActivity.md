# msptiActivity<a name="ZH-CN_TOPIC_0000002048904921"></a>

msptiActivity is the base structure for an activity record. The Activity API uses msptiActivity as the general representation of an activity. The `kind` field is used to determine the specific activity type, allowing an msptiActivity object to be cast to the specific activity record type appropriate for that type. It is defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;   // Activity types
} msptiActivity;
```
