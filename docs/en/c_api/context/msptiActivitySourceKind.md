# msptiActivitySourceKind<a name="ZH-CN_TOPIC_0000002048862745"></a>

Activity data source. either host or device.

msptiActivitySourceKind is an enumeration class invoked in the [msptiActivityMarker](msptiActivityMarker.md) structure. It is defined as follows:

```cpp
typedef enum {
    MSPTI_ACTIVITY_SOURCE_KIND_HOST = 0, // Indicates that the data source is the host.
    MSPTI_ACTIVITY_SOURCE_KIND_DEVICE = 1 // Indicates that the data source is the device.
} msptiActivitySourceKind;
```
