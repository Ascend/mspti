<h1 align="center">MindStudio Profiler Tools Interface</h1>

<div align="center">
  <p><b>Ascend Profiling Tools Interface</b></p>

[📖 User Guide](./docs/en/getting_started/samples_guide.md) |
[🛠️ Installation Guide](./docs/en/getting_started/mspti_install_guide.md) |
[📚 API Reference](./docs/en/c_api/README.md) |
[🌐 Software Download](https://gitcode.com/Ascend/mspti/releases)

</div>

<br>

## 📢 What's New

* [2026.02.06]: Added `26.0.0-alpha.1` release record to Release Notes, compatible with CANN `> 8.5.0`. For details, see [Release Notes](./docs/en/release_notes.md).

## 📌 Introduction

msPTI (MindStudio Profiler Tools Interface) is a collection of profiling APIs for Ascend devices, helping developers build performance profiling and analysis tools for NPU applications, suitable for both inference and training.

msPTI provides the following capabilities:

- `Tracing`: Collects timestamps and additional information for CANN APIs, kernels, memory copies, communication, and markers to identify performance bottlenecks in the execution pipeline.
- `Profiling`: Collects NPU performance metrics for a single kernel or a group of kernels to support computation and communication analysis.

## 🔍 Directory Structure

```text
├─docs
│  └─en                  # Documentation, installation guide, release notes, security statement, API Reference
├─csrc                   # C/C++ core implementation
│  ├─activity            # Activity Data Collection and Parsing
│  ├─callback            # Callback Subscription and Callback Management
│  ├─common              # Common Basic Capabilities
│  └─include             # msPTI C API Header Files
├─mspti                  # Python Wrapper
│  ├─monitor             # Kernel / HCCL / MSTX Monitor
│  └─csrc                # Python Extension Binding Implementation
├─samples                # C++ / Python Samples
├─scripts                # Build, Package, Install, and Test Scripts
├─test                   # UT / ST Test Code
├─CMakeLists.txt         # C++ Build Entry
└─README.md              # Repository Overview
```

## 📖 Features

| Module | Feature | Documentation |
| --- | --- | --- |
| `Activity API` | Collects activity data such as API, kernel, memory, hccl, marker, and external correlation for building tracing and profiling tools. | [C API Reference](./docs/en/c_api/README.md) |
| `Callback API` | Subscribes to runtime and HCCL callbacks to execute custom logic or correlate profile data before and after API calls. | [C API Reference](./docs/en/c_api/README.md) |
| `Python API` | Provides APIs such as `KernelMonitor`, `HcclMonitor`, and `MstxMonitor` for quick integration into Python analysis. | [Python API Reference](./docs/en/python_api/README.md) |
| `Samples` | Covers typical scenarios such as callback, activity, correlation, HCCL, and Python monitor for quick onboarding. | [User Guide](./docs/en/getting_started/samples_guide.md) |

## 🛠️ Installation Guide

msPTI depends on a matching version of CANN. Before installing msPTI, set up the environment:

- Hardware environment: See the [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).
- Software environment: Refer to the [CANN Installation](https://www.hiascend.com/en/cann/download) to install the mapped CANN Toolkit and the ops package.

After setting up the environment, you can install msPTI in either of the following two ways:

- Method 1: Download the pre-built `run` package from the [releases page](https://gitcode.com/Ascend/mspti/releases), perform MD5 verification, and then install it.
- Method 2: From the source repository, run `bash scripts/build.sh [<version>]` to build the `run` package first, and then install it.

For complete environment preparation, detailed steps of both installation methods, installation parameters, and example commands, see [msPTI Tool Installation Guide](./docs/en/getting_started/mspti_install_guide.md).

## 🚀 Getting Started

For details about how to use msPTI tools, see [msPTI Quick Start](./docs/en/getting_started/quick_start.md).

## 📝 Important Notes

- [Security Statement](./docs/en/security_statement.md)
- [LICENSE](./LICENSE)
- [Third_Party_Open_Source_Software_Notice](./Third_Party_Open_Source_Software_Notice)
- Contributions are welcome via [Issues](https://gitcode.com/Ascend/mspti/issues) and Pull Requests. Before submitting a PR, please complete local testing and ensure that new features include necessary tests.

## 💬 Suggestions and Communication

You are welcome to contribute to the community. If you have any questions or suggestions, please submit an [Issue](https://gitcode.com/Ascend/mspti/issues), and we will respond as soon as possible. Thank you for your support.

|                                      📱 Follow the MindStudio Official Account                                       | 💬 More Communication and Support                                                                                                                                                                                                                                                                                                                                                                                                                     |
|:-----------------------------------------------------------------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| <img src="https://raw.gitcode.com/Ascend/msot/files/master/docs/zh/figures/readme/officialAccount.png" width="120"><br><sub>*Scan the QR code for the latest updates*</sub> | 💡 **Join the WeChat Group**:<br>Follow the official account and reply "communication group" to get the group QR code.<br><br>🛠️ **Other Channels**:<br>👉 Ascend Assistant: [![WeChat](https://img.shields.io/badge/WeChat-07C160?style=flat-square&logo=wechat&logoColor=white)](https://gitcode.com/Ascend/msot/blob/master/docs/zh/figures/readme/xiaozhushou.png)<br>👉 Ascend Forum: [![Website](https://img.shields.io/badge/Website-%231e37ff?style=flat-square&logo=RSS&logoColor=white)](https://www.hiascend.com/forum/) |

## 🤝 Acknowledgments

msPTI is contributed by Huawei Ascend Computing MindStudio Development Dept. We appreciate every PR from the community and welcome your continued participation in co-building.

## About the MindStudio Team

The MindStudio team continuously builds toolchain capabilities such as training, inference, and performance analysis around Ascend development scenarios. For more information, visit the [Ascend Community](https://www.hiascend.com/developer/software/mindstudio) and the [Ascend Forum](https://www.hiascend.com/forum/).
