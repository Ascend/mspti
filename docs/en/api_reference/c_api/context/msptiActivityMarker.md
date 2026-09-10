# msptiActivityMarker<a name="ZH-CN_TOPIC_0000002009745268"></a>

msptiActivityMarker is the structure corresponding to the Activity Record type [MSPTI\_ACTIVITY\_KIND\_MARKER](msptiActivityKind.md). It is defined as follows:

```cpp
typedef struct PACKED_ALIGNMENT {
    msptiActivityKind kind; // Activity Record type MSPTI_ACTIVITY_KIND_MARKER
    msptiActivityFlag flag; // Flag for marking
    msptiActivitySourceKind sourceKind; // Source type of the marker data
    uint64_t timestamp; // Timestamp of the marker, in ns. If the value is 0, the timestamp information cannot be collected for the marker.
    uint64_t id; // ID of the marker
    msptiObjectId objectId; // Process ID, thread ID, device ID, and stream ID that identify the marker
    const char *name; // Name of the marker. The value is NULL when the marker is ended.
    const char *domain; // Name of the domain to which the marker belongs. The default domain is NULL.
} msptiActivityMarker;
```
