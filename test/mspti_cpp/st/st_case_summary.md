# test/mspti_cpp/st 用例梳理

| 用例名 | 用例功能 | 校验重点 |
| --- | --- | --- |
| `test_mspti_correlation` | 验证 MSPTI API 与 Activity 的 correlationId 关联机制 | 校验日志中 `API and Activity correlation: correlation: <id>` 记录数，确保 API 与 Activity 正确关联 |
| `test_mspti_activity` | 验证 MSPTI 全量 Activity 采集能力（Kernel/API/Memory/MemCpy） | 校验 KERNEL_AIVEC 记录（kernel 名、时间合法性）、API 记录（aclnn 接口名、时间合法性）、Memory ALLOCATION/RELEASE 各 3 条配对、HTOD MemCpy 3 条 |
| `test_hccl_mspti_correlation` | 验证 HCCL 集合通信 AllReduce Activity 采集能力 | 校验 8 条 HcclAllReduce 记录（含 IP/rank/commName），每条 `end > start` |
| `test_mspti_external_correlation` | 验证外部 correlationId 分组机制 | 校验 8 条 aclnn 结果行，以及三组 External ID（INITIALIZATION/EXECUTION/CLEANUP）到 correlation IDs 的正确映射 |
| `test_mspti_mstx_activity_domain` | 验证 MSTX Marker Activity 的 domain 域标记能力 | 校验 host 侧 domain 非空 marker 2 条、host 侧空 domain marker 2 条、device 侧 marker 2 条 |
| `test_mspti_callback` | 验证基于 Callback 的 MSTX Marker 发射能力 | 校验 2 条 HOST_DATA MARKER 和 2 条 DEVICE_DATA MARKER |
| `test_mspti_callback_domain` | 验证 Callback 方式对 ACL 运行时 API 的 domain 函数追踪 | 校验 7 个 ACL API（aclrtMalloc/Memcpy/LaunchKernelWithHostArgs/SynchronizeStream/Free/DestroyStream/ResetDevice）的 enter/exit 成对出现 |
| `test_python_monitor` | 验证 Python 层 MSPTI Monitor 对 Kernel 和通信 Activity 的捕获 | 校验 8 条 KERNEL_AIVEC、8 条 KERNEL_AICORE、8 条 COMMUNICATION(allReduce)，每条时间合法 |
| `test_python_mstx_monitor` | 验证 Python 层 MSTX Monitor 的 Host/Device Marker 采集 | 校验 8 条 HOST MARKER（mstx_matmul, domain=default）、8 条 DEVICE MARKER、rank_id 在 0~7 均匀分布 |
| `test_MsptiActivityCallbackCase` | 冒烟验证 Activity+Callback C++ 用例编译与运行 | 无结构化校验，依赖基类 plog/stdout 中无 ERROR 日志即判定通过 |
| `test_MsptiMarkMultiThreadCase` | 验证 16 线程并发 Mark 发射（每线程 40000 次）的可靠性 | 校验日志中 MSPTI_SMOKE_MARK_NUM 总数是否为 640000（无标记丢失） |
| `test_MsptiPythonMonitorCase` | 验证分布式 MNIST 训练场景下 Monitor 采集精度 | 校验 CSV 中 MARKER(HcclAllreduce) 7500 条、HCCL 7508 条、KERNEL(Relu) 22512 条，总数精确匹配 |
| `test_mspti_communication` | 验证通信域 profiling 的 API-Communication 关联完整性 | 校验 20 条通信记录（hcom_allReduce_）、API 记录时间合法性、Communication 与 API 按 correlationId 一一关联 |
| `test_mspti_graph` | 验证 Graph 模式 profiling 的 SQLite DB 交付件完整性 | 逐 DB 校验 kernel 与 api 全量关联（无 NULL）、kernel 的 correlationId 分组仅含 1 或 3 行、communication 与 api 的 name 一致 |
