<h1 align="center">MindStudio Profiler Tools Interface</h1>

<div align="center">
  <p><b>Ascend Profiling Tool Interfaces</b></p>

[📖 User Guide](./docs/en/README.md) |
[🛠️ Installation Guide](./docs/en/mspti_install_guide.md) |
[📚 API Reference](./docs/en/c_api/README.md) |
[🌐 Software Download](https://gitcode.com/Ascend/mspti/releases)

</div>

<br>

## 📢 What's New

* [2026.02.06]: Added the : Added the `26.0.0-alpha.1` release notes and compatibility with CANN `> 8.5.0`. For details, see [Release Notes](./docs/en/release_notes.md).

## 📌 Overview

MindStudio Profiler Tools Interface (msPTI) is a collection of profiling APIs for Ascend devices. It helps developers build performance profiling and analysis tools for NPU applications, and is applicable to inference and training scenarios.

msPTI provides the following capabilities:

- `Tracing`: collects timestamps and additional information about CANN APIs, kernels, memory copy, communication, and dotting, to locate performance bottlenecks in the execution link.
- `Profiling`: collects the NPU performance metrics of one or a group of kernels separately, to support computing and communication analysis.

## 🔍 Directory Structure

```text
├─docs
│ └─en # English documents, installation guide, release notes, security statement, and API reference
├─csrc # C/C++ core implementation
│ ├─activity # Activity data collection and parsing
│ ├─callback # Callback subscription and callback management
│ ├─common # Common basic capabilities
│ └─include # msPTI C API header files
├─mspti # Python encapsulation
│  ├─monitor             # Kernel / HCCL / MSTX Monitor
│ └─csrc # Implementation of Python extension binding
├─samples # C++/Python samples
├─scripts # Scripts for building, packaging, installation, and testing
├─test # UT/ST test code
├─CMakeLists.txt # C++ build entry
└─README.md # Repository overview
```

## 📖 Functions

| Module| Description| Documentation Entry|
| --- | --- | --- |
| `Activity API` | Collects activity data of APIs, kernels, memory, HCCL, markers, and external correlations, which is used to build tracing and profiling tools.| [C API Reference](./docs/en/c_api/README.md)|
| `Callback API` | Subscribes to Runtime/HCCL callbacks and executes custom logic or collects associated data before and after API calls.| [C API Reference](./docs/en/c_api/README.md)|
| `Python API` | Provides APIs such as `KernelMonitor`, `HcclMonitor` and `MstxMonitor` to quickly access the Python scenario analysis capability.| [Python API Reference](./docs/en/python_api/README.md)|
| `Sample Set`| It covers typical scenarios such as callback, activity, correlation, HCCL, and Python monitor, facilitating quick start.| [Sample Description](./samples/README.md) / [User Guide](./docs/zh/README.md)|

## 🛠️ Installation Guide

The msPTI running depends on the CANN environment of the matching version. Before installing msPTI, prepare the following environment:

- For hardware environment requirements, see [Ascend Product Models](<>).
- For details about the software environment, see [CANN Software Installation Guide](https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/softwareinst/instg/instg_quick.html?Mode=PmIns&InstallType=local&OS=openEuler&Software=cannToolKit) to install the CANN Toolkit and ops operator package of the matching version.

After the preceding preparations are complete, you can install msPTI in either of the following ways:

- Method 1: Download the pre-built `run` package from the [releases page](https://gitcode.com/Ascend/mspti/releases), perform MD5 verification, and install the package.
- Method 2: Run the `bash scripts/build.sh [<version>]` command in the source code repository to build the `run` package and then install the package.

For details about the environment preparation, two installation methods, installation parameters, and example commands, see [msPTI Installation Guide](./docs/zh/mspti_install_guide.md).

## 🚀 Quick Start

You are advised to complete the quick experience in the following sequence: Installing the tool > Configuring the environment > Running the sample.

1. Install the tool.

   You have installed the msPTI tool by downloading the `run` package or building the `run` package from source code. For details, see the msPTI Installation Guide (./docs/zh/mspti_install_guide.md).

2. Configure the CANN environment variables.

   ```bash
   source ${install_path}/set_env.sh
   ```

3. Go to the sample directory and run the script.

   ```bash
   cd ${install_path}/tools/mspti/samples/callback_domain
   bash sample_run.sh
   ```

Replace `${install_path}` with the CANN installation path, for example, `/usr/local/Ascend/cann`.

For details about the application scenarios, capabilities, and supplementary information of each sample in the `samples` directory, see the Samples Description (./samples/README.md).

## 📝 Additional Information

- [Release Notes](./docs/zh/release_notes.md)
- [C API Reference](./docs/zh/c_api/README.md)
- [Python API Reference](./docs/zh/python_api/README.md)
- [Security Statement](./docs/en/security_statement.md)
- [LICENSE](./LICENSE)
- [Third_Party_Open_Source_Software_Notice](./Third_Party_Open_Source_Software_Notice)
- You are welcome to contribute to the community via [Issues](https://gitcode.com/Ascend/mspti/issues) and Pull Requests. Before submitting, please complete local tests and ensure that the new capabilities are accompanied by necessary tests.

## 💬 Suggestions and Feedback

You are welcome to contribute to the community. If you have any questions or suggestions, please submit a [Issues](https://gitcode.com/Ascend/mspti/issues). We will reply as soon as possible. Thank you for your support.

## 🤝 Acknowledgments

msPTI is contributed by Huawei Ascend Computing MindStudio Development Dept. Thank you for every PR from the community. We welcome your continuous participation in co-building.

## About the MindStudio Team

The MindStudio team continuously builds toolchain capabilities such as training, inference, and performance analysis based on the Ascend development scenario. For more information, visit the Ascend Community (https://www.hiascend.com/developer/software/mindstudio) and Ascend Forum (https://www.hiascend.com/forum/).
