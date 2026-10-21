# PM Sampling 全指标样例

通过公共 C API 查询目标设备支持的指标，并演示会话配置、采样、结果镜像解码、逐样本读取、停止排空及资源释放。编译和运行需要已安装的 MSPTI 头文件及动态库。

## 调用流程

样例使用以下 14 个 PM Sampling 公共接口，完成指标查询、采样和结果读取。

|阶段|调用的公共接口（统一前缀 msptiPmSampling）|
|--|--|
|创建会话|Enable|
|查询全部指标及设备支持情况|GetNumMetrics、GetMetricProperties、GetCounterAvailability|
|配置并启动采集|SetConfig、Start|
|准备结果镜像|GetCounterDataSize、CounterDataImageInitialize|
|消费和读取样本|DecodeData、GetCounterDataInfo、GetSampleInfo、GetSampleEntries|
|停止、排空及释放会话|Stop、Disable；Stop 后继续 DecodeData 并读取至队列为空|

样例根据 GetCounterAvailability 和 GetMetricProperties 查询结果，将全部可用指标加入 SetConfig。每个样本遍历全部返回条目，HCCS 等多实例指标逐实例打印。指标不支持时打印名称、ID 和单位并跳过。

## 文件说明

|文件|作用|
|--|--|
|pm_sampling.cpp|采样和读取完整流程，展示全部可用指标的所有返回实例。|
|CMakeLists.txt|链接指定安装目录中的 libmspti.so。|
|sample_run.sh|检查安装路径、编译运行样例并保存日志。|

## 环境准备

- Linux 昇腾设备，当前账号具备目标设备的访问权限。
- 已安装匹配设备驱动的 CANN，并安装包含 PM Sampling 公共接口实现的 MSPTI 包。
- C++14 编译器、CMake 3.14 及以上、构建工具，以及 bash、timeout、sha256sum、ldd。
- 同一设备上没有其他 PM Sampling 会话；避免与占用同一采集资源的工具同时运行。

在新终端中加载实际安装环境，再进入样例目录：

```bash
source /实际的CANN安装目录/set_env.sh
cd "${ASCEND_HOME_PATH}/tools/mspti/samples/pm_sampling"
bash sample_run.sh 0 10 100
```

MSPTI 安装在其他位置时，将 MSPTI_ROOT 设置为实际的 `tools/mspti` 目录，其中应包含 `include/mspti_pm_sampling.h` 和 `lib64/libmspti.so`。样例源码也可以单独复制到可读目录运行，库仍来自 MSPTI_ROOT。

参数依次为设备 ID、持续时间（秒，1～60）、采样配置周期（毫秒，1～1000），默认值为 `0 10 100`。实际数据更新速度取决于设备和采集实现，不保证等于配置周期。时间过短导致没有完整样本时，应增加持续时间再验证。

编译产物和日志默认存放在 `${HOME}/.cache/mspti/pm_sampling`，可用 `MSPTI_SAMPLE_BUILD_DIR` 指定其他可写目录。脚本不修改安装目录、不自动安装依赖；每次运行最多等待程序 90 秒，超时返回失败。

## 输出说明

|输出|含义|
|--|--|
|Header / Library / SHA256 / ldd 输出|使用的头文件和动态库路径、文件校验值及动态链接信息，便于排查环境问题。|
|METRIC / SKIP_UNAVAILABLE|设备可用指标的名称和单位，或明确跳过不支持的指标。|
|COVERAGE|查询的指标总数、实际选用数及跳过数。|
|BATCH|本批完整样本数、镜像容量、队列溢出标志及解码停止原因。|
|ENTRY|依次为样本序号、开始/结束时间（ns）、指标 ID、指标名称、实例 ID、数值、单位。|
|SUMMARY|每个指标的条目数、非零条目数和最小/最大值；多实例结果一起统计。|
|FLOW_PASS|取得完整样本、基本结果校验通过、无队列溢出且停止后排空成功。|
|FLOW_FAIL|某接口或结果校验失败；结合之前的错误和采集日志定位。|

程序逐批读完镜像中的所有条目后才进行下一次解码，避免下一批覆盖未读取的结果。线程按顺序调用会话及镜像接口。

所有指标名称和单位均从公共接口查询，不固定指标数量，也不对功耗等数值额外换算。不支持的指标会跳过；所有指标都不可用或未取得样本时返回失败。

`FLOW_PASS` 只表示调用流程及基本结果校验通过，输出同时保留 `DATA_VALIDITY_NOT_VERIFIED` 提示。

程序正常完成时返回 0，接口调用或结果校验失败时返回非零值。运行日志路径在脚本启动时以 `Log:` 打印。
