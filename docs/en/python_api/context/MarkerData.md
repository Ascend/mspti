# MarkerData<a name="ZH-CN_TOPIC_0000002154732221"></a>

This section describes the instantaneous dotting data of the mstx API. For details about the mstx API, see https://www.hiascend.com/document/detail/zh/mindstudio/82RC1/API/mstxAPIReference/msprof_tx_0001.html.

MarkerData is the structure invoked by [MstxMonitor.start](MstxMonitor-start.md). The definition is as follows:

```python
class MarkerData:
    self.kind # Activity Record type MSPTI_ACTIVITY_KIND_MARKER
    self.flag: MsptiActivityFlag # Flag of the marker data
    self.source_kind: MsptiActivitySourceKind # Source type of the marker data
    self.timestamp # Timestamp of the marker, in ns. If the value is 0, the timestamp information cannot be collected for the marker.
    self.id # ID of the marker
    self.object_id: MsptiObjectId # Process ID, thread ID, device ID, and stream ID that identify the marker
    self.name # Name of the marker. When the marker ends, the value is empty.
    self.domain # Name of the domain to which the marker belongs. The default domain is default.
class MsptiObjectId:
    PROCESS_ID = "processId" # Process ID. If the data is on the device, the value is fixed to -1.
    THREAD_ID = "threadId" # Thread ID. If the data is on the device, the value is fixed to -1.
    DEVICE_ID = "deviceId" # Device ID. If the data is on the host, the value is fixed to -1.
    STREAM_ID = "streamId" # Stream ID. If the data is on the host, the value is fixed to -1.
```
