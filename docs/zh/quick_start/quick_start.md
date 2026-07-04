# msPTI 快速入门

## 1. 概述

msPTI（MindStudio Profiler Tools Interface）是华为昇腾 MindStudio 提供的一套性能剖析 API 集合。您可以使用 msPTI 构建面向 NPU 应用的性能分析工具，应用于推理和训练场景。

> 建议按照 **安装工具 → 配置环境 → 运行样例** 的顺序完成快速体验。

---

## 2. 环境准备

### 2.1 硬件环境

- 一台搭载支持昇腾 NPU 的服务器（参见[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)）。

### 2.2 软件环境

- **CANN（含 msPTI）**：建议优先安装 CANN 软件（Toolkit + ops 包），msPTI 已集成于 CANN 中。若已安装 CANN，则可直接使用。
  - CANN 快速安装：[昇腾社区 CANN 下载](https://www.hiascend.com/cann/download)
- **Python 环境**：若使用 msPTI Python API，请确保 Python 3.10+ 环境已配置。
- **PyTorch + torch_npu**（可选）：Python Monitor 样例依赖 PyTorch 框架和 Ascend Extension for PyTorch（[安装指南](https://gitcode.com/Ascend/pytorch/blob/v2.7.1-7.3.0/docs/zh/installation_guide/installation_via_binary_package.md)）。

### 2.3 约束说明

msPTI 不可与其他性能数据采集工具同时使用，否则会导致采集的数据丢失。

---

## 3. 工具安装

msPTI 已集成于 CANN 中，若已安装 CANN 且无需升级此工具，可直接跳过本步骤。

如需单独升级或安装最新版本，请参见《[msPTI 工具安装指南](../install_guide/mspti_install_guide.md)》。

安装完成后，执行以下命令验证：

```bash
pip show mspti
```

若输出版本信息且无报错，则表示安装成功。

---

## 4. 快速运行样例

### 4.1 配置 CANN 环境变量

```bash
source ${install_path}/set_env.sh
```

其中 `${install_path}` 为 CANN 安装路径，例如 `/usr/local/Ascend/cann`。

### 4.2 运行基础样例

进入 Activity API 基础样例目录并执行：

```bash
cd ${install_path}/tools/mspti/samples/mspti_activity
bash sample_run.sh
```

运行成功后输出示例如下：

```bash
...
========== UserBufferRequest ============
result[0] is: 1.200000
result[1] is: 2.200000
result[2] is: 3.200000
result[3] is: 5.400000
result[4] is: 6.400000
result[5] is: 7.400000
result[6] is: 9.600000
result[7] is: 10.600000
========== UserBufferComplete ============
[RUNTIME_API] name: DevMalloc, start: 1775186328012443375, end: 1775186328012485525, ...
[MEMORY] operationType: ALLOCATION, memoryKind: MEMORY_DEVICE, ...
[RUNTIME_API] name: MemCopySync, start: 1775186328012545645, end: 1775186328012596965, ...
[MEMCPY] copyKind: HTOD, bytes: 32, ...
...
```

---
