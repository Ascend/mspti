# PM Sampling API 使用指南

## 1. 概述

PM Sampling API 是 msPTI 的设备硬件指标周期采集接口。它以固定时间间隔对目标设备进行点采样，采集 HBM 带宽利用率、HCCS/PCIe/RoCE 通信带宽、功耗、温度、AI Core/Vector/Cube 利用率与频率等硬件指标。

指标的采集能力与芯片型号相关，配置前可通过可用性查询接口过滤。

### 1.1 核心概念

- **PM Sampling Object**：一次 PM 采样会话的不透明句柄，由 `msptiPmSamplingEnable` 创建，`msptiPmSamplingDisable` 销毁。每台设备同一时刻最多存在一个已使能的会话。
- **Metric ID**：静态指标表的指标编号，通过 `msptiPmSamplingMetricId` 枚举标识。
- **Sample（样本）**：一个采样窗口的指标集合，携带起止时间戳（host 纳秒）。
- **Sample Entry（值条目）**：样本的自描述组成单元，每条携带 `{metricId, instanceId, value}`。逐实例指标（如 HCCS 逐链路）为每个实例产出一条条目，因此样本条目数不固定，通常大于等于配置的指标数。
- **Counter Data Image**：对用户不透明的缓冲区，由用户按 `msptiPmSamplingGetCounterDataSize` 返回的大小分配，只能通过本模块接口访问，用于承接解码后的样本。

### 1.2 工作流程

```text
Enable（创建会话对象） → SetConfig（配置指标集/采样间隔/缓冲区）
    → Start（开始采样窗口） → 周期采样产生样本，累积在会话队列中
    → Stop（结束采样窗口） → DecodeData（样本解码进镜像）/ GetSampleEntries（直读队列）
    → Disable（销毁对象，去使能采样）
```

Start/Stop 可反复执行，多个窗口的样本累积在同一会话中；Stop 后可重新 SetConfig 再启动。

---

## 2. 指标集（Metric ID）

| ID | 枚举值 | 说明 | 单位 |
| --- | --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL` | NPU 整体利用率（具体语义随 HDK 版本而定） | % |
| 1 | `MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL` | AI Core 利用率 | % |
| 2 | `MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL` | AI Vector 利用率 | % |
| 3 | `MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL` | AI Cube 利用率 | % |
| 4 | `MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL` | HBM 带宽利用率 | % |
| 5 | `MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL` | HBM 内存利用率（已用容量/总容量） | % |
| 6 | `MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW` | HCCS 发送带宽（逐链路上报） | bytes/s |
| 7 | `MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW` | HCCS 接收带宽（逐链路上报） | bytes/s |
| 8 | `MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW` | PCIe 发送带宽 | bytes/s |
| 9 | `MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW` | PCIe 接收带宽 | bytes/s |
| 10 | `MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW` | RoCE 发送带宽 | bytes/s |
| 11 | `MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW` | RoCE 接收带宽 | bytes/s |
| 12 | `MSPTI_PM_SAMPLING_METRIC_ID_POWER` | 功耗 | W |
| 13 | `MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE` | 温度 | 摄氏度 |
| 14 | `MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ` | AI Core 当前频率 | MHz |

说明：

- HBM 指标均为利用率：只提供 HBM 带宽利用率，不提供读/写字节速率；内存利用率为已用容量与总容量之比，由采集侧归一化。
- HCCS 带宽逐链路上报，`instanceId` 为链路号 0~15，一个样本中 HCCS 指标最多产出 16 条条目。
- HCCS 带宽仅在搭载 HCCS 互联的芯片上支持；采用 UB 互联的芯片（如 950 系列）不提供 HCCS 指标。配置前建议通过 `msptiPmSamplingGetCounterAvailability` 确认目标指标的可用性。
- RoCE 带宽当前版本暂不对外开放。
- 指标无实例概念时（功耗、温度、HBM 带宽利用率等），`instanceId` 固定为 0；某指标本窗口无数据时产出一条 `{metricId, 0, 0.0}`。
- 功耗值以 W 为单位产出。
- 指标的短名、分组、数据类型、单位与可用性可通过 `msptiPmSamplingGetMetricProperties` 查询，用于展示归类。

---

## 3. 核心数据结构

### 3.1 msptiPmSamplingMetricProperties（指标属性）

`msptiPmSamplingGetMetricProperties` 返回的指标静态属性，用于在配置前了解指标的名称、分组、单位与可用性。

```c
typedef struct
{
    char shortName[64];                     // 指标短名，如 "power"、"hccs_tx_bw"
    msptiPmSamplingMetricGroup metricGroup; // 指标分组
    msptiPmSamplingMetricDataType dataType; // 数据类型
    char unit[32];                          // 值的单位字符串，如 "bytes/s"、"%"、"W"
    bool available;                         // 该指标在当前芯片型号上是否可采
} msptiPmSamplingMetricProperties;
```

metricGroup 与 dataType 的取值如下：

| 数值 | 枚举常量 | 说明 |
| --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY` | 内存（HBM 相关指标） |
| 1 | `MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK` | 网络（HCCS/PCIe/RoCE 带宽） |
| 2 | `MSPTI_PM_SAMPLING_METRIC_GROUP_POWER` | 功耗 |
| 3 | `MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL` | 温度 |
| 4 | `MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION` | 利用率 |
| 5 | `MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY` | 频率 |

| 数值 | 枚举常量 | 说明 |
| --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO` | 无量纲比值 |
| 1 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT` | 百分比 |
| 2 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT` | 吞吐（bytes/s） |
| 3 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER` | 功耗（W） |
| 4 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE` | 温度（摄氏度） |
| 5 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY` | 频率（MHz） |

### 3.2 msptiPmSamplingSampleEntry（样本值条目）

样本由值条目列表组成，每条条目自描述其所属的指标与硬件实例。消费方无需了解样本的二进制布局，逐条读取即可。

```c
typedef struct
{
    uint32_t metricId;   // 指标 ID，见 MSPTI_PM_SAMPLING_METRIC_ID_*
    uint32_t instanceId; // 硬件实例 ID，指标无实例概念时为 0（如 HCCS 带宽逐链路 0~15 上报）
    double value;        // 指标值，单位为对应指标的单位
} msptiPmSamplingSampleEntry;
```

- 每个配置的指标按其上报的硬件实例数产出条目：多实例指标（如 HCCS 带宽）每条链路一条条目，无实例指标（功耗、温度、HBM 带宽利用率等）固定 instanceId 为 0。
- 某指标在窗口内无数据时，产出一条 instanceId 为 0、值为 0 的条目。
- `metricId` 的完整取值见第 2 章指标集。

---

## 4. 生命周期与状态

会话对象具有隐式状态机：**Created → Configured → Running → Stopped**。

| 接口 | Created | Configured | Running | Stopped |
| --- | :-: | :-: | :-: | :-: |
| msptiPmSamplingSetConfig | √ | √ | × | √（清空样本队列） |
| msptiPmSamplingStart | × | √ | √（幂等） | √ |
| msptiPmSamplingStop | × | × | √ | × |
| msptiPmSamplingDecodeData | × | × | √ | √ |
| msptiPmSamplingDisable | √ | √ | √（内部先 Stop） | √ |

- SetConfig 在 Stopped 状态重新配置时会清空会话中待取的样本队列，因为指标集可能变化，旧样本与新条目布局不匹配。
- Stop 保留对象、配置与已采集的样本，会话可再次 Start 或解码。
- Disable 在 Running 状态调用时内部先停止采样（冲出最后一个样本窗口）再销毁对象。

---

## 5. API 函数参考

### 5.1 会话生命周期函数

#### 5.1.1 msptiPmSamplingEnable

在指定设备上创建 PM Sampling 对象并使能 PM 采样。每台设备同一时刻最多存在一个已使能的会话，重复使能返回 `MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED`。

```c
msptiResult msptiPmSamplingEnable(msptiPmSamplingEnableParams *params);
```

- `params.deviceId`：目标设备 ID。
- `params.pPmSamplingObject`：[out] 创建的会话对象，后续所有接口均需传入。
- 目标设备离线时返回 `MSPTI_ERROR_DEVICE_OFFLINE`。
- 对象由 `msptiPmSamplingDisable` 销毁。

#### 5.1.2 msptiPmSamplingDisable

销毁 PM Sampling 对象并在设备上去使能 PM 采样。Running 状态调用时内部先停止采样（冲出最后一个样本窗口）再销毁。

```c
msptiResult msptiPmSamplingDisable(msptiPmSamplingDisableParams *params);
```

- `params.pPmSamplingObject`：待销毁的会话对象，销毁后句柄失效。
- 对象销毁时，会话中已采集但未读取的样本一并丢弃。

### 5.2 配置与控制函数

#### 5.2.1 msptiPmSamplingSetConfig

配置指标集、采样间隔、触发模式与硬件缓冲区。可在 Created、Configured、Stopped 状态调用，Running 状态调用返回 `MSPTI_ERROR_INVALID_OPERATION`。

```c
msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params);
```

- `params.numMetrics`/`params.pMetricIds`：指标集。metricId 越界、重复或设备不可采时返回 `MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC`，配置前建议先经 `msptiPmSamplingGetCounterAvailability` 过滤。
- `params.samplingInterval`：采样间隔，单位 ns，范围 [1ms, 1s]。
- `params.triggerMode`：触发模式，当前仅支持 `MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL`。
- `params.hardwareBufferSize`：内部样本队列容量（字节），0 表示默认大小（容纳 10000 个样本）；非 0 时最小需容纳单个样本、最大 512MB，超出范围返回 `MSPTI_ERROR_INVALID_PARAMETER`。队列内单样本开销视指标集而定，容量可按单样本大小与预期在队样本数估算，通常与镜像容量同量级即可。
- `params.hwBufferAppendMode`：队列满时的追加策略，`KEEP_OLDEST` 保留已采集样本、丢弃新样本，`KEEP_LATEST` 覆盖最早样本、保留最新样本。
- Stopped 状态重新配置会清空待取的样本队列，原因见第 4 章。

#### 5.2.2 msptiPmSamplingStart / msptiPmSamplingStop

开启/结束采样窗口。Running 状态重复 Start 幂等；Stop 保留对象、配置与已采集的样本，会话可再次 Start 或解码。Start/Stop 可反复执行，多个窗口的样本累积在同一会话中。

```c
msptiResult msptiPmSamplingStart(msptiPmSamplingStartParams *params);
msptiResult msptiPmSamplingStop(msptiPmSamplingStopParams *params);
```

- `params.pPmSamplingObject`：会话对象。
- 状态不满足时（如未配置即 Start、非 Running 状态 Stop）返回 `MSPTI_ERROR_INVALID_OPERATION`。

### 5.3 指标查询函数

#### 5.3.1 msptiPmSamplingGetNumMetrics

获取可配置的指标总数，即静态指标表的大小。

```c
msptiResult msptiPmSamplingGetNumMetrics(msptiPmSamplingGetNumMetricsParams *params);
```

- `params.deviceId`：目标设备 ID。
- `params.pNumMetrics`：[out] 指标总数。

#### 5.3.2 msptiPmSamplingGetMetricProperties

按 metricId 查询指标的静态属性（短名、分组、数据类型、单位、可用性），用于配置前的展示与归类，结构体见 3.1 节。

```c
msptiResult msptiPmSamplingGetMetricProperties(msptiPmSamplingGetMetricPropertiesParams *params);
```

- `params.deviceId`：目标设备 ID。
- `params.metricId`：指标编号。
- `params.pProperties`：[out] 指标属性。

#### 5.3.3 msptiPmSamplingGetCounterAvailability

批量查询指标在目标设备上的可用性。SetConfig 会直接拒绝设备不可采的指标，配置前应先经本接口过滤。

```c
msptiResult msptiPmSamplingGetCounterAvailability(msptiPmSamplingGetCounterAvailabilityParams *params);
```

- `params.deviceId`：目标设备 ID。
- `params.pMetricIds`/`params.numMetrics`：待查询的指标列表。
- `params.pAvailability`：[out] 与指标列表等长的可用性数组，0 不可采、1 可采。

### 5.4 数据读取函数

样本读取有两条路径，可按需选择或并用：

- **counter data image 路径**：`GetCounterDataSize` → 分配内存 → `CounterDataImageInitialize` → `DecodeData` 将样本解码进镜像，再按索引读取。适合批量导出、落盘场景。
- **样本队列直读路径**：读取接口的 `pCounterDataImage` 传 NULL，直接查询内部样本队列。适合实时消费场景。

样本经过内部样本队列与镜像两级缓冲，两级均具备溢出感知手段，见 `msptiPmSamplingDecodeData` 与 `msptiPmSamplingGetCounterDataInfo` 的说明。

#### 5.4.1 msptiPmSamplingGetCounterDataSize

计算镜像所需字节数。单样本条目容量由指标集在接口内部推导，用户只需通过 `maxSamples` 指定镜像可容纳的样本数。

容量估算与建议：单个值条目固定 16 字节（metricId + instanceId + 8 字节值），逐实例指标按实例数展开，单样本大小由指标集决定，可按"样本头 + 条目数 × 16 字节"估算，也可用本接口返回的字节数除以 `maxSamples` 得到单样本均摊大小。`maxSamples` 常规配置 10000 即可（与样本队列默认容量一致）；上限受镜像总大小 512MB 预算约束：maxSamples ≤ (512MB - 镜像目录区大小) / 单样本大小，超出上限返回 `MSPTI_ERROR_INVALID_PARAMETER`，需减小后重试。若解码间隔更长，按单个解码周期内的产出样本数相应放大。

```c
msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params);
```

- `params.numMetrics`/`params.pMetricIds`：镜像承载的指标集，传 0/NULL 时复用 SetConfig 配置的指标集。
- `params.maxSamples`：镜像可容纳的最大样本数。
- `params.pCounterDataSize`：[out] 所需镜像大小（字节）。

#### 5.4.2 msptiPmSamplingCounterDataImageInitialize

初始化用户分配的镜像内存。镜像大小必须与 `msptiPmSamplingGetCounterDataSize` 的输出一致，内存需 16 字节对齐。

```c
msptiResult msptiPmSamplingCounterDataImageInitialize(msptiPmSamplingCounterDataImageInitializeParams *params);
```

- `params.pCounterDataImage`：用户分配的镜像内存。
- `params.counterDataImageSize`：镜像大小（字节），与 GetCounterDataSize 输出不一致时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

#### 5.4.3 msptiPmSamplingGetCounterDataInfo

查询镜像或内部样本队列的样本统计信息。`pCounterDataImage` 为 NULL 时查询内部样本队列。

```c
msptiResult msptiPmSamplingGetCounterDataInfo(msptiPmSamplingGetCounterDataInfoParams *params);
```

- `params.pNumTotalSamples`：[out] 镜像样本槽位总数（镜像路径）。
- `params.pNumPopulatedSamples`：[out] 已填充样本数；直读路径下即当前队列深度，周期性轮询可在溢出发生前加快消费节奏。
- `params.pNumCompletedSamples`：[out] 已完成样本数。
- `params.pCounterDataImage`：[in] 可选，传 NULL 查询内部样本队列。

#### 5.4.4 msptiPmSamplingDecodeData

将内部样本队列中的样本解码写入 counter data image。解码采用消费式语义：写入的样本从队列移除，可增量调用，不会重复解码同一批样本。

```c
msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params);
```

- `params.pCounterDataImage`/`params.counterDataImageSize`：已初始化的镜像及其大小（字节）。
- `params.pOverflow`：[out] 自上次解码以来队列是否因溢出丢弃过样本，读取后自动清除。
- `params.pDecodeStopReason`：[out] 解码停止原因，镜像写满为 `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL`，队列取空为 `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS`。
- 镜像容量不足时返回 `MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW`，此时队列中的样本不丢失，调整容量后重新解码即可。

#### 5.4.5 msptiPmSamplingGetSampleInfo

按索引获取样本的起止时间戳（host 纳秒），用于与 Activity 记录的时间轴对齐。

```c
msptiResult msptiPmSamplingGetSampleInfo(msptiPmSamplingGetSampleInfoParams *params);
```

- `params.sampleIndex`：样本索引。
- `params.pStartTimestamp`/`params.pEndTimestamp`：[out] 样本起止时间戳。
- `params.pCounterDataImage`：[in] 可选，传 NULL 查询内部样本队列，否则从指定镜像读取。

#### 5.4.6 msptiPmSamplingGetSampleEntries

按索引获取样本的值条目列表。条目数随指标实例数变化，采用两段式取数：`pEntries` 传 NULL 仅回填条目数，分配缓冲后再取条目。

```c
msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params);
```

- `params.sampleIndex`：样本索引。
- `params.pEntries`：[out] 值条目数组，传 NULL 仅查询条目数。
- `params.maxEntries`：`pEntries` 的容量，`pEntries` 为 NULL 时忽略。
- `params.pNumEntries`：[out] 样本实际条目数。
- `params.pCounterDataImage`：[in] 可选，传 NULL 查询内部样本队列。

---

## 6. 完整使用示例

### 6.1 样本队列直读路径

以下示例展示典型流程：使能会话、配置指标、启动采样、Stop 后直读样本队列打印样本。

```cpp
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "mspti.h"

int main() {
    const uint32_t deviceId = 0;
    // 指标短名表，下标即 metricId，通过 msptiPmSamplingGetMetricProperties 查询填充
    std::vector<std::string> metricNames(MSPTI_PM_SAMPLING_METRIC_ID_COUNT);

    // 1. 创建会话对象
    msptiPmSamplingEnableParams enableParams = {};
    enableParams.structSize = sizeof(enableParams);
    enableParams.deviceId = deviceId;
    msptiResult ret = msptiPmSamplingEnable(&enableParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Enable failed: %d\n", ret);
        return 1;
    }

    // 2. 查询指标可用性过滤不支持的指标（可选），并查询指标短名用于打印
    uint32_t allMetrics[MSPTI_PM_SAMPLING_METRIC_ID_COUNT] = {
        MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW,
        MSPTI_PM_SAMPLING_METRIC_ID_POWER,
        MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE,
        MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ,
    };
    uint8_t avail[MSPTI_PM_SAMPLING_METRIC_ID_COUNT] = {};
    msptiPmSamplingGetCounterAvailabilityParams availParams = {};
    availParams.structSize = sizeof(availParams);
    availParams.deviceId = deviceId;
    availParams.numMetrics = MSPTI_PM_SAMPLING_METRIC_ID_COUNT;
    availParams.pMetricIds = allMetrics;
    availParams.pAvailability = avail;
    ret = msptiPmSamplingGetCounterAvailability(&availParams);
    if (ret != MSPTI_SUCCESS) {
        printf("GetCounterAvailability failed: %d\n", ret);
        return 1;
    }
    std::vector<uint32_t> metrics;
    for (uint32_t i = 0; i < MSPTI_PM_SAMPLING_METRIC_ID_COUNT; i++) {
        msptiPmSamplingMetricProperties props = {};
        msptiPmSamplingGetMetricPropertiesParams propsParams = {};
        propsParams.structSize = sizeof(propsParams);
        propsParams.deviceId = deviceId;
        propsParams.metricId = i;
        propsParams.pProperties = &props;
        if (msptiPmSamplingGetMetricProperties(&propsParams) == MSPTI_SUCCESS) {
            metricNames[i] = props.shortName;
        }
        if (avail[i]) {
            metrics.push_back(allMetrics[i]);
        }
    }

    // 3. 配置：指标集 + 100ms 采样间隔
    msptiPmSamplingSetConfigParams configParams = {};
    configParams.structSize = sizeof(configParams);
    configParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    configParams.numMetrics = static_cast<uint32_t>(metrics.size());
    configParams.pMetricIds = metrics.data();
    configParams.samplingInterval = 100 * 1000 * 1000;  // 100ms，单位 ns
    configParams.triggerMode = MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL;
    configParams.hardwareBufferSize = 0;  // 使用默认大小
    configParams.hwBufferAppendMode = MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST;
    ret = msptiPmSamplingSetConfig(&configParams);
    if (ret != MSPTI_SUCCESS) {
        printf("SetConfig failed: %d\n", ret);
        return 1;
    }

    // 4. 开始采样，业务运行期间样本持续累积
    msptiPmSamplingStartParams startParams = {};
    startParams.structSize = sizeof(startParams);
    startParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    ret = msptiPmSamplingStart(&startParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Start failed: %d\n", ret);
        return 1;
    }

    // DoYourWork();  // 执行业务代码（此处执行 ACL 计算等操作）；此间为采样窗口，若无耗时操作，窗口近乎为零，可能采不到样本

    // 5. 结束采样窗口
    msptiPmSamplingStopParams stopParams = {};
    stopParams.structSize = sizeof(stopParams);
    stopParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    ret = msptiPmSamplingStop(&stopParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Stop failed: %d\n", ret);
        return 1;
    }

    // 6. 直读内部样本队列：pCounterDataImage 传 NULL
    msptiPmSamplingGetCounterDataInfoParams infoParams = {};
    infoParams.structSize = sizeof(infoParams);
    infoParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    uint64_t numTotalSamples = 0;
    uint64_t numPopulatedSamples = 0;
    uint64_t numSamples = 0;
    infoParams.pNumTotalSamples = &numTotalSamples;
    infoParams.pNumPopulatedSamples = &numPopulatedSamples;
    infoParams.pNumCompletedSamples = &numSamples;
    ret = msptiPmSamplingGetCounterDataInfo(&infoParams);
    if (ret != MSPTI_SUCCESS) {
        printf("GetCounterDataInfo failed: %d\n", ret);
        return 1;
    }

    // 7. 逐样本取时间窗口与值条目并打印
    for (uint64_t i = 0; i < numSamples; i++) {
        uint64_t startNs = 0, endNs = 0;
        msptiPmSamplingGetSampleInfoParams sampleInfoParams = {};
        sampleInfoParams.structSize = sizeof(sampleInfoParams);
        sampleInfoParams.pPmSamplingObject = enableParams.pPmSamplingObject;
        sampleInfoParams.sampleIndex = i;
        sampleInfoParams.pStartTimestamp = &startNs;
        sampleInfoParams.pEndTimestamp = &endNs;
        ret = msptiPmSamplingGetSampleInfo(&sampleInfoParams);
        if (ret != MSPTI_SUCCESS) {
            continue;
        }

        // 两段式取数：先查条目数，再取条目
        msptiPmSamplingGetSampleEntriesParams entryParams = {};
        entryParams.structSize = sizeof(entryParams);
        entryParams.pPmSamplingObject = enableParams.pPmSamplingObject;
        entryParams.sampleIndex = i;
        entryParams.pEntries = nullptr;
        uint32_t numEntries = 0;
        entryParams.pNumEntries = &numEntries;
        ret = msptiPmSamplingGetSampleEntries(&entryParams);
        if (ret != MSPTI_SUCCESS) {
            continue;
        }
        std::vector<msptiPmSamplingSampleEntry> entries(numEntries);
        entryParams.maxEntries = numEntries;
        entryParams.pEntries = entries.data();
        ret = msptiPmSamplingGetSampleEntries(&entryParams);
        if (ret != MSPTI_SUCCESS) {
            continue;
        }

        printf("Sample %llu: [%llu, %llu] ns\n",
               (unsigned long long)i, (unsigned long long)startNs, (unsigned long long)endNs);
        for (uint32_t j = 0; j < numEntries; j++) {
            const msptiPmSamplingSampleEntry &entry = entries[j];
            if (entry.metricId < MSPTI_PM_SAMPLING_METRIC_ID_COUNT) {
                printf("  %s[%u] = %.2f\n", metricNames[entry.metricId].c_str(),
                       entry.instanceId, entry.value);
            }
        }
    }

    // 8. 销毁对象，去使能采样
    msptiPmSamplingDisableParams disableParams = {};
    disableParams.structSize = sizeof(disableParams);
    disableParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    ret = msptiPmSamplingDisable(&disableParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Disable failed: %d\n", ret);
        return 1;
    }

    return 0;
}
```

### 6.2 counter data image 路径

若需要将样本批量导出或落盘，可使用镜像路径替代第 6~7 步的直读：

```cpp
// 1. 计算镜像大小：单样本条目容量由指标集在接口内部推导，
//    用户只需通过 maxSamples 指定镜像可容纳的样本数
uint64_t maxSamples = 1024;
size_t imageSize = 0;
msptiPmSamplingGetCounterDataSizeParams sizeParams = {};
sizeParams.structSize = sizeof(sizeParams);
sizeParams.pPmSamplingObject = enableParams.pPmSamplingObject;
sizeParams.numMetrics = configParams.numMetrics;
sizeParams.pMetricIds = configParams.pMetricIds;
sizeParams.maxSamples = maxSamples;
sizeParams.pCounterDataSize = &imageSize;
msptiPmSamplingGetCounterDataSize(&sizeParams);

// 2. 分配并初始化镜像（16 字节对齐）
std::vector<uint8_t> image(imageSize);
msptiPmSamplingCounterDataImageInitializeParams initParams = {};
initParams.structSize = sizeof(initParams);
initParams.pPmSamplingObject = enableParams.pPmSamplingObject;
initParams.counterDataImageSize = imageSize;
initParams.pCounterDataImage = image.data();
msptiPmSamplingCounterDataImageInitialize(&initParams);

// 3. 解码（写入的样本从内部样本队列移除，可周期性调用做增量解码；
//    队列解码完毕后再次调用时 pDecodeStopReason 为 END_OF_RECORDS）
bool overflow = false;
msptiPmSamplingDecodeStopReason stopReason = MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER;
msptiPmSamplingDecodeDataParams decodeParams = {};
decodeParams.structSize = sizeof(decodeParams);
decodeParams.pPmSamplingObject = enableParams.pPmSamplingObject;
decodeParams.pCounterDataImage = image.data();
decodeParams.counterDataImageSize = imageSize;
decodeParams.pOverflow = &overflow;
decodeParams.pDecodeStopReason = &stopReason;
msptiPmSamplingDecodeData(&decodeParams);

// 4. 读取时将 pCounterDataImage 指向镜像即可：
//    GetSampleInfo/GetSampleEntries 的 pCounterDataImage 字段填 image.data()
```

---

## 7. 样例参考

请参见 `samples/` 目录下的以下样例：

| 样例 | 说明 |
| --- | --- |
| `samples/pm_sampling/` | PM Sampling API 基础用法：会话使能、指标配置、启动采样与样本读取 |

详细说明请参见《[msPTI 样例说明](../../../samples/README.md)》。
