# msptiActivity<a name="ZH-CN_TOPIC_0000002048904921"></a>

**msptiActivity** is the basic struct of an activity record. The Activity API uses **msptiActivity** as the general representation of activity. The **kind** field determines the specific activity type. In this way, the **msptiActivity** object can be converted to a specific activity record type suitable for this type. The definition is as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity type.
} msptiActivity;
```
