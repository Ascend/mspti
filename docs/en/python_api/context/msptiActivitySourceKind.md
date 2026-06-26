# MsptiActivitySourceKind<a name="ZH-CN_TOPIC_0000002119627750"></a>

Marks the data source of an activity. Marks whether the data comes from a host or a device.

MsptiActivitySourceKind is an enumeration class called within the [MarkerData](MarkerData.md) struct, defined as follows:

```python
class MsptiActivitySourceKind(Enum):
    MSPTI_ACTIVITY_SOURCE_KIND_HOST = 0   # Marks the data source as a host
    MSPTI_ACTIVITY_SOURCE_KIND_DEVICE = 1   # Marks the data source as a device
```
