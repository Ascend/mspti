# RangeMarkerData<a name="ZH-CN_TOPIC_0000002279641170"></a>

Displays the Range marker data of the msTX API. For details about the msTX API, see [msTX API Reference](https://gitcode.com/Ascend/mstx/blob/26.0.0/docs/en/api_reference/README.md).

RangeMarkerData is the structure called by [MstxMonitor.start](MstxMonitor-start.md), defined as follows:

```python
class RangeMarkerData:
    self.kind   # Activity Record type MSPTI_ACTIVITY_KIND_MARKER
    self.source_kind: MsptiActivitySourceKind   # Source type of the marker data
    self.id   # Marker ID
    self.object_id: MsptiObjectId   # Process ID, thread ID, device ID, and stream ID that identify the marker
    self.name   # Name of the marker. The value is empty for an end marker
    self.domain   # Name of the domain to which the marker belongs. The default domain is default
    self.start   # Start time of the Range marker. The value is 0 for a mark marker
    self.end   # End time of the Range marker, with the marker value being 0
class MsptiObjectId:
    PROCESS_ID = "processId"   # Process ID: If it is device-side data, the corresponding value is fixed at -1
    THREAD_ID = "threadId"   # Thread ID: If it is device-side data, the corresponding value is fixed at -1
    DEVICE_ID = "deviceId"   # Device ID: If it is host-side data, the corresponding value is fixed at -1
    STREAM_ID = "streamId"   # Stream ID: If it is host-side data, the corresponding value is fixed at -1
```
