# msptiActivitySourceKind<a name="ZH-CN_TOPIC_0000002048862745"></a>

Activity data source. either Host or Device.

msptiActivitySourceKind is an enumeration class used in the [msptiActivityMarker](msptiActivityMarker.md) structure. It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_SOURCE_KIND_HOST = 0, // Indicates that the data source is the Host.
    MSPTI_ACTIVITY_SOURCE_KIND_DEVICE = 1 // Indicates that the data source is the Device.
} msptiActivitySourceKind;
```
