# Callback API User Guide

## 1. Overview

The Callback API allows you to register callback functions before and after Runtime API or HCCL API calls, implementing custom logic such as interception, instrumentation, and data collection.

### 1.1 Core Concepts

- **Domain**: Callback domain that identifies a group of related API functions. Currently, `MSPTI_CB_DOMAIN_RUNTIME` and `MSPTI_CB_DOMAIN_HCCL` are supported.
- **Callback ID**: Callback identifier that identifies a specific API function in a Domain (for example, `MSPTI_CBID_RUNTIME_LAUNCH`).
- **Callback Site**: Callback point that indicates whether the callback occurs at the API call entry (`MSPTI_API_ENTER`) or exit (`MSPTI_API_EXIT`).
- **Subscriber**: Subscriber handle created through `msptiSubscribe`. It is globally unique, with only one subscriber supported at a time.
- **Userdata**: User-defined data that you can pass in at subscription time and that is passed through in every callback.

### 1.2 Working Process

```text
msptiSubscribe → msptiEnableCallback / msptiEnableDomain → Run the service code
    → Callbacks are triggered (ENTER / EXIT) → msptiUnsubscribe
```

---

## 2. Enumerations and Data Structures

### 2.1 Callback Domain

```c
typedef enum {
    MSPTI_CB_DOMAIN_INVALID = 0,     // Invalid domain
    MSPTI_CB_DOMAIN_RUNTIME = 1,     // Runtime API callback domain
    MSPTI_CB_DOMAIN_HCCL = 2,        // HCCL communication callback domain
} msptiCallbackDomain;
```

### 2.2 Callback Site

```c
typedef enum {
    MSPTI_API_ENTER = 0,   // API entry callback
    MSPTI_API_EXIT = 1,    // API exit callback
} msptiApiCallbackSite;
```

### 2.3 Callback Data (Data Structure)

The callback function receives data that contains context information about the call:

```c
typedef struct {
    msptiApiCallbackSite callbackSite;   // Callback site (ENTER / EXIT)
    const char *functionName;            // API function name
    const void *functionParams;          // API function parameters
    const void *functionReturnValue;     // API return value (valid only on EXIT)
    const char *symbolName;              // Kernel symbol name (valid only for Launch-type callbacks)
    uint64_t correlationId;              // Correlation ID, corresponding to the Activity API records
    uint64_t reserved1;                  // Reserved
    uint64_t reserved2;                  // Reserved
    uint64_t *correlationData;           // Shared data between the entry and exit
} msptiCallbackData;
```

### 2.4 Runtime Callback ID

The `msptiCallbackIdRuntime` enumeration defines the callback IDs that can be subscribed in the Runtime Domain, covering functions such as device management, context, stream, Kernel launch, and memory operations:

| Callback ID | Value | Description |
| --- | --- | --- |
| `MSPTI_CBID_RUNTIME_DEVICE_SET` | 1 | Set the device. |
| `MSPTI_CBID_RUNTIME_DEVICE_RESET` | 2 | Reset the device. |
| `MSPTI_CBID_RUNTIME_LAUNCH` | 10 | Launch the Kernel. |
| `MSPTI_CBID_RUNTIME_CPU_LAUNCH` | 11 | CPU Kernel launch |
| `MSPTI_CBID_RUNTIME_AICPU_LAUNCH` | 12 | AICPU Kernel launch |
| `MSPTI_CBID_RUNTIME_MALLOC` | 15 | Device memory allocation |
| `MSPTI_CBID_RUNTIME_FREE` | 16 | Device memory release |
| `MSPTI_CBID_RUNTIME_MALLOC_HOST` | 17 | Host memory allocation |
| `MSPTI_CBID_RUNTIME_MEMCPY` | 22 | Memory copy |
| `MSPTI_CBID_RUNTIME_MEMCPY_ASYNC` | 24 | Asynchronous memory copy |
| `MSPTI_CBID_RUNTIME_MEM_SET` | 27 | Memory set |
| ... | ... | For the complete list, see `mspti_cbid.h` |

### 2.5 HCCL Callback ID

The `msptiCallbackIdHccl` enumeration defines the callback IDs that can be subscribed in the HCCL Domain:

| Callback ID | Value | Description |
| --- | --- | --- |
| `MSPTI_CBID_HCCL_ALLREDUCE` | 1 | AllReduce operation |
| `MSPTI_CBID_HCCL_BROADCAST` | 2 | Broadcast operation |
| `MSPTI_CBID_HCCL_ALLGATHER` | 3 | AllGather operation |
| `MSPTI_CBID_HCCL_REDUCE_SCATTER` | 4 | ReduceScatter operation |
| `MSPTI_CBID_HCCL_REDUCE` | 5 | Reduce operation |
| `MSPTI_CBID_HCCL_ALL_TO_ALL` | 6 | AllToAll operation |
| `MSPTI_CBID_HCCL_SEND` | 10 | Send operation |
| `MSPTI_CBID_HCCL_RECV` | 11 | Recv operation |
| `MSPTI_CBID_HCCL_BARRIER` | 8 | Barrier operation |

---

## 3. API Function Reference

### 3.1 Subscription and Unsubscription

#### 3.1.1 `msptiSubscribe`

Registers a callback subscriber. Only one subscriber is supported at a time.

```c
msptiResult msptiSubscribe(
    msptiSubscriberHandle *subscriber,  // [out] Subscriber handle
    msptiCallbackFunc callback,          // [in]  Callback function
    void *userdata);                     // [in]  User-defined data (passed through)
```

- Callback function signature:

```c
typedef void (*msptiCallbackFunc)(
    void *userdata,                    // User data passed in at subscription time
    msptiCallbackDomain domain,        // Callback domain
    msptiCallbackId cbid,              // Callback ID
    const msptiCallbackData *cbdata);  // Callback data
```

#### 3.1.2 `msptiUnsubscribe`

Unsubscribes the subscriber and stops all callbacks.

```c
msptiResult msptiUnsubscribe(msptiSubscriberHandle subscriber);
```

### 3.2 Callback Enablement

#### 3.2.1 `msptiEnableDomain`

Enables or disables all callbacks in a Domain.

```c
msptiResult msptiEnableDomain(
    uint32_t enable,                  // 1 to enable, 0 to disable
    msptiSubscriberHandle subscriber,  // Subscriber handle
    msptiCallbackDomain domain);       // Callback domain
```

#### 3.2.2 `msptiEnableCallback`

Enables or disables callbacks of a specific Callback ID in a specific Domain.

```c
msptiResult msptiEnableCallback(
    uint32_t enable,                  // 1 to enable, 0 to disable
    msptiSubscriberHandle subscriber,  // Subscriber handle
    msptiCallbackDomain domain,        // Callback domain
    msptiCallbackId cbid);             // Callback ID
```

---

## 4. Use Scenarios and Examples

### 4.1 Scenario 1: Domain-Level Callback Interception

Subscribe to the entire Runtime Domain and print the function name before and after each Runtime API call. This is suitable for full API call tracking.

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

    // 1. Subscribe to callbacks
    msptiSubscribe(&subscriber, MyCallback, nullptr);

    // 2. Enable the entire Runtime Domain
    msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME);

    // 3. Run the service code (triggers API calls)
    // DoYourWork();

    // 4. Unsubscribe
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 4.2 Scenario 2: Specific API Callback and Userdata Passing

Subscribe only to `MSPTI_CBID_RUNTIME_LAUNCH` and pass the context and stream through userdata in the callback, combined with MSTX for custom instrumentation.

```cpp
#include <cstdio>
#include <cstdlib>
#include "mspti.h"
#include "mstx/ms_tools_ext.h"

// User data passed through
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
        // Instrument the Kernel Launch entry
        mstxMarkA(cbdata->functionName, *(ud->stream));
    } else if (cbdata->callbackSite == MSPTI_API_EXIT) {
        // Instrument the Kernel Launch exit
        mstxMarkA(cbdata->functionName, *(ud->stream));
    }
}

int main() {
    msptiSubscriberHandle subscriber = nullptr;
    UserData ud;
    // Initialize ud.context and ud.stream ...

    // 1. Subscribe to callbacks and pass in userdata
    msptiSubscribe(&subscriber, LaunchCallback, &ud);

    // 2. Enable a specific callback ID
    msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME,
                        MSPTI_CBID_RUNTIME_LAUNCH);

    // 3. You can also enable the Activity API to collect Marker and Kernel data
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);

    // 4. Run the service code
    // DoYourWork();

    // 5. Clean up
    msptiUnsubscribe(subscriber);
    return 0;
}
```

### 4.3 Scenario 3: Entry/Exit Shared Data

Use `correlationData` to pass data between the entry and exit callbacks:

```cpp
void CallbackWithSharedData(void *userdata, msptiCallbackDomain domain,
                             msptiCallbackId cbid, const msptiCallbackData *cbdata) {
    if (domain != MSPTI_CB_DOMAIN_RUNTIME) return;

    if (cbdata->callbackSite == MSPTI_API_ENTER) {
        // Record the start time at the entry and store it in correlationData
        uint64_t startTime = GetCurrentTimestamp();
        if (cbdata->correlationData) {
            *cbdata->correlationData = startTime;
        }
        printf("[ENTER] %s\n", cbdata->functionName);
    } else if (cbdata->callbackSite == MSPTI_API_EXIT) {
        // Get the time recorded at the entry and calculate the duration
        uint64_t startTime = cbdata->correlationData ? *cbdata->correlationData : 0;
        uint64_t endTime = GetCurrentTimestamp();
        printf("[EXIT]  %s, duration=%lu ns\n",
               cbdata->functionName, endTime - startTime);
    }
}
```

### 4.4 Scenario 4: HCCL Communication Callback

Subscribe to the HCCL Domain to monitor communication operations:

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

// At subscription time:
msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_HCCL);
```

---

## 5. Collaboration with the Activity API

The Callback API and the Activity API can be used together for more comprehensive performance analysis:

| Capability | Callback API | Activity API |
| --- | --- | --- |
| Intercept API calls in real time. | ✓ | — |
| Collect Kernel execution durations. | — | ✓ |
| Share data between entry and exit. | ✓ | — |
| Custom instrumentation (MSTX) | ✓ | ✓ |
| Correlate APIs with Kernels. | ✓ (through `correlationId`) | ✓ (through `correlationId`) |

A typical collaboration pattern: use the Callback API to subscribe to Runtime callbacks and call `mstxMarkA` in the callbacks for instrumentation, while using the Activity API to collect Marker and Kernel data. This enables context field collection at the API call level and correlation analysis with Kernel execution data.

---

## 6. Complete Sample Reference

| Sample | Description |
| --- | --- |
| `samples/callback_domain/` | Subscribe to the entire Runtime Domain and print the entry and exit of each API. |
| `samples/callback_mstx/` | Subscribe to a specific Launch callback, combined with MSTX instrumentation and userdata passing. |
