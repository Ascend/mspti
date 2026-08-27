<h1 align="center">MindStudio Profiler Tools Interface</h1>

<div align="center">
  <p><b>Ascend Profiling Tools Interface</b></p>

 [![Quick Start](https://badgen.net/badge/Quick%20Start/QuickStart/blue)](./docs/en/quick_start/mspti_quick_start.md)
 [![AI Q&A DeepWiki](https://badgen.net/badge/AI%20Q%26A/DeepWiki/blue)](https://deepwiki.com/mindstudio-docs/master)
 [![AI Q&A ZRead](https://badgen.net/badge/AI%20Q%26A/ZRead/blue)](https://zread.ai/mindstudio-docs/master)
 [![Exact Search](https://badgen.net/badge/Exact%20Search/ReadTheDocs/blue)](https://mindstudio-docs-master.readthedocs.io)
 [![Ascend Community](https://badgen.net/badge/Ascend%20Community/Community/blue)](https://www.hiascend.com/en/developer/software/mindstudio)
 [![Report an Issue](https://badgen.net/badge/Report%20an%20Issue/Issues/blue)](https://gitcode.com/Ascend/mspti/issues)

</div>

English | [简体中文](./README.md)

## ✨ What's New

🔹 [Feb 6, 2026]: Added the `26.0.0-alpha.1` release record to the Release Notes, compatible with CANN 8.5.0 or later. For details, see [Release Notes](https://gitcode.com/Ascend/mspti/releases).

## ℹ️ Introduction

msPTI (MindStudio Profiler Tools Interface) is a collection of profiling APIs for Ascend devices, helping developers build performance profiling and analysis tools for NPU applications, suitable for both inference and training.

msPTI provides the following capabilities:

- `Tracing`: Collects timestamps and additional information for CANN APIs, kernels, memory copies, communication, and markers to identify performance bottlenecks in the execution pipeline.
- `Profiling`: Collects NPU performance metrics for a single kernel or a group of kernels to support computation and communication analysis.

## ⚙️ Features

| Module | Feature | Documentation |
| --- | --- | --- |
| Activity API | Collects activity data such as API, kernel, memory, HCCL, marker, and external correlation for building tracing and profiling tools. | [C API Reference](./docs/en/api_reference/c_api/README.md) |
| Callback API | Subscribes to runtime and HCCL callbacks to execute custom logic or correlate profile data before and after API calls. | [C API Reference](./docs/en/api_reference/c_api/README.md) |
| Python API | Provides APIs such as `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, and `CommunicationMonitor` for quick integration into Python analysis. | [Python API Reference](./docs/en/api_reference/python_api/README.md) |
| Samples | Covers typical scenarios such as callback, activity, correlation, HCCL, and Python monitor for quick onboarding. | [Sample Description](./samples/README.md)/[Sample Guide](./docs/en/user_guide/samples_guide.md) |

## 🚀 Getting Started

For details about how to use msPTI tools, see [msPTI Quick Start](./docs/en/quick_start/mspti_quick_start.md).

## 📦 Installation Guide

msPTI depends on a matching version of CANN. Before installing msPTI, set up the environment:

- Hardware environment: See the [Ascend Product Overview](https://www.hiascend.com/document/detail/en/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html).
- Software environment: See [CANN Installation](https://www.hiascend.com/cann/download) to install the matching CANN Toolkit and the ops package.

After setting up the environment, you can install msPTI in either of the following two ways:

- Method 1: Download the pre-built `run` package from the [releases page](https://gitcode.com/Ascend/mspti/releases), perform MD5 verification, and then install it.
- Method 2: From the source repository, run `bash scripts/build.sh [<version>]` to build the `run` package first, and then install it.

For complete environment preparation, detailed steps of both installation methods, installation parameters, and example commands, see [msPTI Tool Installation Guide](./docs/en/install_guide/mspti_install_guide.md).

## 📘 User Guide

For detailed instructions on using the tools, see [msPTI User Guide](./docs/en/user_guide/mspti_user_guide.md).

## 💡 Typical Use Cases

Typical problem scenarios help you understand and master the tools. See [msPTI Typical Use Cases](docs/en/best_practices/basic_cases.md).

## 📚 API Reference

The API reference covers two types of interfaces: C APIs and Python APIs. See [C API Reference](./docs/en/api_reference/c_api/README.md) and [Python API Reference](./docs/en/api_reference/python_api/README.md).

## 🌌 Smart Search

To improve documentation search efficiency, we provide multiple efficient search methods:

🔹 [AI Q&A (DeepWiki)](https://deepwiki.com/mindstudio-docs/master): Natural language Q&A to quickly grasp the project architecture and module relationships<br>
🔹 [AI Q&A (ZRead)](https://zread.ai/mindstudio-docs/master): Better Chinese Q&A experience for precisely locating feature usage and details<br>
🔹 [Precise Search (ReadTheDocs)](https://mindstudio-docs-master.readthedocs.io): Full-text keyword search that takes you directly to APIs, parameters, error messages, and more<br>

## 🛠️ Contribution Guide

You are welcome to contribute to the project. See [Contribution Guide](./docs/en/contributing/contributing_guide.md).

## 📝 Important Notes

🔹 [Release Notes](https://gitcode.com/Ascend/mspti/releases) <br>
🔹 [License Notice](docs/en/legal/license_notice.md) <br>
🔹 [Security Statement](./docs/en/legal/security_statement.md) <br>
🔹 [Disclaimer](./docs/en/legal/disclaimer.md) <br>

## 🤝 Suggestions and Communication

You are welcome to contribute to the community. If you have any questions or suggestions, please submit an [Issue](https://gitcode.com/Ascend/mspti/issues), and we will respond as soon as possible. Thank you for your support.

| 💬 Instant Interaction (WeChat Group) | 📢 Official Updates (Official Account) | In-Depth Support (Assistant/Forum) |
| :---: | :---: | :--- |
| <img src="./docs/zh/figures/qr_code_wechat_work.png" width="120"><br><sub>*Scan the QR code to join the technical exchange group directly*</sub> | <img src="./docs/zh/figures/qr_code_wechat_official_account.png" width="120"><br><sub>*Scan the QR code for the latest updates*</sub> |Scan the QR code to join the group and follow the official account, the fastest communication channel for MindStudio users and developers:<br> **Quick Q&A:** Discuss technical issues with community members in real time.<br>**Latest Updates:** Get notified of version releases and feature updates as soon as possible.<br> **Experience Sharing:** Exchange best practices and hands-on experience with developers.<br>🛠️ **More Support Channels**: 👉 Ascend Assistant: [![WeChat](https://img.shields.io/badge/WeChat-07C160?style=flat-square&logo=wechat&logoColor=white)](https://gitcode.com/Ascend/msit/blob/master/docs/zh/figures/readme/xiaozhushou.png)👉 Ascend Forum: [![Website](https://img.shields.io/badge/Website-%231e37ff?style=flat-square&logo=RSS&logoColor=white)](https://www.hiascend.com/forum/) |

## 🙏 Acknowledgments

This tool is contributed by the following departments of Huawei:

🔹 Ascend Computing MindStudio Development Dept.

We appreciate every PR from the community and welcome your contributions.
