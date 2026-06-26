# MarkerData<a name="ZH-CN_TOPIC_0000002154732221"></a>

Describes instantaneous marker data of the msTX API. For details about the msTX API, see [msTX API Reference](https://gitcode.com/Ascend/mstx/blob/26.0.0/docs/en/api_reference/README.md).

MarkerData is the structure called by [MstxMonitor.start](MstxMonitor-start.md) and is defined as follows:

```python
class MarkerData:
    self.kind   # Activity Record type MSPTI_ACTIVITY_KIND_MARKER
    self.flag: MsptiActivityFlag   # Flag of the Marker data
    self.source_kind: MsptiActivitySourceKind   # Source type of the marker data
    self.timestamp   # Timestamp of the marker, in ns. A value of 0 indicates that timestamp information cannot be collected for the marker
    self.id   # ID of the marker
    self.object_id: MsptiObjectId   # Process ID, thread ID, device ID, and stream ID that identify the marker
    self.name   # Name of the marker. The value is empty when the marker ends
    self.domain   # Name of the domain to which the marker belongs. The default domain is default
class MsptiObjectId:
    PROCESS_ID = "processId"   # Process ID. For device-side data, this is fixed to -1
    THREAD_ID = "threadId"   # Thread ID. For device-side data, this is fixed to -1
    DEVICE_ID = "deviceId"   # Device ID. For host-side data, this is fixed to -1
    STREAM_ID = "streamId"   # Stream ID. For host-side data, this is fixed to -1
```
