# msPTI User Guide

## 1. Overview

This document is intended for users who want to use MindStudio Profiler Tools Interface (msPTI) to analyze the performance of NPU applications. msPTI provides three programming interfaces for different development languages and analysis scenarios. If you are new to msPTI, you are advised to read [msPTI Quick Start](../quick_start/mspti_quick_start.md) first to prepare the environment, and then select the interface guide that suits your needs.

### 1.1 Interface Selection at a Glance

| Interface Type | Supported Languages | Core Capabilities | Typical Scenarios |
| --- | --- | --- | --- |
| Activity API | C/C++ | Collects Kernel, Memory, HCCL, and Marker activity data in asynchronous buffer mode, with low overhead and the most complete coverage of activity types. | Building Tracing/Profiling tools and locating performance bottlenecks |
| Callback API | C/C++ | Registers callbacks before and after Runtime/HCCL API calls, and supports `userdata` pass-through and `correlationData` sharing. | Intercepting API calls, collecting data through instrumentation, and injecting custom logic |
| Python API | Python | Provides a Monitor wrapper mode that starts collection with a single line and includes a built-in multithreaded consumer. | Quickly adding performance monitoring to Python training scripts |

### 1.2 Navigation to the Three Guides

| Document | Supported Languages | Scenarios |
| --- | --- | --- |
| [Activity API Guide](./activity_api.md) | C/C++ | Collects Kernel, Memory, HCCL, and Marker activity data and builds Tracing and Profiling tools. |
| [Callback API Guide](./callback_api.md) | C/C++ | Subscribes to Runtime/HCCL callbacks and executes custom logic before and after API calls. |
| [Python API Guide](./python_api.md) | Python | Uses `KernelMonitor`, `HcclMonitor`, `MstxMonitor`, and `CommunicationMonitor` for quick integration. |

### 1.3 Hybrid Usage Strategy

The Callback API and Activity API can be enabled at the same time without conflicting with each other. Typical combinations:

- **Callback instrumentation + Activity collection**: Use the Callback API to call `mstxMarkA` at the entry and exit of Launch Kernel, and enable the Activity API to collect `MSPTI_ACTIVITY_KIND_MARKER` and `MSPTI_ACTIVITY_KIND_KERNEL` at the same time, enabling correlation analysis between the API context and Kernel execution data.
- **Activity collection + Python consumption**: Use the C Activity API to collect all data, and read and analyze the results through Python extension bindings.

## 2. Quick Navigation

| Stage | Reference Document | Description |
| --- | --- | --- |
| Environment preparation and tool installation | [msPTI Quick Start](../quick_start/mspti_quick_start.md) | Required reading for first-time use: hardware requirements, software installation, and environment configuration |
| Complete C interface reference | [C API Reference](../api_reference/c_api/README.md) | All functions and data structures of the Activity API and Callback API |
| Complete Python interface reference | [Python API Reference](../api_reference/python_api/README.md) | All interfaces of `KernelMonitor`/`HcclMonitor`/`MstxMonitor`/`CommunicationMonitor` |
| Sample list and descriptions | [msPTI Sample Guide](../user_guide/samples_guide.md) | 9 hands-on samples covering all interface types |
| Development and build | [msPTI Development Guide](../development_guide/development_guide.md) | Source code compilation, secondary development, testing, and submission |
| Best practices and typical cases | [msPTI Best Practices](../best_practices/basic_cases.md) | Interface selection, buffer management, and anti-pattern avoidance |

## 3. Common Conventions

### 3.1 Environment Variables

To use msPTI, first set the CANN environment variables. `${install_path}` indicates the CANN installation path, for example, `/usr/local/Ascend/cann`. Run the following command:

```bash
source ${install_path}/set_env.sh
```

If you use the msPTI Python API, you also need to set `LD_PRELOAD`:

```bash
export LD_PRELOAD=${ASCEND_HOME_PATH}/lib64/libmspti.so
```

### 3.2 Error Codes

All msPTI functions return an `msptiResult` enumeration value:

| Error Code | Value | Description | Trigger Scenario |
| --- | --- | --- | --- |
| MSPTI_SUCCESS | 0 | Success | The operation completes normally |
| MSPTI_ERROR_INVALID_PARAMETER | 1 | Invalid parameter | A function parameter is NULL or of an incorrect type |
| MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED | 2 | Multiple subscribers are not supported | `msptiSubscribe` is called again when a subscriber already exists |
| MSPTI_ERROR_MAX_LIMIT_REACHED | 3 | Maximum limit reached | The Activity Buffer has no more space for Records |
| MSPTI_ERROR_DEVICE_OFFLINE | 4 | Device offline | Device-side information cannot be obtained |
| MSPTI_ERROR_QUEUE_EMPTY | 5 | Queue is empty | External Correlation ID matching fails |
| MSPTI_ERROR_WITHOUT_LD_PRELOAD | 6 | LD_PRELOAD not set | `libmspti.so` is not preloaded |
| MSPTI_ERROR_INNER | 999 | Internal error | MSPTI initialization fails or an internal exception occurs |

> [!NOTE]
> 
> For detailed error handling, see the "Returns" section in each API reference document.

### 3.3 Constraints and Limitations

- You **must not** use msPTI together with other performance data collection tools. Otherwise, the collected data will be lost.
- msPTI depends on the Linux operating system and Ascend NPU hardware and does not support the Windows environment.
- The maximum Activity Buffer size is 256 MB.
- Only one Callback subscriber is supported at a time.
- All Activity Kinds are disabled by default. You must explicitly call `msptiActivityEnable` to enable them.

### 3.4 Performance Notes

- Enable Activity Kinds on demand. Each additional enabled Kind increases the performance overhead.
- Avoid time-consuming operations in the Buffer CompleteFunc (such as file writes and network transfers). You are advised to put the raw data into a queue and process it asynchronously in background threads.
- Keep the callback functions of the Python Monitor as lightweight as possible. In high-throughput scenarios, you are advised to use the multithreaded consumer mode.
