# Python API Overview<a name="ZH-CN_TOPIC_0000002108084148"></a>

## Description<a name="section172424491588"></a>

The Profiling module provides the msPTI Python APIs for profiling performance data of each module.

For details about the functions and usage examples of the msPTI APIs, see [msPTI Tool](../getting_started/samples_guide.md).

## API List<a name="section18403103610813"></a>

Specific APIs are as follows:

**Table 1** msPTI Python APIs

|Interface|Description|
|--|--|
|**HcclMonitor**|**Description**|
|[HcclMonitor.start](./context/HcclMonitor-start.md)|Marks the start of communication operator performance data profiling.|
|[HcclMonitor.stop](./context/HcclMonitor-stop.md)|Marks the end of communication operator performance data profiling.|
|[HcclMonitor.flush_all](./context/HcclMonitor-flush_all.md)|Invokes the callback function to write all Activity data in the buffer to user memory.|
|[HcclMonitor.set_buffer_size](./context/HcclMonitor-set_buffer_size.md)|Sets the Activity Buffer size before profiling starts.|
|**KernelMonitor**|**Description**|
|[KernelMonitor.start](./context/KernelMonitor-start.md)|Marks the start of Kernel performance data profiling.|
|[KernelMonitor.stop](./context/KernelMonitor-stop.md)|Marks the end of Kernel performance data profiling.|
|[KernelMonitor.flush_all](./context/KernelMonitor-flush_all.md)|Invokes the callback function to write all Activity data in the buffer to user memory.|
|[KernelMonitor.set_buffer_size](./context/KernelMonitor-set_buffer_size.md)|Sets the Activity Buffer size before profiling starts.|
|**MstxMonitor**|**Description**|
|[MstxMonitor.start](./context/MstxMonitor-start.md)|Marks the start of msTX marker data profiling.|
|[MstxMonitor.stop](./context/MstxMonitor-stop.md)|Marks the end of msTX marker data profiling.|
|[MstxMonitor.enable_domain](./context/MstxMonitor-enable_domain.md)|Enables profiling for the corresponding domain marker.|
|[MstxMonitor.disable_domain](./context/MstxMonitor-disable_domain.md)|Disables profiling for the corresponding domain marker.|
|[MstxMonitor.flush_all](./context/MstxMonitor-flush_all.md)|Invokes the callback function to write all Activity data in the buffer to user memory.|
|[MstxMonitor.set_buffer_size](./context/MstxMonitor-set_buffer_size.md)|Sets the activity buffer size before profiling starts.|
|**Data Structure**|**Description**|
|[HcclData](./context/HcclData.md)|Structure corresponding to the activity record type MSPTI_ACTIVITY_KIND_HCCL.|
|[KernelData](./context/KernelData.md)|Structure corresponding to the activity record type MSPTI_ACTIVITY_KIND_KERNEL.|
|[MarkerData](./context/MarkerData.md)|Structure corresponding to the activity record type MSPTI_ACTIVITY_KIND_MARKER.|
|[RangeMarkerData](./context/RangeMarkerData.md)|Structure corresponding to the activity record type MSPTI_ACTIVITY_KIND_MARKER.|
|**Enumeration**|**Description**|
|[msptiResult](./context/msptiResult.md)|Error and result codes returned by MSPTI.|
|[msptiActivityKind](./context/msptiActivityKind.md)|All Activity types supported by MSPTI.|
|[msptiActivityFlag](./context/msptiActivityFlag.md)|Activity flags for activity records.|
|[msptiActivitySourceKind](./context/msptiActivitySourceKind.md)|Marks the source of Activity data.|
