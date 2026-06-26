# msptiActivitySourceKind<a name="ZH-CN_TOPIC_0000002048862745"></a>

Marks the data source of an activity. Marks whether the data comes from the host or the device.

msptiActivitySourceKind is an enumeration class called in the [msptiActivityMarker](msptiActivityMarker.md) structure. The definition is as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_SOURCE_KIND_HOST = 0,   // Marks that the data comes from the host
    MSPTI_ACTIVITY_SOURCE_KIND_DEVICE = 1   // Marks that the data comes from the device
} msptiActivitySourceKind;
```
