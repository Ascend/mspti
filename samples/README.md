# msPTI 样例说明

## 简介

本文档用于说明 `samples` 目录下各个样例的适用场景与功能定位，帮助用户根据分析目标快速选择对应样例。

## 样例使用说明

安装并配置 CANN 环境变量后，可进入目标样例目录执行对应脚本：

```bash
source ${install_path}/set_env.sh
cd ${install_path}/tools/mspti/samples/callback_domain
bash sample_run.sh
```

其中 `${install_path}` 需要替换为 CANN 的安装路径，例如 `/usr/local/Ascend/cann`。

## 样例目录说明

| 样例目录 | 接口类型 | 适用场景 | 说明 |
| --- | --- | --- | --- |
| [callback_domain](./callback_domain) | Callback API | Runtime API 回调拦截与前后处理 | 演示通过 `msptiEnableDomain` 订阅指定 domain，并在 API 调用前后执行回调逻辑。 |
| [callback_mstx](./callback_mstx) | Callback API + MSTX | Launch Kernel 路径打点与上下文透传 | 演示 Callback 与 MSTX 联合采集，以及通过 `userdata` 透传配置或运行参数。 |
| [callback_finalize](./callback_finalize) | Callback API | 采集会话的卸载与新会话启动 | 演示调用 `msptiFinalize` 卸载 MSPTI 后进程继续运行、回调停止分发，以及再次开启会话时恢复采集。 |
| [mspti_activity](./mspti_activity) | Activity API | 基础活动数据采集与缓冲区处理 | 演示 Kernel、Memory 等活动数据采集流程，以及 Activity Buffer 的申请、消费与管理。 |
| [mspti_correlation](./mspti_correlation) | Activity API | API 下发与 Kernel 执行关联分析 | 演示利用 `correlationId` 建立 Runtime API 与 Kernel 执行记录之间的关联关系。 |
| [mspti_external_correlation](./mspti_external_correlation) | Activity API | 跨层调用链路关联分析 | 演示外部关联 ID 的压栈与出栈机制，用于串联不同层级的调用关系。 |
| [mspti_hccl_activity](./mspti_hccl_activity) | Activity API | HCCL 通信行为采集与分析 | 演示开启 HCCL 相关活动采集并获取通信算子数据。 |
| [mspti_mstx_activity_domain](./mspti_mstx_activity_domain) | Activity API + MSTX | 域级打点采集控制 | 演示按 `mstxDomain` 动态启停打点采集，以控制采集范围和性能开销。 |
| [python_monitor](./python_monitor) | Python API | Python 场景下的计算与通信耗时采集 | 演示使用 `KernelMonitor` 和 `CommunicationMonitor` 获取计算算子与通信算子的耗时数据。 |
| [python_mstx_monitor](./python_mstx_monitor) | Python API + MSTX | Python 场景下的自定义打点分析 | 演示使用 `MstxMonitor` 采集指定算子或代码段的打点耗时。 |

## 补充说明

- [`common`](./common) 为样例公共头文件目录，不是独立可运行样例。
- Python 样例额外依赖 PyTorch 和 `torch_npu` 插件。
