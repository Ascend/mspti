# Callback API 使用指南

## 1. 概述

Callback API 允许用户在 Runtime API 或 HCCL API 调用前后注册回调函数，实现拦截、打点、数据采集等自定义逻辑。

### 1.1 核心概念

- **Domain**：回调域，标识一组相关的 API 函数。当前支持 `MSPTI_CB_DOMAIN_RUNTIME` 和 `MSPTI_CB_DOMAIN_HCCL`。
- **Callback ID**：回调标识，标识 Domain 内的具体 API 函数（如 `MSPTI_CBID_RUNTIME_LAUNCH`）。
- **Callback Site**：回调点，标识回调发生在 API 调用入口（`MSPTI_API_ENTER`）还是出口（`MSPTI_API_EXIT`）。
- **Subscriber**：订阅者句柄，通过 `msptiSubscribe` 创建，全局唯一（同一时刻仅支持一个订阅者）。
- **Userdata**：用户自定义数据，可在订阅时传入，在每次回调中透传。

### 1.2 工作流程

```text
msptiSubscribe → msptiEnableCallback / msptiEnableDomain → 执行业务代码
    → 触发回调（ENTER / EXIT） → msptiUnsubscribe
```

---

## 2. 枚举与数据结构

### 2.1 Callback Domain

```c
typedef enum {
    MSPTI_CB_DOMAIN_INVALID = 0,     // 无效域
    MSPTI_CB_DOMAIN_RUNTIME = 1,     // Runtime API 回调域
    MSPTI_CB_DOMAIN_HCCL = 2,        // HCCL 通信回调域
} msptiCallbackDomain;
```

### 2.2 Callback 回调点

```c
typedef enum {
    MSPTI_API_ENTER = 0,   // API 入口回调
    MSPTI_API_EXIT = 1,    // API 出口回调
} msptiApiCallbackSite;
```

### 2.3 Callback Data（回调数据结构）

回调函数接收的数据，包含调用的上下文信息：

```c
typedef struct {
    msptiApiCallbackSite callbackSite;   // 回调点（ENTER / EXIT）
    const char *functionName;            // API 函数名
    const void *functionParams;          // API 函数参数
    const void *functionReturnValue;     // API 返回值（仅 EXIT 有效）
    const char *symbolName;              // Kernel 符号名（仅 Launch 类回调有效）
    uint64_t correlationId;              // 关联 ID，与 Activity API 的记录对应
    uint64_t reserved1;                  // 保留
    uint64_t reserved2;                  // 保留
    uint64_t *correlationData;           // 入口与出口之间的共享数据
} msptiCallbackData;
```

### 2.4 Runtime Callback ID

`msptiCallbackIdRuntime` 枚举定义了 Runtime Domain 下可订阅的回调 ID，涵盖设备管理、上下文、流、Kernel 启动、内存操作等功能：

| 回调 ID | 值 | 说明 |
| --- | --- | --- |
| `MSPTI_CBID_RUNTIME_DEVICE_SET` | 1 | 设置设备 |
| `MSPTI_CBID_RUNTIME_DEVICE_RESET` | 2 | 重置设备 |
| `MSPTI_CBID_RUNTIME_LAUNCH` | 10 | 启动 Kernel |
| `MSPTI_CBID_RUNTIME_CPU_LAUNCH` | 11 | CPU Kernel 启动 |
| `MSPTI_CBID_RUNTIME_AICPU_LAUNCH` | 12 | AICPU Kernel 启动 |
| `MSPTI_CBID_RUNTIME_MALLOC` | 15 | 设备内存分配 |
| `MSPTI_CBID_RUNTIME_FREE` | 16 | 设备内存释放 |
| `MSPTI_CBID_RUNTIME_MALLOC_HOST` | 17 | 主机内存分配 |
| `MSPTI_CBID_RUNTIME_MEMCPY` | 22 | 内存拷贝 |
| `MSPTI_CBID_RUNTIME_MEMCPY_ASYNC` | 24 | 异步内存拷贝 |
| `MSPTI_CBID_RUNTIME_MEM_SET` | 27 | 内存设置 |
| ... | ... | 完整列表参见 `mspti_cbid.h` |

### 2.5 HCCL Callback ID

`msptiCallbackIdHccl` 枚举定义了 HCCL Domain 下可订阅的回调 ID：

| 回调 ID | 值 | 说明 |
| --- | --- | --- |
| `MSPTI_CBID_HCCL_ALLREDUCE` | 1 | AllReduce 操作 |
| `MSPTI_CBID_HCCL_BROADCAST` | 2 | Broadcast 操作 |
| `MSPTI_CBID_HCCL_ALLGATHER` | 3 | AllGather 操作 |
| `MSPTI_CBID_HCCL_REDUCE_SCATTER` | 4 | ReduceScatter 操作 |
| `MSPTI_CBID_HCCL_REDUCE` | 5 | Reduce 操作 |
| `MSPTI_CBID_HCCL_ALL_TO_ALL` | 6 | AllToAll 操作 |
| `MSPTI_CBID_HCCL_SEND` | 10 | Send 操作 |
| `MSPTI_CBID_HCCL_RECV` | 11 | Recv 操作 |
| `MSPTI_CBID_HCCL_BARRIER` | 8 | Barrier 操作 |

---

## 3. API 函数参考

### 3.1 订阅与注销

#### 3.1.1 msptiSubscribe

注册回调订阅者。同一时刻仅支持一个订阅者。

```c
msptiResult msptiSubscribe(
    msptiSubscriberHandle *subscriber,  // [out] 订阅者句柄
    msptiCallbackFunc callback,          // [in]  回调函数
    void *userdata);                     // [in]  用户自定义数据（透传）
```

- 回调函数签名：

```c
typedef void (*msptiCallbackFunc)(
    void *userdata,                    // 订阅时传入的用户数据
    msptiCallbackDomain domain,        // 回调域
    msptiCallbackId cbid,              // 回调 ID
    const msptiCallbackData *cbdata);  // 回调数据
```

#### 3.1.2 msptiUnsubscribe

注销订阅者，停止所有回调。

```c
msptiResult msptiUnsubscribe(msptiSubscriberHandle subscriber);
```

### 3.2 回调使能

#### 3.2.1 msptiEnableDomain

使能或禁用整个 Domain 的所有回调。

```c
msptiResult msptiEnableDomain(
    uint32_t enable,                  // 1 使能，0 禁用
    msptiSubscriberHandle subscriber,  // 订阅者句柄
    msptiCallbackDomain domain);       // 回调域
```

#### 3.2.2 msptiEnableCallback

使能或禁用特定 Domain 中特定 Callback ID 的回调。

```c
msptiResult msptiEnableCallback(
    uint32_t enable,                  // 1 使能，0 禁用
    msptiSubscriberHandle subscriber,  // 订阅者句柄
    msptiCallbackDomain domain,        // 回调域
    msptiCallbackId cbid);             // 回调 ID
```

---

## 4. 使用场景与示例

### 4.1 场景一：Domain 级别回调拦截

订阅整个 Runtime Domain，在每次 Runtime API 调用前后打印函数名。适用于全量 API 调用跟踪。

```cpp
#include <cstdio>
#include "mspti.h"

void MyCallback(void *userdata, msptiCallbackDomain domain,
                msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (domain != MSPTI_CB_DOMAIN_RUNTIME) return;

    if (cbdata->callbackSite == MSPTI_API_ENTER) {
        printf("[ENTER] %s\n", cbdata->functionName);
    } else if (cbdata->callbackSite == MSPTI_API_EXIT) {
        printf("[EXIT]  %s\n", cbdata->functionName);
    }
}

int main() {
    msptiSubscriberHandle subscriber = nullptr;

    // 1. 订阅回调
    msptiSubscribe(&subscriber, MyCallback, nullptr);

    // 2. 使能整个 Runtime Domain
    msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME);

    // 3. 执行业务代码（触发 API 调用）
    // DoYourWork();

    // 4. 注销订阅
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 4.2 场景二：特定 API 回调 + Userdata 透传

仅订阅 `MSPTI_CBID_RUNTIME_LAUNCH`，并在回调中通过 userdata 透传 context 和 stream，结合 MSTX 进行自定义打点。

```cpp
#include <cstdio>
#include <cstdlib>
#include "mspti.h"
#include "mstx/ms_tools_ext.h"

// 用户透传数据
struct UserData {
    aclrtContext *context;
    aclrtStream *stream;
};

void LaunchCallback(void *userdata, msptiCallbackDomain domain,
                    msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (domain != MSPTI_CB_DOMAIN_RUNTIME || !userdata) return;
    if (cbid != MSPTI_CBID_RUNTIME_LAUNCH) return;

    auto *ud = (UserData*)userdata;

    if (cbdata->callbackSite == MSPTI_API_ENTER) {
        // 在 Kernel Launch 入口打点
        mstxMarkA(cbdata->functionName, *(ud->stream));
    } else if (cbdata->callbackSite == MSPTI_API_EXIT) {
        // 在 Kernel Launch 出口打点
        mstxMarkA(cbdata->functionName, *(ud->stream));
    }
}

int main() {
    msptiSubscriberHandle subscriber = nullptr;
    UserData ud;
    // 初始化 ud.context 和 ud.stream ...

    // 1. 订阅回调，传入 userdata
    msptiSubscribe(&subscriber, LaunchCallback, &ud);

    // 2. 使能特定回调 ID
    msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME,
                        MSPTI_CBID_RUNTIME_LAUNCH);

    // 3. 可同时使能 Activity API 采集 Marker 和 Kernel 数据
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

    // 4. 执行业务代码
    // DoYourWork();

    // 5. 清理
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 4.3 场景三：入口/出口共享数据

利用 `correlationData` 在入口和出口回调之间传递数据：

```cpp
void CallbackWithSharedData(void *userdata, msptiCallbackDomain domain,
                             msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (domain != MSPTI_CB_DOMAIN_RUNTIME) return;

    if (cbdata->callbackSite == MSPTI_API_ENTER) {
        // 在入口处记录开始时间，存入 correlationData
        uint64_t startTime = GetCurrentTimestamp();
        if (cbdata->correlationData) {
            *cbdata->correlationData = startTime;
        }
        printf("[ENTER] %s\n", cbdata->functionName);
    } else if (cbdata->callbackSite == MSPTI_API_EXIT) {
        // 在出口处获取入口记录的时间，计算耗时
        uint64_t startTime = cbdata->correlationData ? *cbdata->correlationData : 0;
        uint64_t endTime = GetCurrentTimestamp();
        printf("[EXIT]  %s, duration=%lu ns\n",
               cbdata->functionName, endTime - startTime);
    }
}
```

### 4.4 场景四：HCCL 通信回调

订阅 HCCL Domain，监控通信操作：

```cpp
void HcclCallback(void *userdata, msptiCallbackDomain domain,
                  msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (domain != MSPTI_CB_DOMAIN_HCCL) return;

    const char *opName = "";
    switch (cbid) {
        case MSPTI_CBID_HCCL_ALLREDUCE:  opName = "AllReduce";  break;
        case MSPTI_CBID_HCCL_BROADCAST:  opName = "Broadcast";  break;
        case MSPTI_CBID_HCCL_ALLGATHER:  opName = "AllGather";  break;
        case MSPTI_CBID_HCCL_REDUCE:     opName = "Reduce";     break;
        default:                          opName = "Unknown";    break;
    }

    printf("[HCCL] %s %s\n",
           cbdata->callbackSite == MSPTI_API_ENTER ? "ENTER" : "EXIT",
           opName);
}

// 订阅时：
msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_HCCL);
```

---

## 5. 与 Activity API 的协作

Callback API 和 Activity API 可以协同使用，实现更全面的性能分析：

| 能力 | Callback API | Activity API |
| --- | --- | --- |
| 实时拦截 API 调用 | ✓ | — |
| 采集 Kernel 执行耗时 | — | ✓ |
| 入口/出口数据共享 | ✓ | — |
| 自定义打点（MSTX） | ✓ | ✓ |
| API 与 Kernel 关联 | ✓（通过 correlationId） | ✓（通过 correlationId） |

典型协作模式：使用 Callback API 订阅 Runtime 回调，在回调中通过 `mstxMarkA` 打点，同时使用 Activity API 采集 Marker 和 Kernel 数据，实现 API 调用层面的上下文字段采集与 Kernel 执行数据的关联分析。

---

## 6. 完整样例参考

| 样例 | 说明 |
| --- | --- |
| `samples/callback_domain/` | 订阅整个 Runtime Domain，打印每个 API 的进入和退出 |
| `samples/callback_mstx/` | 订阅特定 Launch 回调，结合 MSTX 打点和 userdata 透传 |
