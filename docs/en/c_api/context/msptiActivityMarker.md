# msptiActivityMarker<a name="ZH-CN_TOPIC_0000002009745268"></a>

msptiActivityMarker is the structure corresponding to the activity record type [MSPTI\_ACTIVITY\_KIND\_MARKER](msptiActivityKind.md), defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind;   // Activity record type MSPTI_ACTIVITY_KIND_MARKER
    msptiActivityFlag flag;   // Flag for the marker
    msptiActivitySourceKind sourceKind;   // Source type of the marker data
    uint64_t timestamp;   // Timestamp of the marker, in ns. A value of 0 indicates that timestamp information cannot be collected for the marker
    uint64_t id;   // Marker ID
    msptiObjectId objectId;   // Process ID, thread ID, device ID, and stream ID for identifying the marker
    const char *name;   // Name of the marker. The value is NULL for the end marker
    const char *domain;   // Name of the domain to which the marker belongs. The default domain is NULL
} msptiActivityMarker;
```
