<h1 align="center">MindStudio Profiler Tools Interface</h1>

<div align="center">
  <p><b>昇腾 Profiling 工具接口</b></p>

 [![QuickStart](https://badgen.net/badge/快速入门/QuickStart/blue)](./docs/zh/quick_start/quick_start.md)
 [![Ask DeepWiki](https://badgen.net/badge/AI问答/DeepWiki/blue)](https://deepwiki.com/mindstudio-docs/master)
 [![Ask ZRead](https://badgen.net/badge/AI问答/ZRead/blue)](https://zread.ai/mindstudio-docs/master)
 [![ReadTheDocs](https://badgen.net/badge/精确搜索/ReadTheDocs/blue)](https://mindstudio-docs-master.readthedocs.io)
 [![Community](https://badgen.net/badge/昇腾社区/Community/blue)](https://www.hiascend.com/cn/developer/software/mindstudio)
 [![Issues](https://badgen.net/badge/报告问题/Issues/blue)](https://gitcode.com/Ascend/mspti/issues)

</div>

## ✨ 最新消息

* [2026.02.06]：版本说明新增 `26.0.0-alpha.1` 发布记录，兼容 CANN `> 8.5.0`，详情请参见 《[版本说明](./docs/zh/release_notes/release_notes.md)》。

## ℹ️ 简介

msPTI（MindStudio Profiler Tools Interface）是面向 Ascend 设备的 Profiling API 集合，帮助开发者为 NPU 应用构建性能采集与分析工具，适用于推理与训练场景。

msPTI 主要提供以下能力：

- `Tracing`：采集 CANN API、Kernel、内存拷贝、通信、打点等时间戳及附加信息，用于定位执行链路中的性能瓶颈。
- `Profiling`：单独采集一个或一组 Kernel 的 NPU 性能指标，支撑计算与通信分析。

## ⚙️ 功能介绍

| 模块 | 功能简介 | 文档 |
| --- | --- | --- |
| `Activity API` | 采集 API、Kernel、Memory、HCCL、Marker、External Correlation 等活动数据，用于构建 Tracing / Profiling 工具。 | [C API 参考](./docs/zh/api_reference/c_api/README.md) |
| `Callback API` | 订阅 Runtime / HCCL 回调，在 API 调用前后执行自定义逻辑或关联采集数据。 | [C API 参考](./docs/zh/api_reference/c_api/README.md) |
| `Python API` | 提供 `KernelMonitor`、`HcclMonitor`、`MstxMonitor`、`CommunicationMonitor` 等接口，快速接入 Python 场景分析能力。 | [Python API 参考](./docs/zh/api_reference/python_api/README.md) |
| `样例集` | 覆盖 callback、activity、correlation、HCCL、Python monitor 等典型场景，便于快速上手。 | [样例说明](./samples/README.md) / [用户指南](./docs/zh/user_guide/samples_guide.md) |

## 🚀 快速入门

快速入门介绍msPTI工具的使用流程，具体请参见《[msPTI快速入门](./docs/zh/quick_start/quick_start.md)》。

## 📦 安装指南

msPTI 运行依赖配套版本的 CANN 环境。安装 msPTI 前，请先完成以下环境准备：

- 硬件环境请参见《[昇腾产品形态说明](https://www.hiascend.com/document/detail/zh/AscendFAQ/ProduTech/productform/hardwaredesc_0001.html)》。
- 软件环境请参见[CANN快速安装](https://www.hiascend.com/cann/download)安装配套版本的 CANN Toolkit 开发套件包和 ops 算子包。

完成上述准备后，可通过以下两种方式安装 msPTI：

- 方式一：从 [releases 页面](https://gitcode.com/Ascend/mspti/releases) 下载预构建的 `run` 包，执行 MD5 校验后安装。
- 方式二：从源码仓执行 `bash scripts/build.sh [<version>]` 先构建 `run` 包，再安装。

完整环境准备、两种安装方式的详细步骤、安装参数与示例命令请参见 《[msPTI 工具安装指南](./docs/zh/install_guide/mspti_install_guide.md)》。

## 📘 使用指南

工具的详细使用方法，请参见 《[msPTI 使用指南](./docs/zh/user_guide/mspti_user_guide.md)》。

## 💡 典型案例

通过典型问题场景帮助用户理解并掌握工具使用，请参见《[msPTI 典型案例](docs/zh/best_practices/basic_cases.md)》。

## 📚 API 参考

包含 msPTI 提供 C 语言接口和 Python 语言接口两种类型，请参见《[msPTI C API 参考](./docs/zh/api_reference/c_api/README.md)》 和《[msPTI Python API 参考](./docs/zh/api_reference/python_api/README.md)》。

## ❓ FAQ

常见问题及解决方案，请参见 《[msPTI FAQ](./docs/zh/support/faq.md)》

## 🌌 智能检索

为提升文档查阅效率，我们提供多种高效检索方式：

🔹 [AI 问答（DeepWiki）](https://deepwiki.com/mindstudio-docs/master)：自然语言问答，快速把握项目架构与模块关系。<br>
🔹 [AI 问答（ZRead）](https://zread.ai/mindstudio-docs/master)：中文问答体验更优，精准定位功能用法与细节。<br>
🔹 [精确搜索（ReadTheDocs）](https://mindstudio-docs-master.readthedocs.io)：关键词全文检索，直达接口、参数与报错等信息。<br>

## 🛠️ 贡献指南

欢迎参与项目贡献，请参见 [《贡献指南》](./docs/zh/contributing/contributing_guide.md)。

## ⚖️ 相关说明

🔹 [《版本说明》](./docs/zh/release_notes/release_notes.md) <br>
🔹 [《许可证声明》](./LICENSE) <br>
🔹 [《安全声明》](./docs/zh/legal/security_statement.md) <br>
🔹 [《免责声明》](./docs/zh/legal/disclaimer.md) <br>

## 🤝 建议与交流

欢迎大家为社区做贡献。如果有任何疑问或建议，请提交 [Issues](https://gitcode.com/Ascend/mspti/issues)，我们会尽快回复。感谢您的支持。

|                                                                            即时互动（微信群）                                                                             |                                                                                  官方资讯（公众号）                                                                                   | 深度支持（助手/论坛）                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
|:------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:------------------------------------------------------------------------------------------------------------------------------------------------------------------------------:|:--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| <img src="https://raw.gitcode.com/mengguangxin/docs/files/dev_0526/common/Writing_Template/figures/qr_code_wechat_work.png" width="120"><br><sub>*扫码加入技术交流群*</sub> | <img src="https://raw.gitcode.com/mengguangxin/docs/files/dev_0526/common/Writing_Template/figures/qr_code_wechat_official_account.png" width="120"><br><sub>*扫码关注官方公众号*</sub> | 扫码入群并关注公众号，直达 MindStudio 用户与开发者最快捷的交流平台：<br> **快速提问：** 与社区小伙伴即时探讨技术问题<br>**掌握动态：** 第一时间获取版本发布与功能更新通知<br> **经验共享：** 与广大开发者交流最佳实践与实战心得  <br> <br> **更多支持渠道**：👉 昇腾助手：[![WeChat](https://img.shields.io/badge/WeChat-07C160?style=flat-square&logo=wechat&logoColor=white)](https://gitcode.com/Ascend/msit/blob/master/docs/zh/figures/readme/xiaozhushou.png) 👉 昇腾论坛：[![Website](https://img.shields.io/badge/Website-%231e37ff?style=flat-square&logo=RSS&logoColor=white)](https://www.hiascend.com/forum/) |

## 🙏 致谢

本工具由华为公司的下列部门贡献：

🔹 昇腾计算MindStudio开发部

感谢来自社区的每一个PR，欢迎贡献。
