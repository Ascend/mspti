# Development Guide

## 1. MindStudio Profiler Tools Interface Development Software

| Software | Purpose |
| --- | --- |
| CLion (recommended)/VS Code | Write and debug the C/C++ core code in the `csrc` directory. |
| PyCharm (recommended)/VS Code | Write and debug the Python wrapper code in the `mspti` directory. |
| Git | Fetch, manage, and commit code. |
| CMake/Make | Build C/C++ code locally. |
| Python virtual environment tool (venv) | Isolate Python development dependencies. |

## 2. Development Environment Configuration

| Software | Version Requirement | Purpose |
| --- | --- | --- |
| gcc/g++ | A stable version is recommended | Compile the C/C++ core code. |
| CMake | 3.14 or later | C++ build |
| Python | Match the target runtime environment | Run the Python interfaces and samples. |
| pip | Match the Python version | Install Python dependencies. |
| lcov/genhtml | Generate C++ coverage reports. | Coverage statistics |

### 2.1 Prerequisites

To run and develop msPTI, you need a CANN environment of a matching version. Before you start development, you are advised to complete the following steps:

1. Install the matching version of the CANN Toolkit development kit and the ops operator package.
2. Configure the CANN environment variables.
3. Prepare the third-party dependencies required for the build.

A typical environment configuration command is as follows:

```bash
source ${install_path}/set_env.sh
```

Here, replace `${install_path}` with the CANN installation path, for example `/usr/local/Ascend/cann`.

### 2.2 Third-Party Dependencies

The repository provides `scripts/download_thirdparty.sh` to download the dependencies required for the build. You are advised to run it before the first build:

```bash
bash scripts/download_thirdparty.sh
```

## 3. Development Steps

### 3.1 Code Download

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
```

### 3.2 Project Structure Description

The repository mainly consists of the following parts:

| Directory | Description |
| --- | --- |
| `csrc` | C/C++ core implementation |
| `csrc/activity` | Activity data collection and parsing |
| `csrc/callback` | Callback subscription and callback management |
| `csrc/common` | Common foundation capabilities |
| `csrc/include` | msPTI C API header files |
| `mspti` | Python wrapper |
| `mspti/csrc` | Python extension binding implementation |
| `mspti/monitor` | Kernel/HCCL/MSTX Monitor |
| `samples` | C++/Python samples |
| `scripts` | Build, packaging, installation, and testing scripts |
| `test/mspti_cpp` | C++ tests |
| `docs/zh` | Chinese documentation |

### 3.3 C/C++ Core Capability Development

The `csrc` directory is the core implementation directory of msPTI. During development, focus on the following paths based on functionality:

| Path | Description |
| --- | --- |
| `csrc/activity` | Collection, buffering, and parsing logic of the Activity API |
| `csrc/callback` | Subscription, callback, and domain management of the Callback API |
| `csrc/common` | Shared utilities, adaptation layer, and common capabilities |
| `csrc/include` | C API header files exposed externally |

Applicable scenarios:

1. Add new Activity types or additional fields.
2. Adjust the Callback subscription and callback process.
3. Modify common data structures or interface definitions.
4. Extend the externally exposed C API.

### 3.4 Python Interface Development

The `mspti` directory provides the Python wrapper and monitoring capabilities. During development, focus on the following:

| Path | Description |
| --- | --- |
| `mspti/csrc` | Python extension bindings |
| `mspti/monitor` | `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, `CommunicationMonitor`, and so on |
| `mspti/activity_data.py` | Activity data wrapper |
| `mspti/constant.py` | Constant definitions |
| `mspti/utils.py` | Common utility functions |

Applicable scenarios:

1. Add new Python Monitor capabilities.
2. Add Python wrapper interfaces.
3. Adjust Python-layer data structures or return formats.
4. Extend the Python sample capabilities.

### 3.5 Sample Development

The `samples` directory demonstrates typical interface usage. The current samples cover:

- Callback API
- Activity API
- Correlation scenarios
- HCCL activity collection
- Python Monitor
- Python MSTX Monitor

If you add new interfaces or enhance existing interface capabilities, you are advised to add corresponding samples and update the following files:

- `samples/README.md`
- `docs/en`

### 3.6 Common Development Scenarios

#### 3.6.1 Activity API Development

If this change involves the Activity API:

1. Check `csrc/activity` first.
2. Confirm whether the external header files in `csrc/include` need to be modified accordingly.
3. Add corresponding samples and documentation.
4. Verify related samples such as `samples/mspti_activity` and `samples/mspti_hccl_activity`.

#### 3.6.2 Callback API Development

If this change involves the Callback API:

1. Check `csrc/callback` first.
2. Verify the domain, callback registration, and callback execution logic.
3. Verify `samples/callback_domain` and `samples/callback_mstx`.
4. Update the C API documentation accordingly.

#### 3.6.3 Python Monitor Development

If this change involves Python Monitor:

1. Check `mspti/monitor` and `mspti/csrc` first.
2. Verify the interface names and parameters exported at the Python layer.
3. Verify `samples/python_monitor` and `samples/python_mstx_monitor`.
4. Update the Python API documentation accordingly.

## 4. Build and Installation

### 4.1 Run Package Build

The repository provides a unified build script, `scripts/build.sh`. The script does the following:

1. Download third-party dependencies.
2. Run the CMake configuration and compilation.
3. Install to a temporary prefix directory.
4. Call `scripts/make_run.sh` to generate the run package.

Common commands are as follows:

```bash
# Default Release build
bash scripts/build.sh

# Debug build
bash scripts/build.sh Debug

# Build with a specified version number
bash scripts/build.sh v1.2.3
```

After the build completes, the following file is generated in the `output` directory:

```text
mindstudio-profiler-tools-interface_<version>_<arch>.run
```

### 4.2 Installation Verification

```bash
chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
```

After installation, you are advised to verify at least the following:

1. msPTI-related files are generated in the CANN directory.
2. The samples in the `samples` directory run correctly.
3. Basic calls to the C API and Python API work correctly.

## 5. Testing and Validation

### 5.1 Unit Test Build

The repository provides `scripts/execute_test_case.sh` to build the C++ unit tests:

```bash
bash scripts/execute_test_case.sh
```

The script does the following:

1. Download third-party dependencies.
2. Run the CMake build in the `test/build_llt` directory.
3. Build the test targets with `PACKAGE=ut`.

### 5.2 C++ Coverage

To generate a C++ coverage report, run the following command:

```bash
bash scripts/generate_coverage_cpp.sh
```

After the script completes, the report is generated in the following directory:

```text
test/build_llt/output/cpp_coverage/result
```

To compare incremental coverage, run the following command:

```bash
bash scripts/generate_coverage_cpp.sh diff
```

### 5.3 Typical Test Targets

According to the coverage script, the current key test targets include:

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

### 5.4 Sample Verification

After completing feature development, you are advised to verify at least one corresponding sample. A typical command is as follows:

```bash
source ${install_path}/set_env.sh
cd ${install_path}/tools/mspti/samples/callback_domain
bash sample_run.sh
```

If you are developing a Python Monitor, you are advised to additionally verify the following samples:

- `samples/python_monitor`
- `samples/python_mstx_monitor`

## 6. Synchronized Documentation Updates

After completing feature development, if the changes affect interfaces, samples, installation methods, or behavior descriptions, update the documentation accordingly.

| Change Type | Documentation to Update |
| --- | --- |
| Installation, packaging, and uninstallation | [msPTI Tool Installation Guide](../install_guide/mspti_install_guide.md) |
| Tool introduction | [msPTI Tool Sample Guide](../user_guide/samples_guide.md) |
| C API changes | [C API Overview](../api_reference/c_api/README.md) and its sub-documents |
| Python API changes | [Python API Overview](../api_reference/python_api/README.md) and its sub-documents |
| Sample updates | [msPTI Sample Description](../../../samples/README.md) |
| Version release information | [Release Notes](https://gitcode.com/Ascend/mspti/releases) |

## 7. Suggested Commit Process

1. After completing feature development, first verify the build locally.
2. If external interfaces are affected, update the header files, samples, and documentation first.
3. Complete at least the related C++ test builds, and generate coverage reports if necessary.
4. If the changes affect user-visible behavior, update the installation instructions, API documentation, and sample descriptions accordingly.
