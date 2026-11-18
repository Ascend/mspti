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
    → msPTI通过CompleteFunc返回可交付的缓冲区 → 用户遍历解析记录
```

---

## 2. Activity API 工作机制

Activity API 通过异步缓冲区机制，采集应用在 Host（CPU）和 Device（NPU）上产生的活动。相关声明位于 `mspti.h` 引用的 `mspti_activity.h` 中。使用前建议先了解以下术语，以及缓冲和刷新机制。

### 2.1 术语说明

1. **Activity Record（活动记录）**

   Host 与 Device 上的活动以 C 结构体形式上报，每种活动类型对应一种结构体（如 `msptiActivityKernel`、`msptiActivityApi`）。所有记录都以 `msptiActivity` 作为通用类型，并通过 `kind` 字段标识具体类型。遍历缓冲区时，先读取 `record->kind`，再将 `msptiActivity*` 强制转换为对应的结构体指针。示例参见第 6 章的 `PrintActivity` 函数和 `samples/mspti_activity` 样例。

2. **Activity Buffer（活动缓冲区）**

   Activity Buffer 用于将一个或多个 Activity Record 从 msPTI 传输到客户端。msPTI 会在活动发生时将记录写入缓冲区。由于不同 Activity Kind 的记录可能由 Host 线程或 Device 侧解析线程异步写入，msPTI **不保证**同一缓冲区内不同 Kind 记录之间的全局时序。客户端需及时提供空缓冲区，否则记录可能被丢弃；丢弃数量可通过 `msptiActivityGetNumDroppedRecords` 查询。

3. **Activity Kind（活动类型）**

   用于标识活动类型的枚举 `msptiActivityKind`。所有 Kind 默认关闭，必须显式调用 `msptiActivityEnable` 开启。

### 2.2 异步缓冲机制

异步缓冲通过 `msptiActivityRegisterCallbacks` 注册的回调完成，并可通过 `msptiActivityFlushAll` 主动触发交付：

1. 调用 `msptiActivityRegisterCallbacks` 注册两个回调：
   - **RequestFunc**：msPTI 需要空缓冲区时调用。客户端返回缓冲区指针、大小和最大记录数；将 `*buffer` 置为 `NULL` 表示拒绝本次请求，此时记录可能被丢弃。
   - **CompleteFunc**：缓冲区可以交付时调用。客户端通过 `msptiActivityGetNextRecord` 遍历 `validSize` 字节内的记录。
2. 回调应尽快返回。避免在回调中执行文件写入、网络传输等耗时操作，否则可能影响业务运行；建议先拷贝或入队记录，再由后台线程异步处理。
3. 可以预分配缓冲区池，在 RequestFunc 中复用已消费的缓冲区，减少频繁的 `malloc`/`free`。
4. 禁止在 RequestFunc 和 CompleteFunc 中调用 `msptiSubscribe`、`msptiUnsubscribe`、`msptiActivityFlushAll` 等管理接口，否则容易导致死锁或进程异常。

缓冲区大小需要权衡：缓冲区过小会导致 msPTI 频繁申请和交付缓冲区，过大则会延迟数据交付并增加内存占用。建议 Activity Buffer 不小于 2 MB，样例默认使用 8 MB；大规模通信采集可适当增大到 10 MB ~ 16 MB。当前缓冲区有效数据达到容量的约 80% 时，msPTI 会将其放入待交付队列，因此不必等缓冲区写满才交付。

> [!NOTE]
>
> msPTI 当前对所有应用线程使用同一个当前缓冲区，并由内部互斥锁保护。该方案内存占用较低，但多线程高并发采集时可能存在锁竞争。

Activity API 不要求先完成 CANN 初始化；只有开启采集后发生的活动才会被记录。调用 `msptiActivityEnable` 开启一个或多个 Kind 后采集即刻生效，建议在 `msptiSubscribe` 之后、业务执行之前调用。部分 Kind 依赖 `LD_PRELOAD` 注入（如已废弃的 `MSPTI_ACTIVITY_KIND_HCCL`，以及部分运行时不支持内存上报时的 `MEMORY`/`MEMSET`/`MEMCPY`）。若未正确预加载 `libmspti.so`，`msptiActivityEnable` 返回 `MSPTI_ERROR_WITHOUT_LD_PRELOAD`；Kind 不受支持时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

### 2.3 缓冲区刷新与数据交付

当缓冲区达到交付阈值时，msPTI 会将其加入待交付队列，由内部线程通过 CompleteFunc 回调返回给客户端。客户端也可以主动请求交付：

- **按需刷新（`msptiActivityFlushAll`）**：立即交付所有已排队缓冲区以及当前正在写入的缓冲区，即使缓冲区尚未写满。建议在采集结束、调用 `msptiFinalize` 或 `msptiUnsubscribe` 之前执行，确保剩余数据全部返回。
- **周期刷新（`msptiActivityFlushPeriod`）**：设置周期检查的时间间隔（单位 ms），msPTI 会定期检查并交付已达到交付阈值的缓冲区；设置为 0 可关闭周期检查。周期刷新默认关闭；即使已开启，仍可随时调用 `msptiActivityFlushAll` 按需交付。

> [!NOTE]
>
> - `msptiActivityFlushAll` 的 `flag` 为保留参数，传入 0 或其他值行为一致；接口不区分“交付已完成记录”和“强制交付未完成记录”。
> - CompleteFunc 的调用线程不固定：自动交付在 msPTI 内部线程中执行，`msptiActivityFlushAll` 触发的交付则在调用该接口的线程中执行。因此 CompleteFunc 必须线程安全，且不能长时间阻塞。
> - msPTI 只在记录信息（包括时间戳）填写完整后，才将记录写入缓冲区。Device 侧记录还需经过解析和 NPU→CPU 时间戳转换，交付可能滞后。对于 `KERNEL`、`COMMUNICATION` 等 Device 侧 Kind，`msptiActivityDisable` 会主动刷新设备采集数据，并等待解析队列排空（最多重试 5 次，每次间隔 20 ms）。因此结束采集时，建议先 Disable 相关 Kind，再调用 `msptiActivityFlushAll`。

### 2.4 属性配置

`msptiActivityGetAttribute` 和 `msptiActivitySetAttribute` 用于读写 Activity 采集行为相关的属性。当前支持的属性如下：

| 属性 | 值类型 | 说明 |
| --- | --- | --- |
| `MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE` | `uint32_t` | 设备侧通道缓冲区大小，默认 2 MB，取值范围 [2 MB, 10 MB]。该属性只对设置之后创建的通道缓冲区生效，因此应在开启 Activity Kind 之前设置。 |
| `MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK` | `msptiTimestampCallbackFunc` | 自定义时间戳回调，也可通过 `msptiActivityRegisterTimestampCallback` 注册。建议在开启任何 Activity Kind 之前完成设置。 |

若属性值缓冲区大小不足，接口返回 `MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT`；属性或值无效时返回 `MSPTI_ERROR_INVALID_PARAMETER`。

### 2.5 msPTI 内部线程模型

为减少对业务线程的干扰，msPTI 将缓冲区交付、Device 数据采集和解析等操作放到内部线程中执行：

- **ActivityManager 线程**：调用 `msptiActivityRegisterCallbacks` 时创建。该线程等待缓冲区达到交付阈值、周期刷新超时或采集停止等事件，并调度 CompleteFunc 回调完成交付。
- **Device 采集与消费线程**：按 Device 和通道创建采集线程（`DevProfTask`）和数据消费线程（`ChConsumer`），负责从驱动侧读取数据、解析 Activity Record，并将 NPU 时间戳转换为 CPU 时间戳。
- **解析任务线程**：按 Activity Kind 启动分析任务，处理 Kernel、Communication 等需要关联计算的记录。

调用 `msptiFinalize`（或执行 `msptiUnsubscribe` 后的清理流程）时，msPTI 会停止并回收上述线程。该过程不会自动交付缓冲区数据，因此调用方仍需提前执行 `msptiActivityFlushAll`。

### 2.6 线程安全与并发调用约束

msPTI 的 Buffer 回调可能在多个业务线程中并发执行，而 Activity API 的管理类接口会修改全局采集状态。请**尽量不要从多个线程并发调用** `msptiSubscribe`、`msptiUnsubscribe`、`msptiFinalize`、`msptiActivityRegisterCallbacks`、`msptiActivityEnable`/`msptiActivityDisable`、`msptiActivityFlushAll`、`msptiActivityFlushPeriod`、`msptiActivitySetAttribute` 等接口；否则可能造成 msPTI 内部状态异常，例如：

- 订阅或注销过程中回调句柄被替换，导致回调丢失，甚至访问已释放的订阅者。
- 缓冲区交付过程中被其他线程再次刷新，导致同一缓冲区被重复交付或数据顺序异常。
- 采集开关与订阅状态不一致，导致 Activity 数据丢失或记录不完整。
- 清理过程中内部线程和资源被并发访问，严重时可能导致进程崩溃。

建议遵循以下约束：

- 由单一管理线程负责订阅、使能、刷新和注销等操作；业务线程只执行业务代码。
- 调用 `msptiActivityFlushAll` 前，先确认业务线程不再产生新的 Activity，或先停止相关 Activity Kind 的采集。
- 不要在 Buffer 回调中调用上述管理接口，避免重入（参见 2.3 节）。
- 如确需在多线程环境中调用，请自行使用互斥锁串行化，并确保调用期间没有并发的订阅、注销、使能或刷新操作。
- 查询类接口（如 `msptiActivityIsEnabled`）可以并发调用。`msptiActivityGetNextRecord` 的遍历状态由缓冲区推导，不同缓冲区可在不同线程中交错遍历，但同一条记录指针不应跨线程共享。

---

## 3. 活动类型（Activity Kind）

| 枚举值 | Kind 常量 | 说明 | 对应数据结构 |
| --- | --- | --- | --- |
| 1 | `MSPTI_ACTIVITY_KIND_MARKER` | 用户自定义打点 | `msptiActivityMarker` |
| 2 | `MSPTI_ACTIVITY_KIND_KERNEL` | NPU Kernel 执行 | `msptiActivityKernel` |
| 3 | `MSPTI_ACTIVITY_KIND_API` | CANN API 调用 | `msptiActivityApi` |
| 4 | `MSPTI_ACTIVITY_KIND_HCCL` | HCCL 通信操作（**已废弃**，请改用 `MSPTI_ACTIVITY_KIND_COMMUNICATION`） | `msptiActivityHccl` |
| 5 | `MSPTI_ACTIVITY_KIND_MEMORY` | 内存分配/释放 | `msptiActivityMemory` |
| 6 | `MSPTI_ACTIVITY_KIND_MEMSET` | 内存设置操作 | `msptiActivityMemset` |
| 7 | `MSPTI_ACTIVITY_KIND_MEMCPY` | 内存拷贝操作 | `msptiActivityMemcpy` |
| 8 | `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` | 外部关联 ID | `msptiActivityExternalCorrelation` |
| 9 | `MSPTI_ACTIVITY_KIND_COMMUNICATION` | 通信算子数据 | `msptiActivityCommunication` |
| 10 | `MSPTI_ACTIVITY_KIND_ACL_API` | ACL 级 API 调用 | — |
| 11 | `MSPTI_ACTIVITY_KIND_NODE_API` | Node 级 API 调用 | — |
| 12 | `MSPTI_ACTIVITY_KIND_RUNTIME_API` | Runtime 级 API 调用 | — |
| 13 | `MSPTI_ACTIVITY_KIND_OVERHEAD` | msPTI 自身开销（资源创建、Buffer 申请/刷新等） | `msptiActivityOverhead` |

---

## 4. 核心数据结构

### 4.1 Activity Kernel（Kernel 执行记录）

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

### 4.2 Activity API（API 调用记录）

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

### 4.3 Activity Memory（内存操作记录）

记录内存分配与释放操作，对应CANN Runtime层内存管理API（如aclrtMalloc、aclrtFree等）。

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType; // 操作类型（ALLOCATION分配/RELEASE释放）
    msptiActivityMemoryKind memoryKind;   // 内存类型（DEVICE设备/HOST主机/MANAGED统一管理）
    uint64_t correlationId;              // 关联 ID
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint64_t address;                    // 内存地址
    uint64_t bytes;                      // 分配或释放的字节数
    uint32_t processId;                  // 进程 ID
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID，未关联流时设置为 MSPTI_INVALID_STREAM_ID
} msptiActivityMemory;
```

### 4.4 Activity Memcpy（内存拷贝记录）

记录Host与Device之间的内存拷贝操作，对应CANN Runtime层内存拷贝API（如aclrtMemcpy、aclrtMemcpyAsync等）。

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind;    // 拷贝方向（HTOH/HTOD/DTOH/DTOD/DEFAULT等）
    uint64_t bytes;                      // 拷贝字节数
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID
    uint64_t correlationId;              // 关联 ID
    uint8_t isAsync;                     // 是否异步操作（通过aclrtMemcpyAsync等异步API调用）
} msptiActivityMemcpy;
```

### 4.5 Activity Memset（内存设置记录）

记录内存置值操作，对应CANN Runtime层内存设置API（如aclrtMemset、aclrtMemsetAsync等）。

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_MEMSET
    uint32_t value;                      // 设置的目标值
    uint64_t bytes;                      // 设置的字节数
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint32_t deviceId;                   // 设备 ID
    uint32_t streamId;                   // 流 ID
    uint64_t correlationId;              // 关联 ID
    uint8_t isAsync;                     // 是否异步操作（通过aclrtMemsetAsync等异步API调用）
} msptiActivityMemset;
```

### 4.6 Activity HCCL（通信操作记录，已废弃）

> [!WARNING]
>
> `MSPTI_ACTIVITY_KIND_HCCL` 已废弃，仅为兼容旧版本保留。新代码请使用 `MSPTI_ACTIVITY_KIND_COMMUNICATION`（对应结构体 `msptiActivityCommunication`），该类型提供更完整的通信算子信息。

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

### 4.7 Activity Communication（通信算子记录）

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

### 4.8 Activity Marker（用户打点记录）

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

### 4.9 Activity External Correlation（外部关联记录）

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
    msptiExternalCorrelationKind externalKind; // 外部 API 类型
    uint64_t externalId;                 // 外部 ID
    uint64_t correlationId;              // 关联 ID
} msptiActivityExternalCorrelation;
```

### 4.10 Activity Overhead（msPTI 自身开销记录）

记录 msPTI 自身在采集数据时产生的额外开销，用于评估 Profiling 对业务造成的性能影响。当前覆盖以下环节：

- `MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE`：msPTI 内部资源创建/销毁（开启/关闭采集任务）。
- `MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST`：Activity Buffer 申请（调用 Request 回调的耗时）。
- `MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH`：Activity Buffer 刷新（调用 Complete 回调的耗时）。

```c
typedef struct {
    msptiActivityKind kind;              // 固定为 MSPTI_ACTIVITY_KIND_OVERHEAD
    msptiActivityOverheadKind overheadKind; // 开销产生的类型
    msptiObjectId objectId;              // 对象标识，有效字段由 objectKind 决定
    uint64_t start;                      // 开始时间戳（ns）
    uint64_t end;                        // 结束时间戳（ns）
    uint64_t correlationId;              // 关联 ID（预留）
    void *overheadData;                  // 附加信息指针（预留）
    msptiActivityObjectKind objectKind;  // 开销关联的对象类型（PROCESS/THREAD/DEVICE）
} msptiActivityOverhead;
```

开启 MSPTI_ACTIVITY_KIND_OVERHEAD 后，每次申请/刷新 Activity Buffer 以及开启/关闭 Profiling 任务时，
msPTI 会自动记录一条 overhead 记录并通过 CompleteFunc 回调返回给用户。

---

## 5. API 函数参考

### 5.1 生命周期函数

#### 5.1.1 msptiActivityRegisterCallbacks

注册 Activity Buffer 的回调函数。必须在开启任何 Activity Kind 之前调用。

```c
msptiResult msptiActivityRegisterCallbacks(
    msptiBuffersCallbackRequestFunc funcBufferRequested,
    msptiBuffersCallbackCompleteFunc funcBufferCompleted);
```

- `funcBufferRequested`：msPTI 请求空缓冲区时的回调。可以在此回调中分配内存或将消费完的缓冲区归还。
- `funcBufferCompleted`：msPTI 交付可读取的缓冲区时的回调，缓冲区不一定已写满。在此回调中解析数据。
- 任一参数为 NULL 则返回 `MSPTI_ERROR_INVALID_PARAMETER`。
- 注册回调时会启动内部 ActivityManager 线程。RequestFunc 通常在首次写入或申请新缓冲区的业务线程中执行；CompleteFunc 可能在 msPTI 内部线程或调用 `msptiActivityFlushAll` 的线程中执行。两个回调都必须保证线程安全。

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

#### 5.1.2 msptiActivityEnable / msptiActivityDisable

开启或关闭指定类型的 Activity 数据采集。可多次调用以开启多种类型。默认所有类型均为关闭状态。

```c
msptiResult msptiActivityEnable(msptiActivityKind kind);
msptiResult msptiActivityDisable(msptiActivityKind kind);
```

#### 5.1.3 msptiActivityIsEnabled

查询指定类型的 Activity 采集是否已开启。

```c
bool msptiActivityIsEnabled(msptiActivityKind kind);
```

### 5.2 数据读取函数

#### 5.2.1 msptiActivityGetNextRecord

遍历缓冲区中的 Activity Record。接口无内部遍历状态：每个缓冲区首次调用时 `*record` 传入 NULL，之后传入上一次返回的指针，下一条记录的位置由 `*record` 推导，不同缓冲区或不同线程可交错遍历。

```c
msptiResult msptiActivityGetNextRecord(
    uint8_t *buffer,                // [in] 缓冲区
    size_t validBufferSizeBytes,    // [in] 有效字节数
    msptiActivity **record);        // [in/out] 记录指针
```

- 返回 `MSPTI_SUCCESS` 表示成功获取一条记录。
- 返回 `MSPTI_ERROR_MAX_LIMIT_REACHED` 表示缓冲区已无更多记录。
- 返回 `MSPTI_ERROR_INVALID_PARAMETER` 表示 `buffer`/`record` 为空，或 `*record` 超出 `[buffer, buffer + validBufferSizeBytes)` 范围。
- 返回 `MSPTI_ERROR_INVALID_KIND` 表示记录的活动类型无效。
- 通过 `record->kind` 判断活动类型，再强转为对应的结构体类型。

### 5.3 缓冲刷新函数

#### 5.3.1 msptiActivityFlushAll

按需交付所有 Activity 缓冲区：既包括已达到交付阈值、正在等待交付的缓冲区，也包括当前正在写入的缓冲区，即使缓冲区未写满也会通过 CompleteFunc 返回。建议在采集结束以及调用 `msptiFinalize`/`msptiUnsubscribe` 之前调用。

```c
msptiResult msptiActivityFlushAll(uint32_t flag);  // flag 保留参数，传入任意值行为一致
```

> [!NOTE]
>
> 不要在多个线程中并发调用该接口，也不要与 `msptiSubscribe`/`msptiUnsubscribe` 等管理接口并发执行，否则可能导致缓冲区重复交付或 msPTI 内部状态异常。具体约束参见 2.6 节。

#### 5.3.2 msptiActivityFlushPeriod

设置周期刷新的时间间隔（单位 ms）。msPTI 会按该间隔交付已达到交付阈值的缓冲区；设置为 0 可关闭周期刷新（默认关闭）。开启周期刷新后，仍可调用 `msptiActivityFlushAll` 按需交付。

```c
msptiResult msptiActivityFlushPeriod(uint32_t time);  // time 单位为 ms
```

### 5.4 域标记控制函数

#### 5.4.1 msptiActivityEnableMarkerDomain / msptiActivityDisableMarkerDomain

按名称动态启停指定域的 Marker 打点采集。默认所有域均为开启状态。

```c
msptiResult msptiActivityEnableMarkerDomain(const char* name);
msptiResult msptiActivityDisableMarkerDomain(const char* name);
```

### 5.5 外部关联函数

#### 5.5.1 msptiActivityPushExternalCorrelationId

为调用线程推送一个外部关联 ID，标记进入外部 API 区域。该区域内生成的 API Activity Record 会先产生一条 `EXTERNAL_CORRELATION` 记录。

```c
msptiResult msptiActivityPushExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // 外部 API 类型
    uint64_t id);                       // 外部关联 ID
```

#### 5.5.2 msptiActivityPopExternalCorrelationId

为调用线程弹出外部关联 ID，标记离开外部 API 区域。

```c
msptiResult msptiActivityPopExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // 外部 API 类型
    uint64_t *lastId);                  // [out] 返回最后一个 ID
```

---

## 6. 完整使用示例

### 6.1 基础数据采集

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
            printf("Kernel: %s (%s), start=%llu, end=%llu, device=%u, stream=%u\n",
                   kernel->name, kernel->type, kernel->start, kernel->end,
                   kernel->ds.deviceId, kernel->ds.streamId);
            break;
        }
        case MSPTI_ACTIVITY_KIND_API: {
            auto *api = (msptiActivityApi*)record;
            printf("API: %s, start=%llu, end=%llu, correlationId=%llu\n",
                   api->name, api->start, api->end, api->correlationId);
            break;
        }
        case MSPTI_ACTIVITY_KIND_MEMCPY: {
            auto *memcpy = (msptiActivityMemcpy*)record;
            printf("Memcpy: bytes=%llu, start=%llu, end=%llu\n",
                   memcpy->bytes, memcpy->start, memcpy->end);
            break;
        }
        case MSPTI_ACTIVITY_KIND_MEMORY: {
            auto *mem = (msptiActivityMemory*)record;
            printf("Memory: op=%d, bytes=%llu, start=%llu, end=%llu\n",
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

    // 2. 开启需要采集的活动类型
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMCPY);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMORY);

    // 3. 执行业务代码（此处执行 ACL 计算等操作）
    // DoYourWork();

    // 4. 按需交付所有缓冲区（包括未写满的当前缓冲区）
    msptiActivityFlushAll(0);

    return 0;
}
```

### 6.2 关联分析：correlationId 的使用

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

### 6.3 外部关联 ID：跨层调用链追踪

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

需开启 `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` 以采集外部关联记录。

### 6.4 域级 Marker 控制

动态启停指定域的 Marker 打点采集：

```cpp
#include "acl/acl.h"
#include "mspti.h"
#include "mstx/ms_tools_ext.h"

aclrtStream stream;

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

### 6.5 通信数据采集（推荐 COMMUNICATION）

采集多卡通信场景下的活动数据。推荐使用 `MSPTI_ACTIVITY_KIND_COMMUNICATION`；`MSPTI_ACTIVITY_KIND_HCCL` 已废弃，请勿在新代码中使用。

```cpp
#include "mspti.h"

void SetUpCommunicationCollection() {
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
}

// 在 CompleteFunc 中通过 ShowCommunicationInfo 打印通信记录
void ShowCommunicationInfo(msptiActivityCommunication *comm) {
    printf("COMMUNICATION: %s, comm=%s, alg=%s, start=%lu, end=%lu\n",
           comm->name, comm->commName, comm->algType,
           comm->start, comm->end);
}
```

如需兼容旧版本代码，仍可使用 `MSPTI_ACTIVITY_KIND_HCCL` 和 `msptiActivityHccl`，用法与新类型类似；但该 Kind 已废弃，请勿在新代码中使用。

---

## 7. 完整样例参考

请参见 `samples/` 目录下的以下样例：

| 样例 | 说明 |
| --- | --- |
| `samples/mspti_activity/` | Activity API 基础用法：Kernel、Memory、API、Memcpy 采集 |
| `samples/mspti_correlation/` | 通过 correlationId 关联 API 与 Kernel |
| `samples/mspti_external_correlation/` | 外部关联 ID Push/Pop 用法 |
| `samples/mspti_hccl_activity/` | 演示已废弃的 `MSPTI_ACTIVITY_KIND_HCCL` 用法，仅用于兼容性参考；新代码请采集 `MSPTI_ACTIVITY_KIND_COMMUNICATION` |
| `samples/mspti_mstx_activity_domain/` | 域级 Marker 采集控制 |

详细说明请参见《[msPTI 样例说明](../../../samples/README.md)》。
