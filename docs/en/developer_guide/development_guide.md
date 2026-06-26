# Development Guide

## 1. MindStudio Profiler Tools Interface Development Software

| Software Name | Usage |
| --- | --- |
| CLion (Recommended) / VS Code | Write and debug C/C++ core code under `csrc` |
| PyCharm (Recommended) / VS Code | Write and debug Python wrapper code under `mspti` |
| Git | Pull, manage, and commit code |
| CMake / Make | Build C/C++ code locally |
| Python virtual environment tool (venv) | Isolate Python development dependencies |

## 2. Development Environment Configuration

| Software | Required Version | Usage |
| --- | --- | --- |
| gcc / g++ | Stable version recommended | Compile C/C++ core code |
| CMake | 3.14 and above | C++ build |
| Python | Compatible with the target runtime environment | Run Python interfaces and samples |
| pip | Bundled with Python | Install Python dependencies |
| lcov / genhtml | C++ coverage report generation | Coverage statistics |

### 2.1 Prerequisites

msPTI runtime and development depend on a matching version of the CANN environment. Before starting development, you are advised to complete the following:

1. Install the matching version of the CANN Toolkit development suite package and the operator package.
2. Configure the CANN environment variables.
3. Prepare the third-party dependencies required for building.

Typical environment configuration commands are as follows:

```bash
source ${install_path}/set_env.sh
```

Where `${install_path}` needs to be replaced with the CANN installation path, for example, `/usr/local/Ascend/cann`.

### 2.2 Third-party Dependencies

The repository provides `scripts/download_thirdparty.sh` for downloading the dependencies required for building. It is recommended to execute it before the first build:

```bash
bash scripts/download_thirdparty.sh
```

## 3. Development Procedure

### 3.1 Code Download

```bash
git clone https://gitcode.com/Ascend/mspti.git
cd mspti
```

### 3.2 Project Structure

The current repository mainly consists of the following:

| Directory | Description |
| --- | --- |
| `csrc` | C/C++ core implementation |
| `csrc/activity` | Activity data profiling and parsing |
| `csrc/callback` | Callback subscription and callback management |
| `csrc/common` | Common basic capabilities |
| `csrc/include` | msPTI C API header files |
| `mspti` | Python wrapper |
| `mspti/csrc` | Python extension binding implementation |
| `mspti/monitor` | Kernel / HCCL / MSTX Monitor |
| `samples` | C++ / Python samples |
| `scripts` | Build, packaging, installation, and test scripts |
| `test/mspti_cpp` | C++ tests |
| `docs/en` | Documentation |

### 3.3 C/C++ Core Capability Development

`csrc` is the core implementation directory of msPTI. During development, you can focus on the following paths based on functionality:

| Path | Description |
| --- | --- |
| `csrc/activity` | Profiling, buffering, and parsing logic of Activity API |
| `csrc/callback` | Subscription, callback, and domain management of Callback API |
| `csrc/common` | Shared utilities, adaptation layer, and common capabilities |
| `csrc/include` | Externally exposed C API header files |

Applicable scenarios:

1. Adding new activity types or additional fields
2. Adjusting the callback subscription and callback flow
3. Modifying public data structures or interface definitions
4. Extending external C APIs

### 3.4 Python Interface Development

The `mspti` directory provides Python encapsulation and monitoring capabilities. Focus on the following during development:

| Path | Description |
| --- | --- |
| `mspti/csrc` | Python extension bindings |
| `mspti/monitor` | `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, etc. |
| `mspti/activity_data.py` | Activity data encapsulation |
| `mspti/constant.py` | Constant definitions |
| `mspti/utils.py` | General utility functions |

Applicable scenarios:

1. Adding new Python Monitor capabilities
2. Adding Python wrapper interfaces
3. Adjusting Python-layer data structures or return formats
4. Extending Python sample capabilities

### 3.5 Sample Development

The `samples` directory is used to demonstrate typical API usage. The current samples cover:

- Callback API
- Activity API
- Correlation scenarios
- HCCL activity profiling
- Python Monitor
- Python MSTX Monitor

If new APIs are added or API capabilities are enhanced, it is recommended to supplement samples accordingly and update:

- `docs/en/getting_started/samples_guide.md`

### 3.6 Common Development Scenarios

#### 3.6.1 Developing Activity API

If this change involves the Activity API:

1. Check `csrc/activity` first.
2. Confirm whether the external header files in `csrc/include` need to be modified synchronously.
3. Add corresponding samples and documentation descriptions.
4. Verify related samples such as `samples/mspti_activity` and `samples/mspti_hccl_activity`.

#### 3.6.2 Developing Callback API

If this change involves the Callback API:

1. Check `csrc/callback` first.
2. Verify the domain, callback registration, and callback execution logic.
3. Verify `samples/callback_domain` and `samples/callback_mstx`.
4. Synchronously update the C API documentation.

#### 3.6.3 Developing Python Monitor

If this change involves Python Monitor:

1. First check `mspti/monitor` and `mspti/csrc`.
2. Verify the interface names and parameters exported by the Python layer.
3. Verify `samples/python_monitor` and `samples/python_mstx_monitor`.
4. Synchronously update the Python API documentation.

## 4. Build and Installation

### 4.1 Building the RUN Package

This repository provides a unified build script `scripts/build.sh`. This script will:

1. Download third-party dependencies.
2. Execute CMake configuration and compilation.
3. Install to a temporary prefix directory.
4. Call `scripts/make_run.sh` to generate a run package.

Common commands are as follows:

```bash
# Default release build
bash scripts/build.sh

# Debug build
bash scripts/build.sh Debug

# Build with specified version number
bash scripts/build.sh v1.2.3
```

After the build is complete, the following will be generated in the `output` directory:

```text
mindstudio-profiler-tools-interface_<version>_<arch>.run
```

### 4.2 Installation Verification

```bash
chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
```

After installation, at least verify that:

1. msPTI-related files have been generated in the CANN directory.
2. The samples in the `samples` directory can run normally.
3. The basic calls of C API and Python API work normally.

## 5. Testing and Verification

### 5.1 Unit Test Build

The repository provides `scripts/execute_test_case.sh` for building C++ unit tests:

```bash
bash scripts/execute_test_case.sh
```

The script will:

1. Download third-party dependencies.
2. Execute CMake build under `test/build_llt`.
3. Build the test target with `PACKAGE=ut`.

### 5.2 C++ Coverage

To generate a C++ coverage report, run:

```bash
bash scripts/generate_coverage_cpp.sh
```

After execution, the report will be generated in the following directory:

```text
test/build_llt/output/cpp_coverage/result
```

To compare incremental coverage, run:

```bash
bash scripts/generate_coverage_cpp.sh diff
```

### 5.3 Typical Test Targets

Based on the coverage script, the current key test targets include:

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

After completing feature development, it is recommended to verify at least one corresponding sample. Typical commands are as follows:

```bash
source ${install_path}/set_env.sh
cd ${install_path}/tools/mspti/samples/callback_domain
bash sample_run.sh
```

If you are developing the Python Monitor, it is recommended to additionally verify:

- `samples/python_monitor`
- `samples/python_mstx_monitor`

## 6. Document Linkage Update

After feature development is complete, if the changes affect interfaces, samples, installation methods, or behavior descriptions, the documentation must be updated synchronously.

| Change Type | Documents to be Updated |
| --- | --- |
| Installation, packaging, uninstallation | [msPTI Tool Installation Guide](../getting_started/mspti_install_guide.md) |
| Tool introduction | [msPTI Tool Sample Guide](../getting_started/samples_guide.md) |
| C API changes | [C API Overview](../c_api/README.md) and its sub-documents |
| Python API changes | [Python API Overview](../python_api/README.md) and its sub-documents |
| Sample updates | [msPTI Sample Description](../getting_started/samples_guide.md#mspti-samples) |
| Version release information | [Release Notes](../release_notes.md) |

## 7. Suggestions on the Commit Process

1. After feature development is complete, first complete the local build verification.
2. If external interface changes are involved, prioritize supplementing header files, samples, and documentation.
3. At least complete the relevant C++ test build, and generate a coverage report if necessary.
4. If user-visible behavior changes are involved, synchronously update the installation instructions, API documentation, and sample descriptions.
