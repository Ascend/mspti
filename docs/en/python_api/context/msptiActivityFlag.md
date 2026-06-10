# msptiActivityFlag<a name="ZH-CN_TOPIC_0000002154732225"></a>

Activity record flag. Flags can be combined by bitwise OR to correlate multiple flags with activity records. Each flag is correlated with a specific activity record.

The msptiActivityFlag is an enumeration class called in the [MarkerData](MarkerData.md) structure. It is defined as follows:

```python
class MsptiActivityFlag(Enum):
    MSPTI_ACTIVITY_FLAG_NONE = 0 # Indicates that there is no activity flag for the activity record.
    This flag is used by MSPTI_ACTIVITY_KIND_MARKER when the stream is set to nullptr when the mstxMarkA interface is called in MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS = 1 << 0 #.
    This flag is used by MSPTI_ACTIVITY_KIND_MARKER when the stream is set to nullptr when the mstxRangeStartA interface is called in MSPTI_ACTIVITY_FLAG_MARKER_START = 1 << 1 #.
    This flag is used by MSPTI_ACTIVITY_KIND_MARKER when the ID passed to the mstxRangeEnd interface is the same as that passed to the mstxRangeStartA interface with nullptr stream.
    This flag is used by MSPTI_ACTIVITY_KIND_MARKER when the mstxMarkA interface is called in MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE = 1 << 3 # and a valid stream is passed.
    This flag is used by MSPTI_ACTIVITY_KIND_MARKER when the mstxRangeStartA interface is called in MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE = 1 << 4 # and a valid stream is passed.
    The ID transferred by MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE = 1 << 5 # invoking mstxRangeEnd is the same as that of mstxRangeStartA when a valid stream is transferred. MSPTI_ACTIVITY_KIND_MARKER is used.
```
