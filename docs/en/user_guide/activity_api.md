# Activity API Usage Guide

## 1. Overview

The Activity API is the core data collection interface of msPTI. It uses an asynchronous buffer mechanism to collect Activity records of various kinds generated while a CANN application runs, including Kernel execution, API calls, memory operations, communication data, user-defined markers, and so on.

### 1.1 Core Concepts

- **Activity Record**: A performance record of an NPU activity. Each activity type corresponds to a C structure.
- **Activity Buffer**: A memory buffer that caches Activity Records. You provide the buffer, and msPTI fills it.
- **Activity Kind**: an activity type enumeration, identified by `msptiActivityKind`
- **Buffer Callback**: The buffer request and completion callbacks that you register. msPTI interacts with you through these callbacks.

### 1.2 Workflow

```text
You register callbacks → msPTI requests an empty buffer through RequestFunc → data collection fills the buffer
    → msPTI returns the full buffer through CompleteFunc → you iterate through and parse the records
    → you return the empty buffer through RequestFunc
```

---

## 2. Activity Kind

| Enumerated Value | Kind Constant | Description | Corresponding Data Structure |
| --- | --- | --- | --- |
| 1 | `MSPTI_ACTIVITY_KIND_MARKER` | User-defined marker | `msptiActivityMarker` |
| 2 | `MSPTI_ACTIVITY_KIND_KERNEL` | NPU Kernel execution | `msptiActivityKernel` |
| 3 | `MSPTI_ACTIVITY_KIND_API` | CANN API call | `msptiActivityApi` |
| 4 | `MSPTI_ACTIVITY_KIND_HCCL` | HCCL communication operation | `msptiActivityHccl` |
| 5 | `MSPTI_ACTIVITY_KIND_MEMORY` | Memory allocation/free | `msptiActivityMemory` |
| 6 | `MSPTI_ACTIVITY_KIND_MEMSET` | Memory set operation | `msptiActivityMemset` |
| 7 | `MSPTI_ACTIVITY_KIND_MEMCPY` | Memory copy operation | `msptiActivityMemcpy` |
| 8 | `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` | External correlation ID | `msptiActivityExternalCorrelation` |
| 9 | `MSPTI_ACTIVITY_KIND_COMMUNICATION` | Communication operator data | `msptiActivityCommunication` |
| 10 | `MSPTI_ACTIVITY_KIND_ACL_API` | ACL-level API call | — |
| 11 | `MSPTI_ACTIVITY_KIND_NODE_API` | Node-level API call | — |
| 12 | `MSPTI_ACTIVITY_KIND_RUNTIME_API` | Runtime-level API call | — |

---

## 3. Core Data Structures

### 3.1 Activity Kernel (Kernel Execution Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_KERNEL
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    struct { uint32_t deviceId; uint32_t streamId; } ds; // Device and stream IDs
    uint64_t correlationId;              // Correlation ID
    const char *type;                    // Kernel type
    const char *name;                    // Kernel name
} msptiActivityKernel;
```

### 3.2 Activity API (API Call Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_API
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    struct { uint32_t processId; uint32_t threadId; } pt; // Process and thread IDs
    uint64_t correlationId;              // Correlation ID
    const char *name;                    // API name
} msptiActivityApi;
```

### 3.3 Activity Memory (Memory Operation Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_MEMORY
    msptiActivityMemoryOperationType memoryOperationType; // Operation type (allocation/free)
    msptiActivityMemoryKind memoryKind;   // Memory type
    uint64_t correlationId;              // Correlation ID
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    uint64_t address;                    // Memory address
    uint64_t bytes;                      // Number of bytes
    uint32_t processId;                  // Process ID
    uint32_t deviceId;                   // Device ID
    uint32_t streamId;                   // Stream ID
} msptiActivityMemory;
```

### 3.4 Activity Memcpy (Memory Copy Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_MEMCPY
    msptiActivityMemcpyKind copyKind;    // Copy type (HTOD / DTOH / DTOD, and so on)
    uint64_t bytes;                      // Number of bytes copied
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    uint32_t deviceId;                   // Device ID
    uint32_t streamId;                   // Stream ID
    uint64_t correlationId;              // Correlation ID
    uint8_t isAsync;                     // Whether the copy is asynchronous
} msptiActivityMemcpy;
```

### 3.5 Activity Memset (Memory Set Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_MEMSET
    uint32_t value;                      // Value to set
    uint64_t bytes;                      // Number of bytes to set
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    uint32_t deviceId;                   // Device ID
    uint32_t streamId;                   // Stream ID
    uint64_t correlationId;              // Correlation ID
    uint8_t isAsync;                     // Whether the operation is asynchronous
} msptiActivityMemset;
```

### 3.6 Activity HCCL (Communication Operation Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_HCCL
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    struct { uint32_t deviceId; uint32_t streamId; } ds; // Device and stream IDs
    double bandWidth;                    // Bandwidth (GB/s)
    const char *name;                    // Communication operator name
    const char *commName;                // Communication group name
} msptiActivityHccl;
```

### 3.7 Activity Communication (Communication Operator Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_COMMUNICATION
    msptiCommunicationDataType dataType; // Data type
    uint64_t count;                      // Data count
    struct { uint32_t deviceId; uint32_t streamId; } ds; // Device and stream IDs
    uint64_t start;                      // Start timestamp (ns)
    uint64_t end;                        // End timestamp (ns)
    const char *algType;                 // Algorithm type
    const char *name;                    // Operator name
    const char *commName;                // Communication group name
    uint64_t correlationId;              // Correlation ID
} msptiActivityCommunication;
```

### 3.8 Activity Marker (User Marker Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_MARKER
    msptiActivityFlag flag;              // Marker flag (instant/start/end)
    msptiActivitySourceKind sourceKind;  // Data source (Host/Device)
    uint64_t timestamp;                  // Timestamp (ns)
    uint64_t id;                         // Marker ID
    msptiObjectId objectId;              // Object ID
    const char *name;                    // Marker name
    const char *domain;                  // Name of the domain it belongs to
} msptiActivityMarker;
```

### 3.9 Activity External Correlation (External Correlation Record)

```c
typedef struct {
    msptiActivityKind kind;              // Fixed to MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
    msptiExternalCorrelationKind externalKind; // External API type
    uint64_t externalId;                 // External ID
    uint64_t correlationId;              // Correlation ID
} msptiActivityExternalCorrelation;
```

---

## 4. API Function Reference

### 4.1 Lifecycle Functions

#### 4.1.1 `msptiActivityRegisterCallbacks`

Registers the Activity Buffer callbacks. You must call this function before enabling any Activity Kind.

```c
msptiResult msptiActivityRegisterCallbacks(
    msptiBuffersCallbackRequestFunc funcBufferRequested,
    msptiBuffersCallbackCompleteFunc funcBufferCompleted);
```

- `funcBufferRequested`: Callback invoked when msPTI requests an empty buffer. In this callback, you can allocate memory or return a buffer that has been fully consumed.
- `funcBufferCompleted`: Callback invoked when msPTI returns a buffer filled with data. Parse the data in this callback.
- If either parameter is NULL, the function returns `MSPTI_ERROR_INVALID_PARAMETER`.

**Callback Function Signatures:**

```c
// Buffer request callback
typedef void(*msptiBuffersCallbackRequestFunc)(
    uint8_t **buffer,    // [out] Buffer pointer to return, or NULL to reject the request
    size_t *size,        // [out] Buffer size
    size_t *maxNumRecords); // [out] Maximum number of records (0 means fill as much as possible)

// Buffer completion callback
typedef void(*msptiBuffersCallbackCompleteFunc)(
    uint8_t *buffer,     // [in] Buffer containing Activity records
    size_t size,         // [in] Total buffer size
    size_t validSize);   // [in] Number of valid data bytes
```

#### 4.1.2 `msptiActivityEnable`/`msptiActivityDisable`

Enables or disables Activity data collection for a specified type. You can call these functions multiple times to enable multiple types. By default, all types are disabled.

```c
msptiResult msptiActivityEnable(msptiActivityKind kind);
msptiResult msptiActivityDisable(msptiActivityKind kind);
```

#### 4.1.3 `msptiActivityIsEnabled`

Queries whether Activity collection is enabled for a specified type.

```c
bool msptiActivityIsEnabled(msptiActivityKind kind);
```

### 4.2 Data Read Functions

#### 4.2.1 `msptiActivityGetNextRecord`

Iterates through the Activity Records in a buffer. On the first call, pass NULL for `record`. On subsequent calls, pass the pointer returned by the previous call.

```c
msptiResult msptiActivityGetNextRecord(
    uint8_t *buffer,                // [in] Buffer
    size_t validBufferSizeBytes,    // [in] Number of valid bytes
    msptiActivity **record);        // [in/out] Record pointer
```

- A return value of `MSPTI_SUCCESS` indicates that a record was successfully obtained.
- A return value of `MSPTI_ERROR_MAX_LIMIT_REACHED` indicates that the buffer contains no more records.
- Check `record->kind` to determine the activity type, and then cast the record to the corresponding structure type.

### 4.3 Buffer Flush Functions

#### 4.3.1 `msptiActivityFlushAll`

Forcefully flushes all Activity buffers and returns the data through the CompleteFunc callback. The buffers are returned even when they are not full.

```c
msptiResult msptiActivityFlushAll(uint32_t flag);  // Reserved parameter
```

#### 4.3.2 `msptiActivityFlushPeriod`

Sets the interval for periodic flushes in milliseconds. Set it to 0 to disable periodic flushing.

```c
msptiResult msptiActivityFlushPeriod(uint32_t time);  // Time unit: ms
```

### 4.4 Domain Marker Control Functions

#### 4.4.1 `msptiActivityEnableMarkerDomain`/`msptiActivityDisableMarkerDomain`

Dynamically enables or disables Marker collection for a specified domain by name. By default, all domains are enabled.

```c
msptiResult msptiActivityEnableMarkerDomain(const char* name);
msptiResult msptiActivityDisableMarkerDomain(const char* name);
```

### 4.5 External Correlation Functions

#### 4.5.1 `msptiActivityPushExternalCorrelationId`

Pushes an external correlation ID for the calling thread to mark entry into an external API region. API Activity Records generated in this region are each preceded by an `EXTERNAL_CORRELATION` record.

```c
msptiResult msptiActivityPushExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // External API type
    uint64_t id);                       // External correlation ID
```

#### 4.5.2 `msptiActivityPopExternalCorrelationId`

Pops the external correlation ID for the calling thread to mark exit from an external API region.

```c
msptiResult msptiActivityPopExternalCorrelationId(
    msptiExternalCorrelationKind kind,  // External API type
    uint64_t *lastId);                  // [out] Returns the last ID
```

---

## 5. Complete Usage Examples

### 5.1 Basic Data Collection

The following example shows how to use the Activity API to collect activity data such as Kernel, API, Memcpy, and Memory:

```cpp
#include <cstdio>
#include <cstdlib>
#include "mspti.h"

// Buffer request callback
void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords) {
    *size = 8 * 1024 * 1024;  // 8MB
    *buffer = (uint8_t*)malloc(*size);
    *maxNumRecords = 0;  // Fill as much as possible
}

// Print Activity records
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

// Buffer completion callback
void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize) {
    msptiActivity *record = nullptr;
    while (msptiActivityGetNextRecord(buffer, validSize, &record) == MSPTI_SUCCESS) {
        PrintActivity(record);
    }
    free(buffer);  // Buffer consumed, release it
}

int main() {
    // 1. Register buffer callbacks
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);

    // 2. Enable the activity types to collect
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMCPY);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMORY);

    // 3. Run the business code (perform ACL computation and other operations here)
    // DoYourWork();

    // 4. Forcefully flush all buffers
    msptiActivityFlushAll(0);

    return 0;
}
```

### 5.2 Correlation Analysis: Using `correlationId`

The following example uses `correlationId` to establish associations between API calls and Kernel execution:

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

### 5.3 External Correlation IDs: Cross-Layer Call Chain Tracing

The following example uses the Push/Pop mechanism to associate calls at different layers:

```cpp
#include "mspti.h"

// Define external correlation stages
enum class ExternalStage {
    INIT = 0,
    EXEC = 1,
    CLEANUP = 2
};

void DoWork() {
    // Initialization stage
    msptiActivityPushExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0,
        (uint64_t)ExternalStage::INIT);
    // ... Initialization code ...
    msptiActivityPopExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, nullptr);

    // Execution stage
    msptiActivityPushExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0,
        (uint64_t)ExternalStage::EXEC);
    // ... Run the computation ...
    msptiActivityPopExternalCorrelationId(
        MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, nullptr);
}
```

You must enable `MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION` to collect external correlation records.

### 5.4 Domain-Level Marker Control

The following example dynamically enables or disables Marker collection for a specified domain:

```cpp
#include "mspti.h"
#include "mstx/ms_tools_ext.h"

void DemoDomainControl() {
    // Create a domain
    mstxDomainHandle_t domain = mstxDomainCreateA("myDomain");

    // The domain is enabled by default, and marker data is collected
    uint64_t id1 = mstxDomainRangeStartA(domain, "range1", stream);
    mstxDomainRangeEnd(domain, id1);

    // Disable domain collection
    msptiActivityDisableMarkerDomain("myDomain");
    // The following markers are not collected
    uint64_t id2 = mstxDomainRangeStartA(domain, "range2", stream);
    mstxDomainRangeEnd(domain, id2);

    // Re-enable domain collection
    msptiActivityEnableMarkerDomain("myDomain");
    uint64_t id3 = mstxDomainRangeStartA(domain, "range3", stream);
    mstxDomainRangeEnd(domain, id3);

    mstxDomainDestroy(domain);
}
```

### 5.5 HCCL Communication Data Collection

The following example collects HCCL activity data in multi-device communication scenarios:

```cpp
#include "mspti.h"

void SetUpHcclCollection() {
    msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_HCCL);
}

// Print HCCL records in CompleteFunc through ShowHcclInfo
void ShowHcclInfo(msptiActivityHccl *hccl) {
    printf("HCCL: %s, comm=%s, bandwidth=%.2f GB/s, start=%lu, end=%lu\n",
           hccl->name, hccl->commName, hccl->bandWidth,
           hccl->start, hccl->end);
}
```

---

## 6. Complete Sample References

See the following samples in the `samples/` directory:

| Sample | Description |
| --- | --- |
| `samples/mspti_activity/` | Basic Activity API usage: collecting Kernel, Memory, API, and Memcpy data |
| `samples/mspti_correlation/` | Uses correlationId to associate APIs with Kernels. |
| `samples/mspti_external_correlation/` | External correlation ID Push/Pop usage |
| `samples/mspti_hccl_activity/` | HCCL communication data collection |
| `samples/mspti_mstx_activity_domain/` | Domain-level Marker collection control |

For details, see [msPTI Sample Descriptions](../../../samples/README.md).
