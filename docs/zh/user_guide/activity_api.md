# Activity API 使用指南

## 1. 概述

Activity API 是 msPTI 的核心数据采集接口。它通过异步缓冲区机制采集 CANN 应用运行过程中的各类活动（Activity）记录，包括 Kernel 执行、API 调用、内存操作、通信数据、用户自定义打点等。

### 1.1 核心概念

- **Activity Record**：NPU 活动的性能记录，每种活动类型对应一个 C 结构体。
- **Activity Buffer**：用于缓存 Activity Record 的内存缓冲区，由用户提供，msPTI 填充。
- **Activity Kind**：活动类型枚举，通过 `msptiActivityKind` 标识。
- **Buffer Callback**：用户注册的缓冲区请求与完成回调，msPTI 通过回调与用户交互。

### 1.2 工作流程

```text
用户注册回调 → msPTI通过RequestFunc申请空缓冲区 → 数据采集填充缓冲区
    → msPTI通过CompleteFunc返回满缓冲区 → 用户遍历解析记录
    → 用户通过RequestFunc归还空缓冲区
```

---

## 2. 活动类型（Activity Kind）

| 枚举值 | Kind 常量 | 说明 | 对应数据结构 |
| --- | --- | --- | --- |
| 1 | `MSPTI_ACTIVITY_KIND_MARKER` | 用户自定义打点 | `msptiActivityMarker` |
| 2 | `MSPTI_ACTIVITY_KIND_KERNEL` | NPU Kernel 执行 | `msptiActivityKernel` |
| 3 | `MSPTI_ACTIVITY_KIND_API` | CANN API 调用 | `msptiActivityApi` |
| 4 | `MSPTI_ACTIVITY_KIND_HCCL` | HCCL 通信操作 | `msptiActivityHccl` |
| 5 | `MSPTI_ACTIVITY_KIND_MEMORY` | 内存分配/释放 | `msptiActivityMemory` |
| 6 | `MSPTI_ACTIVITY_KIND_MEMSET` | 内存设置操作 | `msptiActivityMemset` |
| 7 | `MSPTI_ACTIVITY_KIND_MEMCPY` | 内存拷贝操作 | `msptiActivityMemcpy` |
| 8 | `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` | 外部关联 ID | `msptiActivityExternalCorrelation` |
| 9 | `MSPTI_ACTIVITY_KIND_COMMUNICATION` | 通信算子数据 | `msptiActivityCommunication` |
| 10 | `MSPTI_ACTIVITY_KIND_ACL_API` | ACL 级 API 调用 | — |
| 11 | `MSPTI_ACTIVITY_KIND_NODE_API` | Node 级 API 调用 | — |
| 12 | `MSPTI_ACTIVITY_KIND_RUNTIME_API` | Runtime 级 API 调用 | — |

---

## 3. 核心数据结构

### 3.1 Activity Kernel（Kernel 执行记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_KERNEL
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    struct { uint32_t deviceId; uint32_t streamId; } ds; // 设备与流 ID
    uint64_t correlationId;              // 关联 ID
    const char *type;                    // Kernel 类型
    const char *name;                    // Kernel 名称
} msptiActivityKernel;
```

### 3.2 Activity API（API 调用记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_API
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    struct { uint32_t processId; uint32_t threadId; } pt; // 进程与线程 ID
    uint64_t correlationId;              // 关联 ID
    const char *name;                    // API 名称
} msptiActivityApi;
```

### 3.3 Activity Memory（内存操作记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType; // 操作类型（分配/释放）
    msptiActivityMemoryKind memoryKind;   // 内存类型
    uint64_t correlationId;              // 关联 ID
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint64_t address;                    // 内存地址
    uint64_t bytes;                      // 字节数
    uint32_t processId;                  // 进程 ID
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID
} msptiActivityMemory;
```

### 3.4 Activity Memcpy（内存拷贝记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind;    // 拷贝类型（HTOD / DTOH / DTOD 等）
    uint64_t bytes;                      // 拷贝字节数
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID
    uint64_t correlationId;              // 关联 ID
    uint8_t isAsync;                     // 是否异步
} msptiActivityMemcpy;
```

### 3.5 Activity Memset（内存设置记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMSET
    uint32_t value;                      // 设置的值
    uint64_t bytes;                      // 设置的字节数
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID
    uint64_t correlationId;              // 关联 ID
    uint8_t isAsync;                     // 是否异步
} msptiActivityMemset;
```

### 3.6 Activity HCCL（通信操作记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_HCCL
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    struct { uint32_t deviceId; uint32_t streamId; } ds; // 设备与流 ID
    double bandWidth;                    // 带宽（GB/s）
    const char *name;                    // 通信算子名
    const char *commName;                // 通信组名
} msptiActivityHccl;
```

### 3.7 Activity Communication（通信算子记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_COMMUNICATION
    msptiCommunicationDataType dataType; // 数据类型
    uint64_t count;                      // 数据计数
    struct { uint32_t deviceId; uint32_t streamId; } ds; // 设备与流 ID
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    const char *algType;                 // 算法类型
    const char *name;                    // 算子名
    const char *commName;                // 通信组名
    uint64_t correlationId;              // 关联 ID
} msptiActivityCommunication;
```

### 3.8 Activity Marker（用户打点记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MARKER
    msptiActivityFlag flag;              // 打点标志（瞬时/开始/结束）
    msptiActivitySourceKind sourceKind;  // 数据来源（Host/Device）
    uint64_t timestamp;                  // 时间戳（ns）
    uint64_t id;                         // 标记 ID
    msptiObjectId objectId;              // 对象标识
    const char *name;                    // 标记名称
    const char *domain;                  // 所属域名称
} msptiActivityMarker;
```

### 3.9 Activity External Correlation（外部关联记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
    msptiExternalCorrelationKind externalKind; // 外部 API 类型
    uint64_t externalId;                 // 外部 ID
    uint64_t correlationId;              // 关联 ID
} msptiActivityExternalCorrelation;
```

---

## 4. API 函数参考

### 4.1 生命周期函数

#### 4.1.1 msptiActivityRegisterCallbacks

注册 Activity Buffer 的回调函数。必须在使能任何 Activity Kind 之前调用。

```c
msptiResult msptiActivityRegisterCallbacks(
    msptiBuffersCallbackRequestFunc funcBufferRequested,
    msptiBuffersCallbackCompleteFunc funcBufferCompleted);
```

- `funcBufferRequested`：msPTI 请求空缓冲区时的回调。可以在此回调中分配内存或将消费完的缓冲区归还。
- `funcBufferCompleted`：msPTI 返回填满数据的缓冲区时的回调。在此回调中解析数据。
- 任一参数为 NULL 则返回 `MSPTI_ERROR_INVALID_PARAMETER`。

**回调函数签名：**

```c
// 请求缓冲区回调
typedef void(*msptiBuffersCallbackRequestFunc)(
    uint8_t **buffer,    // [out] 返回缓冲区指针，若设为 NULL 则表示拒绝请求
    size_t *size,        // [out] 缓冲区大小
    size_t *maxNumRecords); // [out] 最大记录数（0 表示尽量填充）

// 缓冲区完成回调
typedef void(*msptiBuffersCallbackCompleteFunc)(
    uint8_t *buffer,     // [in] 包含 Activity 记录的缓冲区
    size_t size,         // [in] 缓冲区总大小
    size_t validSize);   // [in] 有效数据字节数
```

#### 4.1.2 msptiActivityEnable / msptiActivityDisable

使能或禁用指定类型的 Activity 数据采集。可多次调用使能多种类型。默认所有类型均为关闭状态。

```c
msptiResult msptiActivityEnable(msptiActivityKind kind);
msptiResult msptiActivityDisable(msptiActivityKind kind);
```

#### 4.1.3 msptiActivityIsEnabled

查询指定类型的 Activity 采集是否已使能。

```c
bool msptiActivityIsEnabled(msptiActivityKind kind);
```

### 4.2 数据读取函数

#### 4.2.1 msptiActivityGetNextRecord

遍历缓冲区中的 Activity Record。首次调用时 `record` 传入 NULL，之后传入上一次返回的指针。

```c
msptiResult msptiActivityGetNextRecord(
    uint8_t *buffer,                // [in] 缓冲区
    size_t validBufferSizeBytes,    // [in] 有效字节数
    msptiActivity **record);        // [in/out] 记录指针
```

- 返回 `MSPTI_SUCCESS` 表示成功获取一条记录。
- 返回 `MSPTI_ERROR_MAX_LIMIT_REACHED` 表示缓冲区已无更多记录。
- 通过 `record->kind` 判断活动类型，再强转为对应的结构体类型。

### 4.3 缓冲刷新函数

#### 4.3.1 msptiActivityFlushAll

强制刷新所有 Activity 缓冲区，通过 CompleteFunc 回调返回数据。即使缓冲区未满也会返回。

```c
msptiResult msptiActivityFlushAll(uint32_t flag);  // flag 保留参数
```

#### 4.3.2 msptiActivityFlushPeriod

设置周期性 Flush 的时间间隔（毫秒）。设置为 0 可关闭周期刷新。

```c
msptiResult msptiActivityFlushPeriod(uint32_t time);  // time 单位为 ms
```

### 4.4 域标记控制函数

#### 4.4.1 msptiActivityEnableMarkerDomain / msptiActivityDisableMarkerDomain

按名称动态启停指定域的 Marker 打点采集。默认所有域均为开启状态。

```c
msptiResult msptiActivityEnableMarkerDomain(const char* name);
msptiResult msptiActivityDisableMarkerDomain(const char* name);
```

### 4.5 外部关联函数

#### 4.5.1 msptiActivityPushExternalCorrelationId

为调用线程推送一个外部关联 ID，标记进入外部 API 区域。该区域内生成的 API Activity Record 会先产生一条 `EXTERNAL_CORRELATION` 记录。

```c
msptiResult msptiActivityPushExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // 外部 API 类型
    uint64_t id);                       // 外部关联 ID
```

#### 4.5.2 msptiActivityPopExternalCorrelationId

为调用线程弹出外部关联 ID，标记离开外部 API 区域。

```c
msptiResult msptiActivityPopExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // 外部 API 类型
    uint64_t *lastId);                  // [out] 返回最后一个 ID
```

---

## 5. 完整使用示例

### 5.1 基础数据采集

以下示例展示如何使用 Activity API 采集 Kernel、API、Memcpy、Memory 等活动数据：

```cpp
#include <cstdio>
#include <cstdlib>
#include "mspti.h"

// 缓冲区请求回调
void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
    *size = 8 * 1024 * 1024;  // 8MB
    *buffer = (uint8_t*)malloc(*size);
    *maxNumRecords = 0;  // 尽量填充
}

// 打印 Activity 记录
void PrintActivity(msptiActivity *record) {
    switch (record->kind) {
        case MSPTI_ACTIVITY_KIND_KERNEL: {
            auto *kernel = (msptiActivityKernel*)record;
            printf("Kernel: %s (%s), start=%lu, end=%lu, device=%u, stream=%u\n",
                   kernel->name, kernel->type, kernel->start, kernel->end,
                   kernel->ds.deviceId, kernel->ds.streamId);
            break;
        }
        case MSPTI_ACTIVITY_KIND_API: {
            auto *api = (msptiActivityApi*)record;
            printf("API: %s, start=%lu, end=%lu, correlationId=%lu\n",
                   api->name, api->start, api->end, api->correlationId);
            break;
        }
        case MSPTI_ACTIVITY_KIND_MEMCPY: {
            auto *memcpy = (msptiActivityMemcpy*)record;
            printf("Memcpy: bytes=%lu, start=%lu, end=%lu\n",
                   memcpy->bytes, memcpy->start, memcpy->end);
            break;
        }
        case MSPTI_ACTIVITY_KIND_MEMORY: {
            auto *mem = (msptiActivityMemory*)record;
            printf("Memory: op=%d, bytes=%lu, start=%lu, end=%lu\n",
                   mem->memoryOperationType, mem->bytes, mem->start, mem->end);
            break;
        }
        default:
            break;
    }
}

// 缓冲区完成回调
void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
    msptiActivity *record = nullptr;
    while (msptiActivityGetNextRecord(buffer, validSize, &record) == MSPTI_SUCCESS) {
        PrintActivity(record);
    }
    free(buffer);  // 消费完毕，释放缓冲区
}

int main() {
    // 1. 注册缓冲区回调
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);

    // 2. 使能需要采集的活动类型
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMCPY);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMORY);

    // 3. 执行业务代码（此处执行 ACL 计算等操作）
    // DoYourWork();

    // 4. 强制刷新所有缓冲区
    msptiActivityFlushAll(0);

    return 0;
}
```

### 5.2 关联分析：correlationId 的使用

通过 `correlationId` 建立 API 调用与 Kernel 执行之间的关联关系：

```cpp
#include <unordered_map>
#include <cstring>
#include "mspti.h"

std::unordered_map<uint64_t, msptiActivityApi*> g_apiMap;
std::unordered_map<uint64_t, msptiActivityKernel*> g_kernelMap;

void MsptiTrace(uint8_t *buffer, size_t size, size_t validSize) {
    msptiActivity *record = nullptr;
    while (msptiActivityGetNextRecord(buffer, validSize, &record) == MSPTI_SUCCESS) {
        if (record->kind == MSPTI_ACTIVITY_KIND_API) {
            auto *api = (msptiActivityApi*)record;
            auto *copy = (msptiActivityApi*)malloc(sizeof(msptiActivityApi));
            memcpy(copy, api, sizeof(msptiActivityApi));
            g_apiMap[copy->correlationId] = copy;
        } else if (record->kind == MSPTI_ACTIVITY_KIND_KERNEL) {
            auto *kernel = (msptiActivityKernel*)record;
            auto *copy = (msptiActivityKernel*)malloc(sizeof(msptiActivityKernel));
            memcpy(copy, kernel, sizeof(msptiActivityKernel));
            g_kernelMap[copy->correlationId] = copy;
        }
    }
}

void PrintCorrelationTrace() {
    for (auto &entry : g_apiMap) {
        uint64_t corrId = entry.first;
        auto *api = entry.second;
        printf("API: %s (corrId=%lu)\n", api->name, corrId);
        auto it = g_kernelMap.find(corrId);
        if (it != g_kernelMap.end()) {
            auto *kernel = it->second;
            printf("  -> Kernel: %s (%s), duration=%lu ns\n",
                   kernel->name, kernel->type, kernel->end - kernel->start);
        }
    }
}
```

### 5.3 外部关联 ID：跨层调用链追踪

使用 Push/Pop 机制将不同层级的调用关联起来：

```cpp
#include "mspti.h"

// 定义外部关联阶段
enum class ExternalStage {
    INIT = 0,
    EXEC = 1,
    CLEANUP = 2
};

void DoWork() {
    // 初始化阶段
    msptiActivityPushExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0,
        (uint64_t)ExternalStage::INIT);
    // ... 初始化代码 ...
    msptiActivityPopExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, nullptr);

    // 执行阶段
    msptiActivityPushExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0,
        (uint64_t)ExternalStage::EXEC);
    // ... 执行计算 ...
    msptiActivityPopExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, nullptr);
}
```

需使能 `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` 以采集外部关联记录。

### 5.4 域级 Marker 控制

动态启停指定域的 Marker 打点采集：

```cpp
#include "mspti.h"
#include "mstx/ms_tools_ext.h"

void DemoDomainControl() {
    // 创建域
    mstxDomainHandle_t domain = mstxDomainCreateA("myDomain");

    // 默认域为开启状态，打点数据会被采集
    uint64_t id1 = mstxDomainRangeStartA(domain, "range1", stream);
    mstxDomainRangeEnd(domain, id1);

    // 关闭域采集
    msptiActivityDisableMarkerDomain("myDomain");
    // 以下打点不会被采集
    uint64_t id2 = mstxDomainRangeStartA(domain, "range2", stream);
    mstxDomainRangeEnd(domain, id2);

    // 重新开启域采集
    msptiActivityEnableMarkerDomain("myDomain");
    uint64_t id3 = mstxDomainRangeStartA(domain, "range3", stream);
    mstxDomainRangeEnd(domain, id3);

    mstxDomainDestroy(domain);
}
```

### 5.5 HCCL 通信数据采集

采集多卡通信场景下的 HCCL 活动数据：

```cpp
#include "mspti.h"

void SetUpHcclCollection() {
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_HCCL);
}

// 在 CompleteFunc 中通过 ShowHcclInfo 打印 HCCL 记录
void ShowHcclInfo(msptiActivityHccl *hccl) {
    printf("HCCL: %s, comm=%s, bandwidth=%.2f GB/s, start=%lu, end=%lu\n",
           hccl->name, hccl->commName, hccl->bandWidth,
           hccl->start, hccl->end);
}
```

---

## 6. 完整样例参考

请参见 `samples/` 目录下的以下样例：

| 样例 | 说明 |
| --- | --- |
| `samples/mspti_activity/` | Activity API 基础用法：Kernel、Memory、API、Memcpy 采集 |
| `samples/mspti_correlation/` | 通过 correlationId 关联 API 与 Kernel |
| `samples/mspti_external_correlation/` | 外部关联 ID Push/Pop 用法 |
| `samples/mspti_hccl_activity/` | HCCL 通信数据采集 |
| `samples/mspti_mstx_activity_domain/` | 域级 Marker 采集控制 |

详细说明请参见《[msPTI 样例说明](../../../samples/README.md)》。
