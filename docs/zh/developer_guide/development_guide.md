# 开发指南

## 1. MindStudio Profiler Tools Interface开发软件

| 软件名 | 用途 |
| --- | --- |
| CLion（推荐）/ VS Code | 编写和调试 `csrc` 下的 C/C++ 核心代码 |
| PyCharm（推荐）/ VS Code | 编写和调试 `mspti` 下的 Python 封装代码 |
| Git | 拉取、管理和提交代码 |
| CMake / Make | 本地构建 C/C++ 代码 |
| Python 虚拟环境工具（venv） | 隔离 Python 开发依赖 |

## 2. 开发环境配置

| 软件名 | 版本要求 | 用途 |
| --- | --- | --- |
| gcc / g++ | 推荐使用稳定版本 | 编译 C/C++ 核心代码 |
| CMake | 3.14 及以上 | C++ 构建 |
| Python | 与目标运行环境匹配 | 运行 Python 接口与样例 |
| pip | 与 Python 配套 | 安装 Python 依赖 |
| lcov / genhtml | 生成 C++ 覆盖率报告 | 覆盖率统计 |

### 2.1 前置环境

msPTI 运行和开发依赖配套版本的 CANN 环境。开始开发前，建议先完成：

1. 安装配套版本的 CANN Toolkit 开发套件包和 ops 算子包。
2. 配置 CANN 环境变量。
3. 准备构建所需的第三方依赖。

典型环境配置命令如下：

```bash
source ${install_path}/set_env.sh
```
其中`${install_path}`需要替换为CANN的安装路径，例如`/usr/local/Ascend/cann`。

### 2.2 三方依赖

仓库提供 `scripts/download_thirdparty.sh` 用于下载构建所需依赖。在首次构建前建议先执行：

```bash
bash scripts/download_thirdparty.sh
```

## 3. 开发步骤

### 3.1 代码下载

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
```

### 3.2 项目结构说明

当前仓库主要由以下几部分组成：

| 目录 | 说明 |
| --- | --- |
| `csrc` | C/C++ 核心实现 |
| `csrc/activity` | Activity 数据采集与解析 |
| `csrc/callback` | Callback 订阅与回调管理 |
| `csrc/common` | 公共基础能力 |
| `csrc/include` | msPTI C API 头文件 |
| `mspti` | Python 封装 |
| `mspti/csrc` | Python 扩展绑定实现 |
| `mspti/monitor` | Kernel / HCCL / MSTX Monitor |
| `samples` | C++ / Python 样例 |
| `scripts` | 构建、打包、安装、测试脚本 |
| `test/mspti_cpp` | C++ 测试 |
| `docs/zh` | 中文文档 |

### 3.3 C/C++ 核心能力开发

`csrc` 是 msPTI 的核心实现目录。开发时可按功能关注以下路径：

| 路径 | 说明 |
| --- | --- |
| `csrc/activity` | Activity API 的采集、缓冲、解析逻辑 |
| `csrc/callback` | Callback API 的订阅、回调、domain 管理 |
| `csrc/common` | 共享工具、适配层和公共能力 |
| `csrc/include` | 对外暴露的 C API 头文件 |

适用场景：

1. 新增 Activity 类型或附加字段。
2. 调整 Callback 订阅和回调流程。
3. 修改公共数据结构或接口定义。
4. 扩展对外 C API。

### 3.4 Python 接口开发

`mspti` 目录提供 Python 封装和监控能力，开发时重点关注：

| 路径 | 说明 |
| --- | --- |
| `mspti/csrc` | Python 扩展绑定 |
| `mspti/monitor` | `KernelMonitor`、`HcclMonitor`、`MstxMonitor` 等 |
| `mspti/activity_data.py` | Activity 数据封装 |
| `mspti/constant.py` | 常量定义 |
| `mspti/utils.py` | 通用工具函数 |

适用场景：

1. 新增 Python Monitor 能力。
2. 增加 Python 封装接口。
3. 调整 Python 层数据结构或返回格式。
4. 扩展 Python 样例能力。

### 3.5 样例开发

`samples` 目录用于演示典型接口用法。当前样例覆盖：

- Callback API
- Activity API
- Correlation 场景
- HCCL 活动采集
- Python Monitor
- Python MSTX Monitor

若新增接口或增强接口能力，建议同步补充样例，并更新：

- `samples/README.md`
- `docs/zh/README.md`

### 3.6 常见开发场景

#### 3.6.1 开发 Activity API

若本次改动涉及 Activity API：

1. 优先检查 `csrc/activity`。
2. 确认是否需要同步修改 `csrc/include` 对外头文件。
3. 补充对应样例和文档说明。
4. 验证 `samples/mspti_activity`、`samples/mspti_hccl_activity` 等相关样例。

#### 3.6.2 开发 Callback API

若本次改动涉及 Callback API：

1. 优先检查 `csrc/callback`。
2. 核对 domain、callback 注册和回调执行逻辑。
3. 验证 `samples/callback_domain`、`samples/callback_mstx`。
4. 同步更新 C API 文档。

#### 3.6.3 开发 Python Monitor

若本次改动涉及 Python Monitor：

1. 优先检查 `mspti/monitor` 和 `mspti/csrc`。
2. 核对 Python 层导出的接口名称和参数。
3. 验证 `samples/python_monitor`、`samples/python_mstx_monitor`。
4. 同步更新 Python API 文档。

## 4. 构建与安装

### 4.1 构建 run 包

仓库提供 `scripts/build.sh` 统一构建脚本。该脚本会：

1. 下载第三方依赖。
2. 执行 CMake 配置和编译。
3. 安装到临时前缀目录。
4. 调用 `scripts/make_run.sh` 生成 run 包。

常用命令如下：

```bash
# 默认 Release 构建
bash scripts/build.sh

# Debug 构建
bash scripts/build.sh Debug

# 指定版本号构建
bash scripts/build.sh v1.2.3
```

构建完成后，会在 `output` 目录下生成：

```text
mindstudio-profiler-tools-interface_<version>_<arch>.run
```

### 4.2 安装验证

```bash
chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
```

安装完成后，建议至少验证：

1. CANN 目录下已生成 msPTI 相关文件。
2. `samples` 目录样例可正常运行。
3. C API 和 Python API 的基础调用正常。

## 5. 测试与验证

### 5.1 单元测试构建

仓库提供 `scripts/execute_test_case.sh` 用于构建 C++ 单元测试：

```bash
bash scripts/execute_test_case.sh
```

该脚本会：

1. 下载第三方依赖。
2. 在 `test/build_llt` 下执行 CMake 构建。
3. 以 `PACKAGE=ut` 构建测试目标。

### 5.2 C++ 覆盖率

若需要生成 C++ 覆盖率报告，可执行：

```bash
bash scripts/generate_coverage_cpp.sh
```

执行完成后，会在以下目录生成报告：

```text
test/build_llt/output/cpp_coverage/result
```

若需要比较增量覆盖率，可执行：

```bash
bash scripts/generate_coverage_cpp.sh diff
```

### 5.3 典型测试目标

根据覆盖率脚本，当前重点测试目标包括：

- `activity_utest`
- `mspti_channel_utest`
- `dev_prof_task_utest`
- `mspti_parser_utest`
- `mspti_reporter_utest`
- `callback_utest`
- `context_manager_utest`
- `function_loader_utest`
- `mspti_utils_utest`
- `mspti_adapter_utest`

### 5.4 样例验证

功能开发完成后，建议至少验证一个对应样例。典型命令如下：

```bash
source ${install_path}/set_env.sh
cd ${install_path}/tools/mspti/samples/callback_domain
bash sample_run.sh
```

如开发的是 Python Monitor，建议额外验证：

- `samples/python_monitor`
- `samples/python_mstx_monitor`

## 6. 文档联动更新

功能开发完成后，若改动影响接口、样例、安装方式或行为说明，需要同步更新文档。

| 改动类型 | 需同步更新的文档 |
| --- | --- |
| 安装、打包、卸载 | 《[msPTI工具安装指南](../getting_started/mspti_install_guide.md)》 |
| 工具介绍 | 《[msPTI工具样例指南](../getting_started/samples_guide.md)》|
| C API 变更 | 《[C API总体说明](../c_api/README.md)》及其子文档 |
| Python API 变更 | 《[Python API总体说明](../python_api/README.md)》及其子文档 |
| 样例更新 | 《[msPTI样例说明](../../../samples/README.md)》 |
| 版本发布信息 | 《[版本说明](../release_notes.md)》 |

## 7. 提交流程建议

1. 功能开发完成后，先完成本地构建验证。
2. 若涉及对外接口变化，优先补充头文件、样例和文档。
3. 至少完成相关 C++ 测试构建，必要时生成覆盖率报告。
4. 若涉及用户可见行为变化，同步更新安装说明、API 文档和样例说明。
