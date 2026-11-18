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

## 2. Callback API 工作机制

Callback API 允许用户在被采集应用调用 Runtime 或 HCCL 接口时插入自定义回调。msPTI 会在接口调用的入口和出口同步触发回调，用户可在回调中完成拦截、打点、数据采集等操作。

### 2.1 术语说明

1. **Callback Domain（回调域）**

    回调按域分组，每个域对应一组相关接口。当前支持以下域：

    | 回调域 | 说明 |
    | --- | --- |
    | `MSPTI_CB_DOMAIN_RUNTIME` | Runtime（ACL Runtime）接口回调域，覆盖设备、上下文、流、Kernel 启动、内存操作等接口。 |
    | `MSPTI_CB_DOMAIN_HCCL` | HCCL 通信接口回调域，覆盖 AllReduce、Broadcast、Send/Recv 等通信操作。 |

    只有 `mspti_cbid.h` 中已定义且被 msPTI 注入的接口才会产生回调；未注入的接口不会触发。

2. **Callback ID（回调标识）**

    每个回调在所属 Domain 内具有唯一 ID，用于在回调函数中区分具体接口。Runtime 域的 ID 由 `msptiCallbackIdRuntime` 定义，HCCL 域的 ID 由 `msptiCallbackIdHccl` 定义，完整列表参见 `mspti_cbid.h`。也可通过 `msptiGetCallbackName` 获取某个 ID 对应的接口名。

3. **Callback Function（回调函数）**

    回调函数类型为 `msptiCallbackFunc`，包含四个参数：

    - `userdata`：订阅时传入的用户数据，msPTI 原样透传。
    - `domain`：回调域，用于区分 Runtime 或 HCCL。
    - `cbid`：回调 ID，用于区分具体接口。
    - `cbdata`：本次回调的数据，包括回调点、接口名等，结构定义参见 3.3 节。

    `cbdata` 及其指向的内容仅在本次回调执行期间有效；如需在回调返回后使用，请自行拷贝。

4. **Callback Site（回调点）**

    用于标识回调发生在接口调用的哪个阶段：

    - `MSPTI_API_ENTER`：在真实接口调用之前触发。
    - `MSPTI_API_EXIT`：在真实接口调用返回之后触发。

    使能某个 Callback ID 后，该接口的 ENTER 和 EXIT 回调都会触发。正常调用时两者成对出现；若在接口调用过程中动态启停回调，可能只观察到其中一个回调点。

5. **Subscriber（订阅者）**

    订阅者用于将用户回调函数与一个或多个接口关联。同一时刻最多只能有一个订阅者；重复调用 `msptiSubscribe` 会返回 `MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED`。如需初始化新的订阅者，请先调用 `msptiUnsubscribe` 注销已有订阅者。

### 2.2 回调触发流程

msPTI 通过 `LD_PRELOAD` 注入接管已支持的 Runtime/HCCL 接口。应用调用这些接口时，处理流程如下：

1. 进入接口封装，准备回调上下文。
2. 若对应 Callback ID 已使能，在 ENTER 点同步调用用户回调。
3. 调用真实的 Runtime/HCCL 接口。
4. 接口返回后，在 EXIT 点再次同步调用用户回调；即使接口调用失败，EXIT 回调也会触发。
5. 将接口返回值返回给应用。

回调在调用该接口的应用线程中同步执行，不经过 msPTI 内部线程。多线程应用可能同时在多个线程上触发回调，因此回调函数必须线程安全。此外，回调耗时会计入被采集接口的耗时。

### 2.3 订阅与使能规则

- `msptiSubscribe` 仅创建订阅者，不会自动开启任何回调。
- 使用 `msptiEnableCallback` 开启或关闭指定 Domain 下某个 Callback ID 的回调。
- 使用 `msptiEnableDomain` 一次性开启或关闭整个 Domain 的回调；使用 `msptiEnableAllDomains` 可操作所有 Domain。
- 回调可在采集过程中随时开启或关闭，适合分段采集。同一订阅者上的使能和查询操作需要串行化，避免并发调用 `msptiEnableCallback`、`msptiEnableDomain`、`msptiEnableAllDomains` 和 `msptiGetCallbackState`。
- 可通过 `msptiGetCallbackState` 查询单个回调的开关状态，通过 `msptiGetEnabledCallbacks` 获取某个 Domain 下已开启的回调列表，通过 `msptiSupportedDomains` 获取支持的 Domain 及 Callback ID 范围。
- 使能回调依赖 `LD_PRELOAD` 中已加载有效的 `libmspti.so`，否则返回 `MSPTI_ERROR_WITHOUT_LD_PRELOAD`；订阅者未初始化时返回 `MSPTI_ERROR_NOT_INITIALIZED`；订阅者、Domain 或 Callback ID 无效时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

### 2.4 回调函数编写约束

- **保持轻量**：回调在业务线程中同步执行，因此文件写入、网络传输、大规模内存分配等耗时操作会直接增加接口耗时。建议在回调中只记录数据，将后续处理交给后台线程。
- **避免递归调用**：不要在回调中再次调用被拦截的 Runtime/HCCL 接口（如 Kernel 启动、内存拷贝），否则可能递归触发回调，导致死循环或栈溢出。
- **避免修改订阅状态**：不建议在回调中调用 `msptiSubscribe`、`msptiUnsubscribe`、`msptiFinalize` 或启停回调；这些操作会修改订阅状态，可能引起重入或死锁。
- **不要长期保存指针**：`cbdata` 及其内容仅在回调执行期间有效，回调返回后 msPTI 不再保证其有效性。
- **不要抛出异常**：回调函数应自行处理错误，避免异常传播到被拦截接口的调用栈。

### 2.5 与其他接口的协作和清理

- Callback API 与 Activity API 可以同时使用：在回调中通过 `mstxMarkA` 打点，并同时开启 `MSPTI_ACTIVITY_KIND_MARKER`、`MSPTI_ACTIVITY_KIND_KERNEL` 等 Kind，即可将 API 调用上下文与 Kernel 执行数据关联分析。
- 结束采集时，建议先调用 `msptiActivityFlushAll` 交付剩余 Activity 数据，再调用 `msptiUnsubscribe`。需要注意，`msptiUnsubscribe` 和 `msptiFinalize` 都不会自动刷新 Activity 缓冲区。
- `msptiFinalize` 会先停止 Activity 采集管线，再注销订阅者，适合在进程退出前统一清理；该接口可重复调用。

---

## 3. 枚举与数据结构

### 3.1 Callback Domain

```c
typedef enum {
    MSPTI_CB_DOMAIN_INVALID = 0,     // 无效域
    MSPTI_CB_DOMAIN_RUNTIME = 1,     // Runtime API 回调域
    MSPTI_CB_DOMAIN_HCCL = 2,        // HCCL 通信回调域
} msptiCallbackDomain;
```

### 3.2 Callback 回调点

```c
typedef enum {
    MSPTI_API_ENTER = 0,   // API 入口回调
    MSPTI_API_EXIT = 1,    // API 出口回调
} msptiApiCallbackSite;
```

### 3.3 Callback Data（回调数据结构）

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

### 3.4 Runtime Callback ID

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

### 3.5 HCCL Callback ID

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

## 4. API 函数参考

### 4.1 订阅与注销

#### 4.1.1 msptiSubscribe

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

- 该接口仅创建订阅者，不会开启任何回调；还需继续调用 `msptiEnableDomain` 或 `msptiEnableCallback`。
- 同一时刻仅支持一个订阅者。若已有订阅者，再次调用会返回 `MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED`，需先调用 `msptiUnsubscribe` 注销。
- `subscriber` 为 NULL 时返回 `MSPTI_ERROR_INVALID_PARAMETER`；内部初始化失败时返回 `MSPTI_ERROR_INNER`。
- 订阅成功后，msPTI 会启动 Activity 采集和同步时间相关的内部资源，建议在业务代码开始执行前完成订阅。

#### 4.1.2 msptiUnsubscribe

注销订阅者，停止后续回调。

```c
msptiResult msptiUnsubscribe(msptiSubscriberHandle subscriber);
```

- 注销时会同时停止 msPTI 内部采集线程。该接口**不会**自动刷新 Activity 缓冲区；若同时使用 Activity API，应先调用 `msptiActivityFlushAll`。
- 订阅者句柄与当前订阅不匹配时返回 `MSPTI_ERROR_INVALID_PARAMETER`；重复注销会被安全忽略。

### 4.2 回调开启

#### 4.2.1 msptiEnableDomain

开启或关闭整个 Domain 的回调。`enable` 非 0 表示开启，0 表示关闭；开启时会遍历该 Domain 内所有有效的 Callback ID。

```c
msptiResult msptiEnableDomain(
    uint32_t enable,                  // 1 开启，0 关闭
    msptiSubscriberHandle subscriber,  // 订阅者句柄
    msptiCallbackDomain domain);       // 回调域
```

- 未初始化订阅者时返回 `MSPTI_ERROR_NOT_INITIALIZED`；未正确设置 `LD_PRELOAD` 时返回 `MSPTI_ERROR_WITHOUT_LD_PRELOAD`；Domain 或订阅者无效时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

#### 4.2.2 msptiEnableCallback

开启或关闭指定 Domain 中某个 Callback ID 的回调。

```c
msptiResult msptiEnableCallback(
    uint32_t enable,                  // 1 开启，0 关闭
    msptiSubscriberHandle subscriber,  // 订阅者句柄
    msptiCallbackDomain domain,        // 回调域
    msptiCallbackId cbid);             // 回调 ID
```

- 使能后该接口的 ENTER 和 EXIT 回调都会触发。
- 错误码与 `msptiEnableDomain` 相同；Callback ID 超出当前 Domain 的有效范围时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

---

## 5. 使用场景与示例

### 5.1 场景一：Domain 级别回调拦截

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

    // 2. 开启整个 Runtime Domain
    msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME);

    // 3. 执行业务代码（触发 API 调用）
    // DoYourWork();

    // 4. 注销订阅
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 5.2 场景二：特定 API 回调 + Userdata 透传

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

    // 2. 开启特定回调 ID
    msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME,
                        MSPTI_CBID_RUNTIME_LAUNCH);

    // 3. 可同时开启 Activity API 采集 Marker 和 Kernel 数据
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

    // 4. 执行业务代码
    // DoYourWork();

    // 5. 清理
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 5.3 场景三：入口/出口共享数据

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

### 5.4 场景四：HCCL 通信回调

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

## 6. 与 Activity API 的协作

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

## 7. 完整样例参考

| 样例 | 说明 |
| --- | --- |
| `samples/callback_domain/` | 订阅整个 Runtime Domain，打印每个 API 的进入和退出 |
| `samples/callback_mstx/` | 订阅特定 Launch 回调，结合 MSTX 打点和 userdata 透传 |
