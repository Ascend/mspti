# 版本说明

## 版本列表

| 发布版本 | 发布时间 | 发布Tag | 兼容性说明 |
| -------------- | ---------- | ----------------------------- |----------------------------------------------------------------------|
| 26.0.0-alpha.1 | 2026-02-06 | tag_MindStudio_26.0.0-alpha.1 | 大于 CANN 8.5.0。请参见[CANN安装指南](https://www.hiascend.com/cann)获取CANN安装包。 |

## 26.0.0-alpha.1 版本详情

### 新增特性

- 提供 Activity API 支持采集 Kernel、Memory、Memcpy、HCCL、Marker、Communication 等 12 种活动类型
- 提供 Callback API 支持 Runtime 和 HCCL 域的回调订阅
- 提供 Python API 封装（KernelMonitor / HcclMonitor / MstxMonitor / CommunicationMonitor）
- 支持 External Correlation 跨层调用链路关联
- 支持 MSTX 域级打点动态启停控制

### 安装包 MD5 校验值

| 文件名 | MD5 |
| --- | --- |
| `mindstudio-profiler-tools-interface_26.0.0-alpha.1_x86_64.run` | `17d5a573b01272ee1a282762393a02dd` |
| `mindstudio-profiler-tools-interface_26.0.0-alpha.1_aarch64.run` | `4988310d4cd11679c3c3c440c563f85f` |

### 已知问题

1. msPTI 不可与其他性能数据采集工具同时使用，否则会导致采集的数据丢失。
2. 同一时刻仅支持一个 Callback 订阅者（Subscriber），多订阅者场景暂不支持。
3. Python Monitor 的回调函数中不建议执行耗时操作，否则可能阻塞采集线程。

### 升级说明

- 升级方式："先卸后装"。直接执行新版本安装命令，工具将自动卸载旧版本并引导完成覆盖安装。
- 升级前请通过 `pip show mspti` 查看当前版本，确认版本配套关系。
- 升级后需重新配置 CANN 环境变量。

---

> 本版本说明将持续更新。历史版本的详细记录请参见仓库的 Release 页面。
