# RangeMarkerData<a name="ZH-CN_TOPIC_0000002279641170"></a>

This section describes the range dotting data of the mstx API. For details about the mstx API, see https://www.hiascend.com/document/detail/zh/mindstudio/82RC1/API/mstxAPIReference/msprof_tx_0001.html.

RangeMarkerData is the structure invoked by [MstxMonitor.start](MstxMonitor-start.md). The definition is as follows:

```python
class RangeMarkerData:
    self.kind # Activity Record type MSPTI_ACTIVITY_KIND_MARKER
    self.source_kind: MsptiActivitySourceKind # Source type of the marker data
    self.id # Marker ID
    self.object_id: MsptiObjectId # Process ID, thread ID, device ID, and stream ID that identify the marker
    self.name # Marker name. The value is empty when the marker ends.
    self.domain # Name of the domain to which the marker belongs. The default domain is default.
    self.start # Start time of the range dotting. The value of the mark dotting is 0.
    self.end # End time of the range dotting. The value of the mark dotting is 0.
class MsptiObjectId:
    PROCESS_ID = "processId" # Process ID. If the data is on the device, the value is fixed to -1.
    THREAD_ID = "threadId" # Thread ID. If the data is on the device, the value is fixed to -1.
    DEVICE_ID = "deviceId" # Device ID. If the data is on the host, the value is fixed to -1.
    STREAM_ID = "streamId" # Stream ID. If the data is on the host, the value is fixed to -1.
```
