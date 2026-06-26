# C API Overview<a name="ZH-CN_TOPIC_0000001977973392"></a>

## API Introduction<a name="section883612815318"></a>

The Profiling module provides msPTI C APIs for profiling performance data of each module.

For details about the functions and usage examples of msPTI APIs, see [msPTI Tool](../getting_started/samples_guide.md).

Header file path: `$\{INSTALL\_DIR\}/include/mspti`

Library file path: `${INSTALL_DIR}/lib64/libmspti.so`

Replace `${INSTALL_DIR}` with the file storage path after the CANN Toolkit development suite package is installed. For example, if installed as root, the file storage path after installation is: `/usr/local/Ascend/cann`.

## API List<a name="section2321145165316"></a>

Specific APIs are listed as follows:

**Table 1** Activity API

|API|Description|
|--|--|
|**Function**|**Description**|
|[msptiActivityRegisterCallbacks](./context/msptiActivityRegisterCallbacks.md)|Registers callback functions with MSPTI for Activity Buffer processing.|
|[msptiActivityEnable](./context/msptiActivityEnable.md)|Enables the collection of data for a specified activity type.|
|[msptiActivityDisable](./context/msptiActivityDisable.md)|Stops collecting a specific type of Activity Record.|
|[msptiActivityGetNextRecord](./context/msptiActivityGetNextRecord.md)|Retrieves Activity Record data from the Activity Buffer sequentially.|
|[msptiActivityFlushAll](./context/msptiActivityFlushAll.md)|Allows the subscriber to manually flush the data recorded in the Activity Buffer.|
|[msptiActivityFlushPeriod](./context/msptiActivityFlushPeriod.md)|Sets the execution period for flushing.|
|[msptiActivityPushExternalCorrelationId](./context/msptiActivityPushExternalCorrelationId.md)|Pushes an external correlation ID for the calling thread.|
|[msptiActivityPopExternalCorrelationId](./context/msptiActivityPopExternalCorrelationId.md)|Pops an external correlation ID for the calling thread.|
|[msptiActivityEnableMarkerDomain](./context/msptiActivityEnableMarkerDomain.md)|Enables the collection of markers for the corresponding domain.|
|[msptiActivityDisableMarkerDomain](./context/msptiActivityDisableMarkerDomain.md)|Disables the collection of markers for the corresponding domain.|
|**Typedef**|**Description**|
|[msptiBuffersCallbackRequestFunc](./context/msptiBuffersCallbackRequestFunc.md)|Registers a callback function with MSPTI to request storage space for the Activity Buffer.|
|[msptiBuffersCallbackCompleteFunc](./context/msptiBuffersCallbackCompleteFunc.md)|Registers a callback function with MSPTI to release data in the Activity Buffer.|
|**Enumeration**|**Description**|
|[msptiActivityKind](./context/msptiActivityKind.md)|All activity types supported by MSPTI.|
|[msptiActivityFlag](./context/msptiActivityFlag.md)|Activity flags for an Activity Record.|
|[msptiActivitySourceKind](./context/msptiActivitySourceKind.md)|Marks the source of activity data.|
|[msptiActivityMemoryOperationType](./context/msptiActivityMemoryOperationType.md)|Enumeration class for memory operation types.|
|[msptiActivityMemoryKind](./context/msptiActivityMemoryKind.md)|Enumeration class for memory types.|
|[msptiActivityMemcpyKind](./context/msptiActivityMemcpyKind.md)|Enumeration class for memory copy types.|
|[msptiExternalCorrelationKind](./context/msptiExternalCorrelationKind.md)|Types of external APIs supported for correlation.|
|[msptiCommunicationDataType](./context/msptiCommunicationDataType.md)|Records the data type of communication operators.|
|**Data Structure**|**Description**|
|[msptiActivity](./context/msptiActivity.md)|Base structure for an Activity Record.|
|[msptiActivityApi](./context/msptiActivityApi.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_API.|
|[msptiActivityHccl](./context/msptiActivityHccl.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_HCCL.|
|[msptiActivityKernel](./context/msptiActivityKernel.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_KERNEL.|
|[msptiActivityMarker](./context/msptiActivityMarker.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_MARKER.|
|[msptiActivityMemory](./context/msptiActivityMemory.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_MEMORY.|
|[msptiActivityMemset](./context/msptiActivityMemset.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_MEMSET.|
|[msptiActivityMemcpy](./context/msptiActivityMemcpy.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_MEMCPY.|
|[msptiActivityExternalCorrelation](./context/msptiActivityExternalCorrelation.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION.|
|[msptiActivityCommunication](./context/msptiActivityCommunication.md)|Structure corresponding to the Activity Record type MSPTI_ACTIVITY_KIND_COMMUNICATION.|
|**Union**|**Description**|
|[msptiObjectId](./context/msptiObjectId.md)|Used to identify the process ID, thread ID, device ID, and stream ID of a marker.|

Activity record: A profiling record of the NPU, represented by structures such as msptiActivityApi and msptiActivityMarker.

Activity buffer: Used to cache activity record data and transfer one or more Activity Records from MSPTI to the client. Users provide empty Activity Buffer buffers based on service requirements to ensure that no activity records are missed.

**Table 2**  Callback API

|API|Description|
|--|--|
|**Function**|**Description**|
|[msptiSubscribe](./context/msptiSubscribe.md)|Registers a callback function with MSPTI through this API.|
|[msptiUnsubscribe](./context/msptiUnsubscribe.md)|Unsubscribes the current subscriber from MSPTI.|
|[msptiEnableCallback](./context/msptiEnableCallback.md)|Enables or disables a callback for a subscriber of a specific **domain** and **CallbackId**.|
|[msptiEnableDomain](./context/msptiEnableDomain.md)|Enables or disables all callbacks for a subscriber of a specific **domain**.|
|**Typedef**|**Description**|
|[msptiCallbackFunc](./context/msptiCallbackFunc.md)|Callback function type.|
|[msptiCallbackId](./context/msptiCallbackId.md)|ID that identifies a callback trace function.|
|[msptiSubscriberHandle](./context/msptiSubscriberHandle.md)|Handle of the subscriber.|
|**Enumeration**|**Description**|
|[msptiCallbackDomain](./context/msptiCallbackDomain.md)|Callback points for related API functions or CANN driver activities.|
|[msptiApiCallbackSite](./context/msptiApiCallbackSite.md)|Specifies the point in an API call where a callback is issued, such as the start and end of the callback.|
|[msptiCallbackIdRuntime](./context/msptiCallbackIdRuntime.md)|Index definitions for Runtime API functions.|
|[msptiCallbackIdHccl](./context/msptiCallbackIdHccl.md)|Brief index definitions for communication API functions.|
|**Data Structure**|**Description**|
|[msptiCallbackData](./context/msptiCallbackData.md)|Used to specify the data passed to the callback function.|

**Table 3** Result Codes

|API|Description|
|--|--|
|**Enumeration**|**Description**|
|[msptiResult](./context/msptiResult.md)|Error and result codes returned by MSPTI.|
