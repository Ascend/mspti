# General Description <a name="ZH-CN_TOPIC_0000002108084148"></a>

## Interface Overview<a name="section172424491588"></a>

The Profiling module provides the msPTI Python API to collect performance data of each module.

For details about the functions and usage examples of the msPTI API, see [msPTI Tool](../README.md).

## Interface List<a name="section18403103610813"></a>

APIs are listed below.

**Table 1** msPTI Python APIs

|Interface|Description|
|--|--|
|**HcclMonitor Type**|**HcclMonitor Description**|
|[HcclMonitor.start](./context/HcclMonitor-start.md)|Marks the start of the communication operator profiling.|
|[HcclMonitor.stop](./context/HcclMonitor-stop.md)|Marks the end of the communication operator profiling.|
|[HcclMonitor.flush_all](./context/HcclMonitor-flush_all.md)|Calls the callback function to write all activity data in the buffer to the user memory.|
|[HcclMonitor.set_buffer_size](./context/HcclMonitor-set_buffer_size.md)|Sets the size of the activity buffer before profiling starts.|
|**KernelMonitor Type**|**KernelMonitor Description**|
|[KernelMonitor.start](./context/KernelMonitor-start.md)|Marks the start of the kernel profiling.|
|[KernelMonitor.stop](./context/KernelMonitor-stop.md)|Marks the end of the kernel profiling.|
|[KernelMonitor.flush_all](./context/KernelMonitor-flush_all.md)|Calls the callback function to write all activity data in the buffer to the user memory.|
|[KernelMonitor.set_buffer_size](./context/KernelMonitor-set_buffer_size.md)|Sets the size of the activity buffer before profiling starts.|
|**MstxMonitor Type**|**Description of MstxMonitor**|
|[MstxMonitor.start](./context/MstxMonitor-start.md)|Marks the start of the mstx profiling.|
|[MstxMonitor.stop](./context/MstxMonitor-stop.md)|Marks the end of the mstx marker for profiling.|
|[MstxMonitor.enable_domain](./context/MstxMonitor-enable_domain.md)|Enables the profiling for a specific domain.|
|[MstxMonitor.disable_domain](./context/MstxMonitor-disable_domain.md)|Disables the profiling for a specific domain.|
|[MstxMonitor.flush_all](./context/MstxMonitor-flush_all.md)|Calls the callback function to write all activity data in the buffer to the user memory.|
|[MstxMonitor.set_buffer_size](./context/MstxMonitor-set_buffer_size.md)|Sets the size of the activity buffer before profiling starts.|
|**Data structure type**|**Description of Data Structure**|
|[HcclData](./context/HcclData.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_HCCL.|
|[KernelData](./context/KernelData.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_KERNEL.|
|[MarkerData](./context/MarkerData.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_MARKER.|
|[RangeMarkerData](./context/RangeMarkerData.md)|Structure corresponding to MSPTI_ACTIVITY_KIND_MARKER of the activity record type.|
|**Enumeration type**|**Enumeration description**|
|[msptiResult](./context/msptiResult.md)|Error and result code returned by MSPTI.|
|[msptiActivityKind](./context/msptiActivityKind.md)|All activity types supported by MSPTI.|
|[msptiActivityFlag](./context/msptiActivityFlag.md)|Activity record flag.|
|[msptiActivitySourceKind](./context/msptiActivitySourceKind.md)|Activity data source.|
