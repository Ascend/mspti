# msPTI Development Guide

## 1. Compilation Environment Preparation

### 1.1 Method 1: One-Click devcontainer Development Environment (Recommended)

msPTI includes a built-in [devcontainer](https://containers.dev/) development environment configuration. After opening the repository in VS Code, you can enter a standardized container with one click, without manually installing any dependencies. The container automatically completes the following preparations:

- Python 3 environment and compilation toolchain (GCC 11.2.0 and CMake 3.14+)
- Installation of system dependencies (python3-devel and pip packages)
- Pre-download of third-party dependencies (googletest, mockcpp, boost, and securec)
- Automatic enabling of pre-commit (including gitleaks secret scanning)
- Ready-to-use clangd C++ language service
- Git identity synchronization

**Prerequisites:**

| Environment | Requirement |
| --- | --- |
| PC | VS Code with the Dev Containers and Remote-SSH extensions installed |
| Linux server | Docker service running |

**Procedure:**

1. Clone the msPTI repository to the Linux server.
2. Connect to the server through Remote-SSH in VS Code and open the repository directory.
3. VS Code automatically detects the `.devcontainer` configuration. Click **"Reopen in Container"** in the bottom-left corner.
4. The container automatically runs `post-create.sh` to complete the initialization (about 1 to 2 minutes).
5. Press `Ctrl+Shift+P` → `Tasks: Run Task` and select a build task.

**VS Code Built-in Tasks:**

| Task | Shortcut | Description |
| --- | --- | --- |
| Build: Release Mode | `Ctrl+Shift+B` | One-click Release build (equivalent to `bash scripts/build.sh`) |
| Build: Debug Mode | — | Debug compilation (with debug symbols) |
| Test: Run Unit Tests | — | Run all C++ unit tests. |
| Clean: All Workspace | — | Clean all build artifacts (`build/`, `prefix/`, `output/`, `test/build_llt/`, and so on). |

**Code Intelligence and Navigation:**

- C++: After the build, use clangd for cross-file navigation (`F12`), reference search (`Shift+F12`), and code completion.
- Python: Pylance provides semantic navigation and type inference.

**Graphical Debugging:**

- Open a Python source file and press `F5` to select `Python: Debug Active File` and start debugging with debugpy.
- Breakpoints, variable inspection, the call stack, and other features all work normally.

### 1.2 Method 2: Manual Environment Configuration

Follow the [msPTI Tool Installation Guide — Compilation Environment Preparation](../install_guide/mspti_install_guide.md#231-environment-preparation) section to set up the build and test environment.

> **Note:** The environment image build method and the supporting software versions are maintained by the MindStudio Unified Build Image Creation Guide and are not redefined in this repository.

## 2. Development Steps

### 2.1 Code Download

In the compilation container environment, run the following command to download the code:

```bash
git clone https://atomgit.com/Ascend/mspti.git
cd mspti
```

### 2.2 Project Structure Description

The repository mainly consists of the following parts:

| Directory | Description |
| --- | --- |
| csrc | C/C++ core implementation |
| csrc/activity | Activity data collection and parsing |
| csrc/callback | Callback subscription and callback management |
| csrc/common | Common foundation capabilities |
| csrc/include | msPTI C API header files |
| mspti | Python wrapper |
| mspti/csrc | Python extension binding implementation |
| mspti/monitor | Kernel/HCCL/MSTX Monitor |
| samples | C++/Python samples |
| scripts | Build, packaging, installation, and testing scripts |
| test/mspti_cpp | C++ tests |
| docs/zh | Chinese documentation |

### 2.3 C/C++ Core Capability Development

The `csrc` directory is the core implementation directory of msPTI. During development, focus on the following paths based on functionality:

| Path | Description |
| --- | --- |
| csrc/activity | Collection, buffering, and parsing logic of the Activity API |
| csrc/callback | Subscription, callback, and domain management of the Callback API |
| csrc/common | Shared utilities, adaptation layer, and common capabilities |
| csrc/include | C API header files exposed externally |

Applicable scenarios:

1. Add new Activity types or additional fields.
2. Adjust the Callback subscription and callback process.
3. Modify common data structures or interface definitions.
4. Extend the externally exposed C API.

### 2.4 Python Interface Development

The `mspti` directory provides the Python wrapper and monitoring capabilities. During development, focus on the following:

| Path | Description |
| --- | --- |
| mspti/csrc | Python extension bindings |
| mspti/monitor | `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, `CommunicationMonitor`, and so on |
| mspti/activity_data.py | Activity data wrapper |
| mspti/constant.py | Constant definitions |
| mspti/utils.py | Common utility functions |

Applicable scenarios:

1. Add new Python Monitor capabilities.
2. Add Python wrapper interfaces.
3. Adjust Python-layer data structures or return formats.
4. Extend the Python sample capabilities.

### 2.5 Sample Development

The `samples` directory demonstrates typical interface usage. The current samples cover:

- Callback API
- Activity API
- Correlation scenarios
- HCCL activity collection
- Python Monitor
- Python MSTX Monitor

If you add new interfaces or enhance existing interface capabilities, you are advised to add corresponding samples and update the following files:

- `samples/README.md`
- `docs/zh/README.md`

### 2.6 Common Development Scenarios

#### 2.6.1 Activity API Development

If this change involves the Activity API:

1. Check `csrc/activity` first.
2. Confirm whether the external header files in `csrc/include` need to be modified accordingly.
3. Add corresponding samples and documentation.
4. Verify related samples such as `samples/mspti_activity` and `samples/mspti_hccl_activity`.

#### 2.6.2 Callback API Development

If this change involves the Callback API:

1. Check `csrc/callback` first.
2. Verify the domain, callback registration, and callback execution logic.
3. Verify `samples/callback_domain` and `samples/callback_mstx`.
4. Update the C API documentation accordingly.

#### 2.6.3 Python Monitor Development

If this change involves Python Monitor:

1. Check `mspti/monitor` and `mspti/csrc` first.
2. Verify the interface names and parameters exported at the Python layer.
3. Verify `samples/python_monitor` and `samples/python_mstx_monitor`.
4. Update the Python API documentation accordingly.

## 3. Build and Installation

### 3.1 Third-Party Dependencies

The repository provides `scripts/download_thirdparty.sh` to download the dependencies required for the build. You are advised to run it before the first build:

```bash
bash scripts/download_thirdparty.sh
```

### 3.2 Run Package Build

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

### 3.3 Installation Verification

```bash
chmod +x mindstudio-profiler-tools-interface_<version>_<arch>.run
./mindstudio-profiler-tools-interface_<version>_<arch>.run --install
```

After installation, you are advised to verify at least the following:

1. msPTI-related files are generated in the CANN directory.
2. The samples in the `samples` directory run correctly.
3. Basic calls to the C API and Python API work correctly.

## 4. Testing and Validation

### 4.1 Unit Test Build

The repository provides `scripts/execute_test_case.sh` to build the C++ unit tests:

```bash
bash scripts/execute_test_case.sh
```

The script does the following:

1. Download third-party dependencies.
2. Run the CMake build in the `test/build_llt` directory.
3. Build the test targets with `PACKAGE=ut`.

### 4.2 C++ Coverage

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

### 4.3 Typical Test Targets

According to the coverage script, the current key test targets include:

- `activity_utest`
- `mspti_channel_utest`
- `dev_task_utest`
- `mspti_parser_utest`
- `mspti_reporter_utest`
- `callback_utest`
- `context_manager_utest`
- `function_loader_utest`
- `mspti_utils_utest`
- `mspti_adapter_utest`

### 4.4 Sample Verification

After completing feature development, you are advised to verify at least one corresponding sample. A typical command is as follows:

```bash
source ${install_path}/set_env.sh
cd ${install_path}/tools/mspti/samples/callback_domain
bash sample_run.sh
```

`${install_path}` should be replaced with the actual CANN installation path, for example `/usr/local/Ascend/cann`.

If you are developing a Python Monitor, you are advised to additionally verify the following samples:

- `samples/python_monitor`
- `samples/python_mstx_monitor`

## 5. Synchronized Documentation Updates

After completing feature development, if the changes affect interfaces, samples, installation methods, or behavior descriptions, update the documentation accordingly.

| Change Type | Documentation to Update |
| --- | --- |
| Installation, packaging, and uninstallation | [msPTI Tool Installation Guide](../install_guide/mspti_install_guide.md) |
| Tool introduction | [msPTI Tool Sample Guide](../user_guide/samples_guide.md) |
| C API changes | [C API Overview](../api_reference/c_api/README.md) and its sub-documents |
| Python API changes | [Python API Overview](../api_reference/python_api/README.md) and its sub-documents |
| Sample updates | [msPTI Sample Description](../../../samples/README.md) |
| Version release information | [Release Notes](https://atomgit.com/Ascend/mspti/releases) |

## 6. Suggested Commit Process

1. After completing feature development, first verify the build locally.
2. If external interfaces are affected, update the header files, samples, and documentation first.
3. Complete at least the related C++ test builds, and generate coverage reports if necessary.
4. If the changes affect user-visible behavior, update the installation instructions, API documentation, and sample descriptions accordingly.
