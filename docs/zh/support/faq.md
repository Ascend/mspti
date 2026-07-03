# msPTI 常见问题（FAQ）

## 一、产品概述

### Q1: msPTI 是什么？

msPTI（MindStudio Profiler Tools Interface）是华为昇腾 MindStudio 提供的一套性能剖析 API 集合。您可以使用 msPTI 构建面向 NPU 应用的性能分析工具，应用于推理和训练场景。详细信息请参见《[msPTI 快速入门](../quick_start/quick_start.md)》。

### Q2: msPTI 提供哪些核心能力？

msPTI 提供两大核心能力：

- **Tracing（跟踪）**：采集 CANN 应用的执行时间戳与元数据，涵盖 CANN API 调用、Kernel 执行、内存拷贝、通信操作、用户自定义打点等，帮助定位执行瓶颈。
- **Profiling（性能分析）**：采集单个或一组 Kernel 的 NPU 性能指标，支持计算与通信分析。

### Q3: msPTI 支持哪些产品型号？

| 产品类型 | 是否支持 |
| --- | :---: |
| Ascend 950 系列产品 | √ |
| Atlas A3 训练系列产品 / Atlas A3 推理系列产品 | √ |
| Atlas A2 训练系列产品 / Atlas A2 推理系列产品 | √ |
| Atlas 200I/500 A2 推理产品 | √ |
| Atlas 推理系列产品 | × |
| Atlas 训练系列产品 | × |

### Q4: msPTI 是否支持 Windows 环境？

不支持。msPTI 依赖 Linux 操作系统和昇腾 NPU 硬件，当前仅支持 Linux 环境。

---

## 二、安装与卸载

### Q5: msPTI 如何安装？

msPTI 已集成于 CANN 中，若已安装 CANN 且无需升级此工具，可直接使用。如需单独安装或升级，支持以下三种方式：

| 安装方式 | 适用场景 |
| --- | --- |
| 在线安装 | 设备有互联网访问能力 |
| 离线安装 | 企业内网等无外网环境 |
| 源码安装 | 需使用最新开发版代码 |

详细安装步骤请参见《[msPTI 工具安装指南](../install_guide/mspti_install_guide.md)》。

### Q7: 安装后如何验证是否成功？

执行以下命令：

```bash
pip show mspti
```

若输出版本信息且无报错，则表示安装成功。

### Q8: 如何卸载 msPTI？

执行以下步骤：

```bash
curl -O https://inst.obs.cn-north-4.myhuaweicloud.com/26.0.0/ms_install.py
python ms_install.py uninstall {tools_name}
```

其中 `{tools_name}` 可通过 `python ms_install.py help` 命令查询。若环境无法联网，请先在可联网的设备下载 `ms_install.py` 后拷贝至目标设备。

### Q9: 如何升级 msPTI？

升级即"先卸后装"。直接执行新版本的安装命令，工具将自动卸载旧版本并引导完成覆盖安装。升级前请通过 `pip show mspti` 查看当前版本，并关注版本配套关系，参见《[版本说明](../release_notes/release_notes.md)》。

### Q10: 安装 run 包时支持哪些参数？

| 参数 | 说明 |
| --- | --- |
| `--install` | 安装软件包，可配合 `--install-path` 指定路径 |
| `--uninstall` | 卸载软件包 |
| `--install-path=<path>` | 指定安装路径，必须指定到 CANN 层目录 |
| `--install-for-all` | 允许其他用户具有安装用户组的权限（存在安全风险，请谨慎使用） |

### Q11: MD5 校验不一致怎么办？

若 `md5sum -c -` 输出显示 `FAILED`，请勿继续安装。请删除当前文件并重新下载，再次进行 MD5 校验。若仍然失败，请检查文件名和版本是否与 releases 页面一致，并通过 [Issues](https://gitcode.com/Ascend/mspti/issues) 反馈问题。

---

## 三、环境与配置

### Q12: 使用 msPTI 需要哪些前置条件？

- **硬件**：搭载支持昇腾 NPU 的服务器。
- **软件**：已安装 CANN Toolkit 和 ops 算子包（版本 ≥ 8.5.0）。
- **Python 版本**：若使用 Python API，建议 Python 3.10+。

### Q13: msPTI 可以和其他性能采集工具同时使用吗？

**不可以**。msPTI 不可与其他性能数据采集工具同时使用，否则会导致采集的数据丢失。

---

## 四、使用问题

### Q14: 如何快速上手 msPTI？

建议按照以下顺序操作：

1. 安装 CANN 并配置环境变量。
2. 进入样例目录：`cd ${install_path}/tools/mspti/samples/mspti_activity`
3. 执行样例脚本：`bash sample_run.sh`

详细说明请参见《[msPTI 快速入门](../quick_start/quick_start.md)》。

### Q15: msPTI 提供哪些 API？

msPTI 提供两类 API：

| API 类型 | 说明 |
| --- | --- |
| **C Activity API** | 采集 Kernel、Memory、HCCL、Marker 等活动数据，用于 Tracing 和 Profiling |
| **C Callback API** | 订阅 Runtime / HCCL 回调，在 API 调用前后执行自定义逻辑 |
| **Python API** | 提供 `KernelMonitor`、`HcclMonitor`、`MstxMonitor`、`CommunicationMonitor` 等高层接口 |

详细接口定义请参见《[C API 参考](../api_reference/c_api/README.md)》和《[Python API 参考](../api_reference/python_api/README.md)》。

### Q16: 有哪些可用的样例？

| 样例 | 接口类型 | 适用场景 |
| --- | --- | --- |
| `callback_domain` | Callback API | Runtime API 回调拦截与前后处理 |
| `callback_mstx` | Callback API + MSTX | Launch Kernel 打点与上下文透传 |
| `mspti_activity` | Activity API | 基础活动数据采集与缓冲区处理 |
| `mspti_correlation` | Activity API | API 下发与 Kernel 执行关联分析 |
| `mspti_external_correlation` | Activity API | 跨层调用链路关联分析 |
| `mspti_hccl_activity` | Activity API | HCCL 通信行为采集与分析 |
| `mspti_mstx_activity_domain` | Activity API + MSTX | 域级打点采集控制 |
| `python_monitor` | Python API | Python 计算与通信耗时采集 |
| `python_mstx_monitor` | Python API + MSTX | Python 自定义打点分析 |

### Q17: Python 样例需要额外安装什么？

Python 样例（`python_monitor`、`python_mstx_monitor`）额外依赖 PyTorch 框架和 `torch_npu` 插件。请参见《[Ascend Extension for PyTorch 安装指南](https://gitcode.com/Ascend/pytorch/blob/v2.7.1-7.3.0/docs/zh/installation_guide/installation_via_binary_package.md)》。

### Q18: 什么是 Activity Buffer？如何管理？

Activity Buffer 是 msPTI 用于缓存 Activity Record 数据的内存缓冲区。用户需注册 `RequestFunc` 和 `CompleteFunc` 回调函数：

1. msPTI 通过 `RequestFunc` 申请空缓冲区。
2. 数据填满后，msPTI 通过 `CompleteFunc` 回调返回满缓冲区。
3. 用户通过 `msptiActivityGetNextRecord` 遍历记录。
4. 消费完毕后，通过 `RequestFunc` 将空缓冲区归还 msPTI。

### Q19: correlationId 的作用是什么？

`correlationId` 用于将 CANN API 调用与其触发的 Kernel 执行、内存操作等 Activity 记录进行关联。通过该字段，您可以建立 API 下发与实际硬件执行的一一对应关系，方便分析性能瓶颈。

### Q20: 外部关联 ID（External Correlation）有什么用？

外部关联 ID 通过 `msptiActivityPushExternalCorrelationId` 和 `msptiActivityPopExternalCorrelationId` 实现压栈/出栈机制，允许将不同层级的 API 调用关联到一起，便于回溯完整的函数调用栈。

### Q21: 如何控制标记（Marker）打点的采集范围？

使用 `msptiActivityEnableMarkerDomain` / `msptiActivityDisableMarkerDomain`（C API）或 `MstxMonitor.enable_domain` / `MstxMonitor.disable_domain`（Python API）按域动态启停打点采集，以控制采集范围和性能开销。

---

## 五、常见错误与排障

### Q22: `pip show mspti` 提示命令不存在？

请确认当前终端使用的是安装 msPTI 的 Python 环境。可通过 `which python` 确认 Python 路径，并切换到正确的虚拟环境。

### Q23: 样例运行报错，提示找不到 `set_env.sh`？

请确认 `${install_path}` 替换为 CANN 实际安装路径，例如 `/usr/local/Ascend/cann`。若不确定安装路径，可通过 `find / -name "set_env.sh" 2>/dev/null` 查找。

### Q24: `ms_install.py` 下载失败或 SSL 证书错误？

参见 [FAQ](https://www.hiascend.com/developer/blog/details/02176213671719317003)。若环境不允许联网，请先在可联网的设备下载该脚本后拷贝到目标设备。

### Q25: 编译源码时提示 CMake 版本过低？

msPTI 要求 CMake 3.14 及以上版本。请升级 CMake 后重试。

### Q26: 运行样例时采集不到数据或数据不完整？

1. 确认 msPTI 是否已正确安装：`pip show mspti`
2. 确认 CANN 环境变量已正确配置：`source ${install_path}/set_env.sh`
3. 确认没有其他性能采集工具在同时运行。
4. 确认使用的昇腾产品在支持列表中。
5. 检查是否正确调用了 `msptiActivityEnable` 使能了对应类型的 Activity Kind。
