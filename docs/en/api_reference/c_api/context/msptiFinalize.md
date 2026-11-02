# msptiFinalize<a name="ZH-CN_TOPIC_0000002091012100"></a>

## Supported Products<a name="section8178181118226"></a>

> [!NOTE]
>
> For details about specific Ascend product models, see [Ascend Product Models](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).

<a name="zh-cn_topic_0000002014413734_table38301303190"></a>

<!-- npu="950" id1 -->
- Atlas 950PR and 950DT products: supported
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3 products: supported
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2 products: supported
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2 inference products: supported
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas inference products: not supported
<!-- end id5 -->
<!-- npu="910" id6 -->
- Atlas training products: not supported
<!-- end id6 -->

## Description<a name="section20806203412479"></a>

This API unloads msPTI resources from the current process and destroys them.

After the unload, the following applies:

- The process continues to run, but is no longer intercepted by msPTI. All hijacked CANN APIs revert to direct pass-through.
- Calling `msptiIsTracingSessionRunning` after the unload returns 0. You can then start collection again by calling the enablement APIs (`msptiSubscribe`, `msptiActivityRegisterCallbacks`, and `msptiActivityEnable`). Therefore, the cyclic operation of "collect → unload → collect again" is supported.

Calling this API performs the following cleanup operations:

1. Stop collection: Shut down all Activity collection processes that have been enabled, and drain the collection types.
2. Stop tasks: Stop the device-side Profiling task and the msPTI worker threads.
3. Unsubscribe: Automatically unregister all registered callback subscribers (equivalent to calling `msptiUnsubscribe`).

> [!NOTE]
>
> To use this API safely, you are advised to call it at any `MSPTI_API_EXIT` callback point for which CANN APIs have been subscribed.<br>
> If you call it in ordinary code that is not at a callback point, you must ensure that the following prerequisites are met:<br>
>
> - Required CANN synchronization is complete.
> - Activity buffers are flushed and processing is complete (call `msptiActivityFlushAll` and wait for the `msptiBuffersCallbackCompleteFunc` callback to finish).
>
> This API does not process Activity buffers itself. You must process them in advance.<br>
> When you call this API at an `MSPTI_API_ENTER` callback point, the current API still runs to completion, but its paired EXIT callback is discarded.<br>

> [!WARNING]
>
> Never call any msPTI API (including this API, as well as `msptiSubscribe`, `msptiUnsubscribe`, `msptiActivityEnable`, `msptiActivityDisable`, `msptiActivityRegisterCallbacks`, `msptiActivityFlushAll`, and other APIs) in the msPTI buffer callback functions (`msptiBuffersCallbackRequestFunc` or `msptiBuffersCallbackCompleteFunc`). The unload process holds a lock and stops worker threads. Therefore, if you call any msPTI API again on the thread that processes buffers (with the sole exception of `msptiActivityGetNextRecord`), you are likely to cause a deadlock or a process exception.

## Function Prototype<a name="section1121883194712"></a>

```cpp
msptiResult msptiFinalize(void)
```

## Parameters<a name="section11506138144715"></a>

None.

## Returns<a name="section16621124213477"></a>

- `MSPTI_SUCCESS` indicates success, including repeated calls when no session is running.
- `MSPTI_ERROR_INNER` indicates that a step failed to execute (for example, the device-side task failed to stop).
