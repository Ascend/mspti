# General Description <a name="ZH-CN_TOPIC_0000001977973392"></a>

## Interface Overview<a name="section883612815318"></a>

The Profiling module provides the msPTI C interface to collect performance data of each module.

For details about the functions and usage examples of the msPTI API, see [msPTI Tool](../README.md).

Header file path: $\{INSTALL\_DIR\}/include/mspti.

Library file path: $\{INSTALL\_DIR\}/lib64/libmspti.so.

Replace $\{INSTALL\_DIR\} with the path for storing the files after the CANN Toolkit is installed. For example, if the installation is performed by the `root` user, the path is `/usr/local/Ascend/cann`.

## Interface List<a name="section2321145165316"></a>

APIs are listed below.

**Table 1** Activity APIs

|Interface|Description|
|--|--|
|**Function type**|**Function Description**|
|[msptiActivityRegisterCallbacks](./context/msptiActivityRegisterCallbacks.md)|Registers callback functions with MSPTI for activity buffer processing.|
|[msptiActivityEnable](./context/msptiActivityEnable.md)|Profiles data of a specified activity type.|
|[msptiActivityDisable](./context/msptiActivityDisable.md)|Stops profiling activity records of a specified type.|
|[msptiActivityGetNextRecord](./context/msptiActivityGetNextRecord.md)|Obtains the activity record data from the activity buffer in sequence.|
|[msptiActivityFlushAll](./context/msptiActivityFlushAll.md)|Subscribers manually flush the data recorded in the activity buffer.|
|[msptiActivityFlushPeriod](./context/msptiActivityFlushPeriod.md)|Sets the execution period of flushing.|
|[msptiActivityPushExternalCorrelationId](./context/msptiActivityPushExternalCorrelationId.md)|Pushes an external correlation ID for the calling thread.|
|[msptiActivityPopExternalCorrelationId](./context/msptiActivityPopExternalCorrelationId.md)|Pops the external correlation ID for the calling thread.|
|[msptiActivityEnableMarkerDomain](./context/msptiActivityEnableMarkerDomain.md)|Enables the profiling for a specific domain.|
|[msptiActivityDisableMarkerDomain](./context/msptiActivityDisableMarkerDomain.md)|Disables the profiling for a specific domain.|
|**Typedef type**|**Description**|
|[msptiBuffersCallbackRequestFunc](./context/msptiBuffersCallbackRequestFunc.md)|Registers the callback function with MSPTI to allocate the storage space of the activity buffer.|
|[msptiBuffersCallbackCompleteFunc](./context/msptiBuffersCallbackCompleteFunc.md)|Registers the callback function with MSPTI to release the data in the activity buffer.|
|**Enumeration type**|**Enumeration description**|
|[msptiActivityKind](./context/msptiActivityKind.md)|All activity types supported by MSPTI.|
|[msptiActivityFlag](./context/msptiActivityFlag.md)|Activity record flag.|
|[msptiActivitySourceKind](./context/msptiActivitySourceKind.md)|Activity data source.|
|[msptiActivityMemoryOperationType](./context/msptiActivityMemoryOperationType.md)|Enumeration of memory operation types.|
|[msptiActivityMemoryKind](./context/msptiActivityMemoryKind.md)|Enumeration of memory types.|
|[msptiActivityMemcpyKind](./context/msptiActivityMemcpyKind.md)|Enumeration of memory copy types.|
|[msptiExternalCorrelationKind](./context/msptiExternalCorrelationKind.md)|Supported types of external APIs that can be correlated.|
|[msptiCommunicationDataType](./context/msptiCommunicationDataType.md)|Data type of the communication operator.|
|**Data structure type**|**Data Structure Description**|
|[msptiActivity](./context/msptiActivity.md)|Basic struct of an activity record.|
|[msptiActivityApi](./context/msptiActivityApi.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_API.|
|[msptiActivityHccl](./context/msptiActivityHccl.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_HCCL.|
|[msptiActivityKernel](./context/msptiActivityKernel.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_KERNEL.|
|[msptiActivityMarker](./context/msptiActivityMarker.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_MARKER.|
|[msptiActivityMemory](./context/msptiActivityMemory.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_MEMORY.|
|[msptiActivityMemset](./context/msptiActivityMemset.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_MEMSET.|
|[msptiActivityMemcpy](./context/msptiActivityMemcpy.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_MEMCPY.|
|[msptiActivityExternalCorrelation](./context/msptiActivityExternalCorrelation.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION.|
|[msptiActivityCommunication](./context/msptiActivityCommunication.md)|Struct corresponding to the activity record type MSPTI_ACTIVITY_KIND_COMMUNICATION.|
|**Union type**|**Union Description**|
|[msptiObjectId](./context/msptiObjectId.md)|Identifies the process ID, thread ID, device ID, and stream ID of the marker.|

Activity record: NPU profiling record, which is represented by a struct, such as **msptiActivityApi** or **msptiActivityMarker**.

Activity buffer: Buffers activity record data and transfers one or more activity records from MSPTI to the client. You need to provide an empty activity buffer based on service requirements to ensure that no activity record is missing.

**Table 2** Callback APIs

|API|Description|
|--|--|
|**Function type**|**Function Description**|
|[msptiSubscribe](./context/msptiSubscribe.md)|Registers callback functions with MSPTI.|
|[msptiUnsubscribe](./context/msptiUnsubscribe.md)|Deregisters the current subscriber from MSPTI.|
|[msptiEnableCallback](./context/msptiEnableCallback.md)|Enables or disables callbacks for subscribers of specific **domain** and **CallbackId**.|
|[msptiEnableDomain](./context/msptiEnableDomain.md)|Enables or disables all callbacks for subscribers of specific **domain**.|
|**Typedef type**|**Description**|
|[msptiCallbackFunc](./context/msptiCallbackFunc.md)|Callback function type.|
|[msptiCallbackId](./context/msptiCallbackId.md)|ID of the callback tracing function.|
|[msptiSubscriberHandle](./context/msptiSubscriberHandle.md)|Handle to the subscriber.|
|**Enumeration type**|**Enumeration description**|
|[msptiCallbackDomain](./context/msptiCallbackDomain.md)|Callback point of an API function or CANN driver activity.|
|[msptiApiCallbackSite](./context/msptiApiCallbackSite.md)|Callback point in an API call, for example, the start and end of the callback.|
|[msptiCallbackIdRuntime](./context/msptiCallbackIdRuntime.md)|Index definition of the runtime API function.|
|[msptiCallbackIdHccl](./context/msptiCallbackIdHccl.md)|Brief definition of the communication API function index.|
|**Data structure type**|**Data Structure Description**|
|[msptiCallbackData](./context/msptiCallbackData.md)|Data to be passed to the callback function.|

**Table 3** Result codes

|API|Description|
|--|--|
|**Enumeration type**|**Enumeration description**|
|[msptiResult](./context/msptiResult.md)|Error and result code returned by MSPTI.|
