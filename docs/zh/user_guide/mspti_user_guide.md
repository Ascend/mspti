# msPTI 用户指南

## 1. 概述

本文档面向希望使用 msPTI（MindStudio Profiler Tools Interface）进行 NPU 应用性能分析的用户。msPTI 提供三种编程接口，分别适用于不同开发语言和分析场景。建议新用户先阅读《[msPTI 快速入门](../quick_start/quick_start.md)》完成环境准备，再根据自身需求选择对应的接口指南。

### 1.1 接口选型速览

| 接口类型 | 适用语言 | 核心能力 | 典型场景 |
| --- | --- | --- | --- |
| Activity API | C / C++ | 异步缓冲区模式采集 Kernel、Memory、HCCL、Marker 等活动数据，开销低，覆盖类型最全 | 构建 Tracing / Profiling 工具，性能瓶颈定位 |
| Callback API | C / C++ | 在 Runtime / HCCL API 调用前后注册回调，支持 userdata 透传和 correlationData 共享 | API 调用拦截、打点采集、自定义逻辑注入 |
| Python API | Python | Monitor 封装模式，一行启动采集，内置多线程消费者 | 快速为 Python 训练脚本添加性能监控 |

### 1.2 三篇指南导航

| 文档 | 适用语言 | 适用场景 |
| --- | --- | --- |
| [《Activity API 使用指南》](./activity_api.md) | C / C++ | 采集 Kernel、Memory、HCCL、Marker 等活动数据，构建 Tracing 和 Profiling 工具 |
| [《Callback API 使用指南》](./callback_api.md) | C / C++ | 订阅 Runtime / HCCL 回调，在 API 调用前后执行自定义逻辑 |
| [《Python API 使用指南》](./python_api.md) | Python | 使用 `KernelMonitor`、`HcclMonitor`、`MstxMonitor`、`CommunicationMonitor` 快速接入 |

### 1.3 混合使用策略

Callback API 和 Activity API 可以同时使能，互不冲突。典型组合：

- **Callback 打点 + Activity 采集**：使用 Callback API 在 Launch Kernel 入口/出口调用 `mstxMarkA` 打点，同时使能 Activity API 采集 `MSPTI_ACTIVITY_KIND_MARKER` 和 `MSPTI_ACTIVITY_KIND_KERNEL`，实现 API 上下文与 Kernel 执行数据的关联分析。
- **Activity 采集 + Python 消费**：使用 C Activity API 采集全量数据，通过 Python 扩展绑定读取和分析结果。

## 2. 快速导航

| 阶段 | 参考文档 | 说明 |
| --- | --- | --- |
| 环境准备与工具安装 | 《[msPTI 快速入门](../quick_start/quick_start.md)》 | 首次使用必读：硬件要求、软件安装、环境配置 |
| C 接口完整参考 | 《[C API 参考](../api_reference/c_api/README.md)》 | Activity API + Callback API 全部函数与数据结构 |
| Python 接口完整参考 | 《[Python API 参考](../api_reference/python_api/README.md)》 | KernelMonitor / HcclMonitor / MstxMonitor / CommunicationMonitor 全接口 |
| 样例列表与说明 | 《[msPTI 样例指南](../user_guide/samples_guide.md)》 | 9 个覆盖各类接口的实战样例 |
| 开发与构建 | 《[msPTI 开发指南](../development_guide/development_guide.md)》 | 源码编译、二次开发、测试与提交 |
| 最佳实践与典型案例 | 《[msPTI 最佳实践](../best_practices/basic_cases.md)》 | 接口选型、缓冲区管理、反模式规避 |

## 3. 通用约定

### 3.1 环境变量

msPTI 使用前需设置 CANN 环境变量，`${install_path}` 指 CANN 安装路径，例如 `/usr/local/Ascend/cann`。使用前需执行：

```bash
source ${install_path}/set_env.sh
```

若使用 msPTI Python API，还需设置 `LD_PRELOAD`：

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
```

### 3.2 错误码

所有 msPTI 函数返回 `msptiResult` 枚举值：

| 错误码 | 值 | 说明 | 触发场景 |
| --- | --- | --- | --- |
| `MSPTI_SUCCESS` | 0 | 成功 | 操作正常完成 |
| `MSPTI_ERROR_INVALID_PARAMETER` | 1 | 无效参数 | 函数参数为 NULL 或类型不正确 |
| `MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED` | 2 | 不允许多个订阅者 | 已有订阅者时再次调用 `msptiSubscribe` |
| `MSPTI_ERROR_MAX_LIMIT_REACHED` | 3 | 已达到最大限制 | Activity Buffer 无更多 Record 空间 |
| `MSPTI_ERROR_DEVICE_OFFLINE` | 4 | 设备离线 | 无法获取 Device 侧信息 |
| `MSPTI_ERROR_QUEUE_EMPTY` | 5 | 队列为空 | External Correlation ID 匹配失败 |
| `MSPTI_ERROR_WITHOUT_LD_PRELOAD` | 6 | 未设置 LD_PRELOAD | `libmspti.so` 未预加载 |
| `MSPTI_ERROR_INNER` | 999 | 内部错误 | MSPTI 初始化失败或内部异常 |

> [!NOTE] Note
> 详细错误处理请参见各 API 参考文档中的"返回值说明"章节。

### 3.3 约束与限制

- msPTI **不可**与其他性能数据采集工具同时使用，否则会导致采集的数据丢失。
- msPTI 依赖 Linux 操作系统和昇腾 NPU 硬件，不支持 Windows 环境。
- Activity Buffer 缓冲区大小上限为 256 MB。
- 同一时刻仅支持一个 Callback 订阅者（Subscriber）。
- 所有 Activity Kind 默认关闭，必须显式调用 `msptiActivityEnable` 使能。

### 3.4 性能注意事项

- 按需使能 Activity Kind：每多使能一个 Kind 都会增加性能开销。
- 避免在 Buffer CompleteFunc 中执行耗时操作（文件写入、网络传输等），建议将原始数据放入队列由后台线程异步处理。
- Python Monitor 的回调函数应尽量轻量，高吞吐场景下推荐使用多线程消费者模式。
