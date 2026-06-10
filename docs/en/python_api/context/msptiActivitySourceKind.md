# msptiActivitySourceKind<a name="ZH-CN_TOPIC_0000002119627750"></a>

Activity data source. either host or device.

The msptiActivitySourceKind is an enumeration class called in the [MarkerData](MarkerData.md) structure. It is defined as follows:

```python
class MsptiActivitySourceKind(Enum):
    MSPTI_ACTIVITY_SOURCE_KIND_HOST = 0 # Indicates that the data source is the host.
    MSPTI_ACTIVITY_SOURCE_KIND_DEVICE = 1 # Indicates that the data source is the device.
```
