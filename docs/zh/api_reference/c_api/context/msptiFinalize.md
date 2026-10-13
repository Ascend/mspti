# msptiFinalize<a name="ZH-CN_TOPIC_0000002091012100"></a>

## 产品支持情况<a name="section8178181118226"></a>

> [!NOTE]
>
> 昇腾产品的具体型号，请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/Productform/hardwaredesc_0001.html)》。

<a name="zh-cn_topic_0000002014413734_table38301303190"></a>

<!-- npu="950" id1 -->
- Ascend 950PR&950DT系列产品：支持
<!-- end id1 -->
<!-- npu="A3" id2 -->
- Atlas A3系列产品：支持
<!-- end id2 -->
<!-- npu="910b" id3 -->
- Atlas A2系列产品：支持
<!-- end id3 -->
<!-- npu="310b" id4 -->
- Atlas 200I/500 A2推理产品：支持
<!-- end id4 -->
<!-- npu="310p" id5 -->
- Atlas推理系列产品：不支持
<!-- end id5 -->
<!-- npu="910" id6 -->
- Atlas训练系列产品：不支持
<!-- end id6 -->

## 功能说明<a name="section20806203412479"></a>

该接口用于从当前进程中卸载并销毁 msPTI 资源。

卸载后状态：

- 进程继续运行，但不再受 msPTI 拦截，所有被劫持的 CANN 接口恢复为直接透传。
- 卸载后调用 `msptiIsTracingSessionRunning` 接口会返回 0，随后可通过开启接口（`msptiSubscribe`、`msptiActivityRegisterCallbacks` 以及 `msptiActivityEnable`）重新发起采集，支持 “采集 → 卸载 → 再采集” 的循环操作。

调用该接口将执行以下清理操作：

1. 停止采集：关闭所有已开启的 Activity 采集流程，排空采集类型。
2. 停止任务：停止设备侧 Profiling 任务及 msPTI 工作线程。
3. 注销订阅：自动注销所有已注册的回调订阅者（等效于调用 msptiUnsubscribe）。

> [!NOTE]
>
> 为安全使用本接口，推荐在任意已订阅CANN API的 MSPTI_API_EXIT 回调点调用。<br>
> 若在非回调点的普通代码中调用，调用方必须自行确保以下前提条件已满足：<br>
>
> - 完成所需的 CANN 同步。
> - 刷新 Activity 缓冲区并等待处理完成（调用 `msptiActivityFlushAll` 并等待 `msptiBuffersCallbackCompleteFunc` 回调结束）。
>
> 本接口本身不处理 Activity 缓冲区，需由调用方预先处理。<br>
> 在 MSPTI_API_ENTER 回调点调用时，当前API仍会执行完毕，但其配对的 EXIT 回调将被丢弃。<br>


> [!WARNING]
>
> 严禁在 msPTI 的缓冲区回调函数（`msptiBuffersCallbackRequestFunc` 或 `msptiBuffersCallbackCompleteFunc`）中调用任何 msPTI 接口（包括本接口，以及 `msptiSubscribe`、`msptiUnsubscribe`、`msptiActivityEnable`、`msptiActivityDisable`、`msptiActivityRegisterCallbacks`、`msptiActivityFlushAll`等接口）。原因：卸载过程持有锁并停止工作线程，若在处理缓冲区的线程上再次调用任何 msPTI 接口（仅 `msptiActivityGetNextRecord` 除外），易导致死锁或进程异常。

## 函数原型<a name="section1121883194712"></a>

```cpp
msptiResult msptiFinalize(void)
```

## 参数说明<a name="section11506138144715"></a>

无。

## 返回值说明<a name="section16621124213477"></a>

- 返回 `MSPTI_SUCCESS` 表示成功，包括无会话运行时的重复调用场景。
- 返回 `MSPTI_ERROR_INNER` 表示某一步执行失败（例如设备侧任务停止失败）。
