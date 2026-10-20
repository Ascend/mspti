# PM Sampling API Usage Guide

## 1. Overview

The PM Sampling API is the periodic device hardware metric collection interface of msPTI. It samples the target device at a fixed time interval, collecting hardware metrics such as HBM bandwidth utilization, HCCS/PCIe/RoCE interconnect bandwidth, power consumption, temperature, and AI Core/Vector/Cube utilization and frequency.

The collection capability of metrics depends on the chip model; filter unavailable metrics through the availability query API before configuration.

### 1.1 Key Concepts

- **PM Sampling Object**: An opaque handle of a PM sampling session, created by `msptiPmSamplingEnable` and destroyed by `msptiPmSamplingDisable`. A device allows at most one enabled session at a time.
- **Metric ID**: The metric number of the static metric table, identified by the `msptiPmSamplingMetricId` enumeration.
- **Sample**: The metric set of one sampling window, carrying start and end timestamps (host nanoseconds).
- **Sample Entry**: The self-describing unit of a sample, each carrying `{metricId, instanceId, value}`. Per-instance metrics (e.g. HCCS per link) produce one entry per instance, so the entry count of a sample is not fixed and is usually greater than or equal to the number of configured metrics.
- **Counter Data Image**: A buffer opaque to users, allocated by the user with the size returned by `msptiPmSamplingGetCounterDataSize` and accessible only through the APIs of this module, used to hold decoded samples.

### 1.2 Workflow

```text
Enable (create the session object) → SetConfig (configure metric set/interval/buffer)
    → Start (begin the sampling window) → periodic sampling produces samples accumulated in the session queue
    → Stop (end the sampling window) → DecodeData (decode samples into the image) / GetSampleEntries (read the queue directly)
    → Disable (destroy the object and disable sampling)
```

Start/Stop can be executed repeatedly; samples of multiple windows accumulate in the same session. After Stop, SetConfig can be called again to reconfigure and restart.

---

## 2. Metric Set (Metric ID)

| ID | Enumeration | Description | Unit |
| --- | --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL` | NPU overall utilization (semantics may vary with HDK versions) | % |
| 1 | `MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL` | AI Core utilization | % |
| 2 | `MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL` | AI Vector utilization | % |
| 3 | `MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL` | AI Cube utilization | % |
| 4 | `MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL` | HBM bandwidth utilization | % |
| 5 | `MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL` | HBM memory utilization (used capacity / total capacity) | % |
| 6 | `MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW` | HCCS transmit bandwidth (per link) | bytes/s |
| 7 | `MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW` | HCCS receive bandwidth (per link) | bytes/s |
| 8 | `MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW` | PCIe transmit bandwidth | bytes/s |
| 9 | `MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW` | PCIe receive bandwidth | bytes/s |
| 10 | `MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW` | RoCE transmit bandwidth | bytes/s |
| 11 | `MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW` | RoCE receive bandwidth | bytes/s |
| 12 | `MSPTI_PM_SAMPLING_METRIC_ID_POWER` | Power consumption | W |
| 13 | `MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE` | Temperature | degrees Celsius |
| 14 | `MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ` | Current AI Core frequency | MHz |

Notes:

- Both HBM metrics are utilization ratios: only the HBM bandwidth utilization is provided, not read/write byte rates; the memory utilization is the ratio of used capacity to total capacity, normalized by the collector.
- HCCS bandwidth is reported per link, where `instanceId` is the link number 0–15; one HCCS metric produces up to 16 entries in a sample.
- HCCS bandwidth is supported only on chips with the HCCS interconnect; chips using the UB interconnect (e.g., the 950 series) do not provide HCCS metrics. Before configuration, it is recommended to check metric availability via `msptiPmSamplingGetCounterAvailability`.
- RoCE bandwidth is not yet available in the current version.
- For metrics without the instance concept (power consumption, temperature, HBM bandwidth utilization, etc.), `instanceId` is fixed to 0; when a metric has no data in the window, one entry of `{metricId, 0, 0.0}` is produced.
- Power is produced in watts.
- The short name, group, data type, unit, and availability of a metric can be queried by `msptiPmSamplingGetMetricProperties` for display classification.

---

## 3. Key Data Structures

### 3.1 msptiPmSamplingMetricProperties (Metric Properties)

Static properties of a metric returned by `msptiPmSamplingGetMetricProperties`, used to learn the name, group, unit, and availability of a metric before configuration.

```c
typedef struct
{
    char shortName[64];                     // Metric short name, e.g. "power", "hccs_tx_bw"
    msptiPmSamplingMetricGroup metricGroup; // Metric group
    msptiPmSamplingMetricDataType dataType; // Data type
    char unit[32];                          // Unit string of the value, e.g. "bytes/s", "%", "W"
    bool available;                         // Whether the metric is available on the current chip model
} msptiPmSamplingMetricProperties;
```

The values of metricGroup and dataType are as follows:

| Enumeration | Constant | Description |
| --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY` | Memory (HBM related metrics) |
| 1 | `MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK` | Network (HCCS/PCIe/RoCE bandwidth) |
| 2 | `MSPTI_PM_SAMPLING_METRIC_GROUP_POWER` | Power |
| 3 | `MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL` | Temperature |
| 4 | `MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION` | Utilization |
| 5 | `MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY` | Frequency |

| Enumeration | Constant | Description |
| --- | --- | --- |
| 0 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO` | Dimensionless ratio |
| 1 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT` | Percentage |
| 2 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT` | Throughput (bytes/s) |
| 3 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER` | Power (W) |
| 4 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE` | Temperature (°C) |
| 5 | `MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY` | Frequency (MHz) |

### 3.2 msptiPmSamplingSampleEntry (Sample Value Entry)

A sample consists of a list of value entries. Each entry self-describes the metric and hardware instance it belongs to. Consumers do not need to know the binary layout of a sample and can simply read it entry by entry.

```c
typedef struct
{
    uint32_t metricId;   // Metric ID, see MSPTI_PM_SAMPLING_METRIC_ID_*
    uint32_t instanceId; // Hardware instance ID, 0 if the metric has no instances (e.g. HCCS bandwidth is reported per link 0~15)
    double value;        // Metric value, in the unit of the corresponding metric
} msptiPmSamplingSampleEntry;
```

- Each configured metric produces entries according to the number of hardware instances it reports: multi-instance metrics (such as HCCS bandwidth) produce one entry per link, while metrics without instances (power, temperature, HBM bandwidth utilization, etc.) always use instanceId 0.
- If a metric has no data in a window, a single entry with instanceId 0 and value 0 is produced.
- For the complete values of `metricId`, see the metric set in Chapter 2.

---

## 4. Lifecycle and States

A session object has an implicit state machine: **Created → Configured → Running → Stopped**.

| API | Created | Configured | Running | Stopped |
| --- | :-: | :-: | :-: | :-: |
| msptiPmSamplingSetConfig | √ | √ | × | √ (clears the sample queue) |
| msptiPmSamplingStart | × | √ | √ (idempotent) | √ |
| msptiPmSamplingStop | × | × | √ | × |
| msptiPmSamplingDecodeData | × | × | √ | √ |
| msptiPmSamplingDisable | √ | √ | √ (internally stops first) | √ |

- Reconfiguring a Stopped session by SetConfig clears its pending sample queue, because the metric set may change and old samples do not match the new entry layout.
- Stop keeps the object, configuration, and already-collected samples, so the session can be restarted or decoded afterwards.
- If Disable is called in the Running state, sampling is stopped internally first (flushing the last sample window) before the object is destroyed.

---

## 5. API Reference

### 5.1 Session Lifecycle Functions

#### 5.1.1 msptiPmSamplingEnable

Creates a PM Sampling object and enables PM sampling on the specified device. A device allows at most one enabled session at a time; enabling again returns `MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED`.

```c
msptiResult msptiPmSamplingEnable(msptiPmSamplingEnableParams *params);
```

- `params.deviceId`: Target device ID.
- `params.pPmSamplingObject`: [out] The created session object, required by all subsequent APIs.
- Returns `MSPTI_ERROR_DEVICE_OFFLINE` if the target device is offline.
- The object is destroyed by `msptiPmSamplingDisable`.

#### 5.1.2 msptiPmSamplingDisable

Destroys the PM Sampling object and disables PM sampling on the device. If called in the Running state, sampling is stopped internally first (flushing the last sample window) before the object is destroyed.

```c
msptiResult msptiPmSamplingDisable(msptiPmSamplingDisableParams *params);
```

- `params.pPmSamplingObject`: The session object to destroy; the handle becomes invalid afterwards.
- When the object is destroyed, collected but unread samples in the session are discarded.

### 5.2 Configuration and Control Functions

#### 5.2.1 msptiPmSamplingSetConfig

Configures the metric set, sampling interval, trigger mode, and hardware buffer. Can be called in the Created, Configured, or Stopped state; calling in the Running state returns `MSPTI_ERROR_INVALID_OPERATION`.

```c
msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params);
```

- `params.numMetrics`/`params.pMetricIds`: Metric set. Out-of-range, duplicated, or unavailable metric IDs return `MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC`; filter through `msptiPmSamplingGetCounterAvailability` before configuration.
- `params.samplingInterval`: Sampling interval in ns, range [1ms, 1s].
- `params.triggerMode`: Trigger mode; currently only `MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL` is supported.
- `params.hardwareBufferSize`: Capacity of the internal sample queue in bytes; 0 for the default size (10000 samples). A non-zero value must hold at least one sample and at most 512 MB; otherwise, `MSPTI_ERROR_INVALID_PARAMETER` is returned. The per-sample overhead in the queue depends on the metric set; size the queue from the per-sample size and the expected number of in-queue samples, typically on the same order as the image capacity.
- `params.hwBufferAppendMode`: Append policy when the queue is full; `KEEP_OLDEST` keeps collected samples and drops new ones, `KEEP_LATEST` overwrites the oldest samples and keeps the newest.
- Reconfiguring a Stopped session clears its pending sample queue; see Chapter 4 for the reason.

#### 5.2.2 msptiPmSamplingStart / msptiPmSamplingStop

Starts/ends the sampling window. Repeated Start calls in the Running state are idempotent; Stop keeps the object, configuration, and already-collected samples, so the session can be restarted or decoded. Start/Stop can be executed repeatedly, and samples of multiple windows accumulate in the same session.

```c
msptiResult msptiPmSamplingStart(msptiPmSamplingStartParams *params);
msptiResult msptiPmSamplingStop(msptiPmSamplingStopParams *params);
```

- `params.pPmSamplingObject`: Session object.
- Returns `MSPTI_ERROR_INVALID_OPERATION` when the state does not allow the call (e.g. Start before configuration, or Stop in a non-Running state).

### 5.3 Metric Query Functions

#### 5.3.1 msptiPmSamplingGetNumMetrics

Obtains the total number of configurable metrics, that is, the size of the static metric table.

```c
msptiResult msptiPmSamplingGetNumMetrics(msptiPmSamplingGetNumMetricsParams *params);
```

- `params.deviceId`: Target device ID.
- `params.pNumMetrics`: [out] Total number of metrics.

#### 5.3.2 msptiPmSamplingGetMetricProperties

Queries the static properties of a metric by metricId (short name, group, data type, unit, availability) for display and classification before configuration. See Section 3.1 for the structure.

```c
msptiResult msptiPmSamplingGetMetricProperties(msptiPmSamplingGetMetricPropertiesParams *params);
```

- `params.deviceId`: Target device ID.
- `params.metricId`: Metric ID.
- `params.pProperties`: [out] Metric properties.

#### 5.3.3 msptiPmSamplingGetCounterAvailability

Batch-queries the availability of metrics on the target device. SetConfig directly rejects metrics unavailable on the device, so filter through this API before configuration.

```c
msptiResult msptiPmSamplingGetCounterAvailability(msptiPmSamplingGetCounterAvailabilityParams *params);
```

- `params.deviceId`: Target device ID.
- `params.pMetricIds`/`params.numMetrics`: Metric list to query.
- `params.pAvailability`: [out] Availability array of the same length as the metric list; 0 unavailable, 1 available.

### 5.4 Data Retrieval Functions

Samples can be read through two paths, which can be chosen or combined as needed:

- **Counter data image path**: `GetCounterDataSize` → allocate memory → `CounterDataImageInitialize` → `DecodeData` decodes samples into the image, then read by index. Suitable for batch export and persistence scenarios.
- **Sample queue direct-read path**: Pass **NULL** as `pCounterDataImage` of the read APIs to query the internal sample queue directly. Suitable for real-time consumption scenarios.

Samples pass through two buffer levels, the internal sample queue and the image; both levels have overflow awareness means, see the descriptions of `msptiPmSamplingDecodeData` and `msptiPmSamplingGetCounterDataInfo`.

#### 5.4.1 msptiPmSamplingGetCounterDataSize

Computes the number of bytes required by the image. The per-sample entry capacity is derived internally from the metric set; users only specify how many samples the image can hold via `maxSamples`.

Capacity estimation and suggestion: each value entry takes 16 fixed bytes (metricId + instanceId + an 8-byte value), and per-instance metrics are expanded by instance count, so the per-sample size depends on the metric set. Estimate it as "sample header + number of entries x 16 bytes", or divide the byte count returned by this API by `maxSamples` to get the average per-sample size. A `maxSamples` of 10000 is sufficient for common scenarios (aligned with the default sample queue capacity). The upper limit is subject to the 512 MB total image size budget: maxSamples ≤ (512 MB - image directory size) / per-sample size; requests exceeding the limit return `MSPTI_ERROR_INVALID_PARAMETER`. For longer decode intervals, scale the value up by the number of samples produced per decode period.

```c
msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params);
```

- `params.numMetrics`/`params.pMetricIds`: Metric set carried by the image; pass 0/NULL to reuse the metric set configured by SetConfig.
- `params.maxSamples`: Maximum number of samples the image can hold.
- `params.pCounterDataSize`: [out] Required image size in bytes.

#### 5.4.2 msptiPmSamplingCounterDataImageInitialize

Initializes the user-allocated image memory. The image size must match the output of `msptiPmSamplingGetCounterDataSize`, and the memory must be 16-byte aligned.

```c
msptiResult msptiPmSamplingCounterDataImageInitialize(msptiPmSamplingCounterDataImageInitializeParams *params);
```

- `params.pCounterDataImage`: User-allocated image memory.
- `params.counterDataImageSize`: Image size in bytes; returns `MSPTI_ERROR_INVALID_PARAMETER` if it does not match the GetCounterDataSize output.

#### 5.4.3 msptiPmSamplingGetCounterDataInfo

Queries sample statistics of the image or the internal sample queue. If `pCounterDataImage` is NULL, the internal sample queue is queried.

```c
msptiResult msptiPmSamplingGetCounterDataInfo(msptiPmSamplingGetCounterDataInfoParams *params);
```

- `params.pNumTotalSamples`: [out] Total number of sample slots in the image (image path).
- `params.pNumPopulatedSamples`: [out] Number of populated samples; in the direct-read path this is the current queue depth, and periodic polling allows speeding up consumption before overflow occurs.
- `params.pNumCompletedSamples`: [out] Number of completed samples.
- `params.pCounterDataImage`: [in] Optional; pass NULL to query the internal sample queue.

#### 5.4.4 msptiPmSamplingDecodeData

Decodes samples from the internal sample queue into the counter data image. Decoding is incremental and consumes the queue: decoded samples are removed, calls can be incremental, and the same batch of samples is never decoded twice.

```c
msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params);
```

- `params.pCounterDataImage`/`params.counterDataImageSize`: The initialized image and its size in bytes.
- `params.pOverflow`: [out] Whether the queue dropped samples due to overflow since the last decode; the flag is cleared upon reading.
- `params.pDecodeStopReason`: [out] Reason why decoding stopped; `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL` when the image is full, `MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS` when the queue is drained.
- Returns `MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW` when the image capacity is insufficient; samples in the queue are not lost in this case, adjust the capacity and decode again.

#### 5.4.5 msptiPmSamplingGetSampleInfo

Obtains the start and end timestamps of a sample by index (host nanoseconds), used to align with the Activity record timeline.

```c
msptiResult msptiPmSamplingGetSampleInfo(msptiPmSamplingGetSampleInfoParams *params);
```

- `params.sampleIndex`: Sample index.
- `params.pStartTimestamp`/`params.pEndTimestamp`: [out] Sample start and end timestamps.
- `params.pCounterDataImage`: [in] Optional; pass NULL to query the internal sample queue, otherwise read from the specified image.

#### 5.4.6 msptiPmSamplingGetSampleEntries

Obtains the value entry list of a sample by index. The entry count varies with the number of metric instances, so retrieval is two-step: pass NULL as `pEntries` to fill in the entry count only, then allocate a buffer and fetch the entries.

```c
msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params);
```

- `params.sampleIndex`: Sample index.
- `params.pEntries`: [out] Value entry array; pass NULL to query the entry count only.
- `params.maxEntries`: Capacity of `pEntries`, ignored when `pEntries` is NULL.
- `params.pNumEntries`: [out] Actual number of entries of the sample.
- `params.pCounterDataImage`: [in] Optional; pass NULL to query the internal sample queue.

---

## 6. Complete Usage Example

### 6.1 Sample Queue Direct-Read Path

The following example shows the typical workflow: enable a session, configure metrics, start sampling, stop, and read the sample queue directly to print samples.

```cpp
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include "mspti.h"

int main() {
    const uint32_t deviceId = 0;
    // Metric short name table indexed by metricId, filled via msptiPmSamplingGetMetricProperties
    std::vector<std::string> metricNames(MSPTI_PM_SAMPLING_METRIC_ID_COUNT);

    // 1. Create the session object
    msptiPmSamplingEnableParams enableParams = {};
    enableParams.structSize = sizeof(enableParams);
    enableParams.deviceId = deviceId;
    msptiResult ret = msptiPmSamplingEnable(&enableParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Enable failed: %d\n", ret);
        return 1;
    }

    // 2. Query metric availability to filter metrics unsupported on this chip (optional),
    //    and query metric short names for printing
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

    // 3. Configure: metric set + 100ms sampling interval
    msptiPmSamplingSetConfigParams configParams = {};
    configParams.structSize = sizeof(configParams);
    configParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    configParams.numMetrics = static_cast<uint32_t>(metrics.size());
    configParams.pMetricIds = metrics.data();
    configParams.samplingInterval = 100 * 1000 * 1000;  // 100ms, in ns
    configParams.triggerMode = MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL;
    configParams.hardwareBufferSize = 0;  // Use the default size
    configParams.hwBufferAppendMode = MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST;
    ret = msptiPmSamplingSetConfig(&configParams);
    if (ret != MSPTI_SUCCESS) {
        printf("SetConfig failed: %d\n", ret);
        return 1;
    }

    // 4. Start sampling; samples accumulate while the workload runs
    msptiPmSamplingStartParams startParams = {};
    startParams.structSize = sizeof(startParams);
    startParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    ret = msptiPmSamplingStart(&startParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Start failed: %d\n", ret);
        return 1;
    }

    // DoYourWork();  // Execute the workload (ACL computation, etc.). This spans the sampling window; without a time-consuming operation the window is nearly zero-length and no samples may be collected.

    // 5. End the sampling window
    msptiPmSamplingStopParams stopParams = {};
    stopParams.structSize = sizeof(stopParams);
    stopParams.pPmSamplingObject = enableParams.pPmSamplingObject;
    ret = msptiPmSamplingStop(&stopParams);
    if (ret != MSPTI_SUCCESS) {
        printf("Stop failed: %d\n", ret);
        return 1;
    }

    // 6. Read the internal sample queue directly: pass NULL as pCounterDataImage
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

    // 7. Retrieve the time window and value entries of each sample and print them
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

        // Two-step retrieval: query the entry count first, then fetch the entries
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

    // 8. Destroy the object and disable sampling
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

### 6.2 Counter Data Image Path

To export or persist samples in batches, use the image path instead of steps 6–7 of the direct read:

```cpp
// 1. Compute the image size: the per-sample entry capacity is derived
//    internally from the configured metric set; users only decide how many
//    samples the image should hold via maxSamples
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

// 2. Allocate and initialize the image (16-byte aligned)
std::vector<uint8_t> image(imageSize);
msptiPmSamplingCounterDataImageInitializeParams initParams = {};
initParams.structSize = sizeof(initParams);
initParams.pPmSamplingObject = enableParams.pPmSamplingObject;
initParams.counterDataImageSize = imageSize;
initParams.pCounterDataImage = image.data();
msptiPmSamplingCounterDataImageInitialize(&initParams);

// 3. Decode (decoded samples are removed from the internal sample queue; call
//    periodically for incremental decoding. Once the queue is drained,
//    a subsequent call reports END_OF_RECORDS in pDecodeStopReason)
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

// 4. When reading, point pCounterDataImage to the image:
//    set pCounterDataImage of GetSampleInfo/GetSampleEntries to image.data()
```

---

## 7. Complete Sample Reference

See the following sample in the `samples/` directory:

| Sample | Description |
| --- | --- |
| `samples/pm_sampling/` | Basic usage of the PM Sampling API: session enabling, metric configuration, starting sampling, and reading samples |

For details, see [msPTI Sample Descriptions](../../../samples/README.md).
