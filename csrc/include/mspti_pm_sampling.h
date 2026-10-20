/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This file is part of the MindStudio project.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *    http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef MSPTI_PM_SAMPLING_H
#define MSPTI_PM_SAMPLING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mspti_result.h"

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(__GNUC__) && defined(MSPTI_LIB)
#pragma GCC visibility push(default)
#endif

    struct msptiPmSamplingObject_st;

    /**
     * @brief A PM sampling session object.
     *
     * Opaque handle created by @ref msptiPmSamplingEnable and destroyed by
     * @ref msptiPmSamplingDisable. Each device allows at most one enabled
     * PM sampling session at a time.
     */
    typedef struct msptiPmSamplingObject_st *msptiPmSamplingObjectPtr;

    /**
     * @brief The append mode of the hardware buffer when it is full.
     */
    typedef enum
    {
        /**
         * Keep the oldest samples and discard new samples.
         */
        MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST = 0,
        /**
         * Keep the latest samples and overwrite the oldest samples.
         */
        MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST = 1,
        MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_COUNT,
        MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_FORCE_INT = 0x7fffffff
    } msptiPmSamplingBufferAppendMode;

    /**
     * @brief The trigger mode of PM sampling.
     */
    typedef enum
    {
        /**
         * Periodic sampling triggered at a fixed time interval.
         */
        MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL = 0,
        MSPTI_PM_SAMPLING_TRIGGER_MODE_COUNT,
        MSPTI_PM_SAMPLING_TRIGGER_MODE_FORCE_INT = 0x7fffffff
    } msptiPmSamplingTriggerMode;

    /**
     * @brief The reason why @ref msptiPmSamplingDecodeData stopped decoding.
     */
    typedef enum
    {
        /**
         * Decoding stopped for a reason other than the ones below.
         */
        MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER = 0,
        /**
         * The counter data image has no free sample slots left.
         */
        MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL,
        /**
         * All records have been decoded.
         */
        MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS,
        MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNT,
        MSPTI_PM_SAMPLING_DECODE_STOP_REASON_FORCE_INT = 0x7fffffff
    } msptiPmSamplingDecodeStopReason;

    /**
     * @brief The data type of a PM sampling metric.
     */
    typedef enum
    {
        /**
         * Dimensionless ratio.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO = 0,
        /**
         * Percentage.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT,
        /**
         * Throughput in bytes/s.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT,
        /**
         * Power in watts.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER,
        /**
         * Temperature in degrees Celsius.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE,
        /**
         * Frequency in MHz.
         */
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY,
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_COUNT,
        MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FORCE_INT = 0x7fffffff
    } msptiPmSamplingMetricDataType;

    /**
     * @brief The group a PM sampling metric belongs to.
     */
    typedef enum
    {
        MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY = 0,
        MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK,
        MSPTI_PM_SAMPLING_METRIC_GROUP_POWER,
        MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL,
        MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION,
        MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY,
        MSPTI_PM_SAMPLING_METRIC_GROUP_COUNT,
        MSPTI_PM_SAMPLING_METRIC_GROUP_FORCE_INT = 0x7fffffff
    } msptiPmSamplingMetricGroup;

    /**
     * @brief Metric IDs of the static metric table.
     *
     * All metrics are collected via DCMI interface point queries.
     * Interconnect bandwidth metrics (HCCS per-link, PCIe, RoCE)
     * are normalized to bytes/s by the collector.
     * MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL is the HBM bandwidth
     * utilization (DCMI exposes HBM bandwidth only as a utilization
     * ratio, not as read/write bytes/s). Whether a metric is
     * supported on the current chip model is reported by
     * @ref msptiPmSamplingGetCounterAvailability.
     */
    typedef enum
    {
        /**
         * Overall NPU utilization in percent.
         * The exact semantics depend on the HDK version.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL = 0,
        /**
         * AI Core utilization in percent.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL = 1,
        /**
         * AI Vector utilization in percent.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL = 2,
        /**
         * AI Cube utilization in percent.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL = 3,
        /**
         * HBM bandwidth utilization in percent.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL = 4,
        /**
         * HBM memory utilization in percent (memory_usage / memory_size,
         * normalized by the collector).
         */
        MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL = 5,
        /**
         * HCCS transmit bandwidth in bytes/s, per link.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW = 6,
        /**
         * HCCS receive bandwidth in bytes/s, per link.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW = 7,
        /**
         * PCIe transmit bandwidth in bytes/s.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW = 8,
        /**
         * PCIe receive bandwidth in bytes/s.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW = 9,
        /**
         * RoCE transmit bandwidth in bytes/s.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW = 10,
        /**
         * RoCE receive bandwidth in bytes/s.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW = 11,
        /**
         * Power in watts.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_POWER = 12,
        /**
         * Temperature in degrees Celsius.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE = 13,
        /**
         * AI Core current frequency in MHz.
         */
        MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ = 14,
        MSPTI_PM_SAMPLING_METRIC_ID_COUNT = 15,
        MSPTI_PM_SAMPLING_METRIC_ID_FORCE_INT = 0x7fffffff
    } msptiPmSamplingMetricId;

    /**
     * @brief Properties of a PM sampling metric.
     */
    typedef struct
    {
        /**
         * Short name of the metric, e.g. "hccs_tx_bw".
         */
        char shortName[64];

        /**
         * Group the metric belongs to.
         */
        msptiPmSamplingMetricGroup metricGroup;

        /**
         * Data type of the metric.
         */
        msptiPmSamplingMetricDataType dataType;

        /**
         * Unit of the metric value, e.g. "bytes/s", "%", "W".
         */
        char unit[32];

        /**
         * Whether the metric is available on the current chip model.
         */
        bool available;
    } msptiPmSamplingMetricProperties;

    /**
     * @brief One self-describing value entry of a PM sampling sample.
     *
     * A sample consists of a list of entries. Each configured metric
     * yields one entry per reported hardware instance (e.g. per HCCS
     * link 0~15); metrics without an instance concept (e.g. power,
     * temperature, HBM bandwidth utilization) always use instanceId 0.
     * A metric with no data in the window yields a single entry with
     * instanceId 0 and value 0.
     */
    typedef struct
    {
        uint32_t metricId;   /* [out] Metric ID, see MSPTI_PM_SAMPLING_METRIC_ID_* */
        uint32_t instanceId; /* [out] Hardware instance ID, 0 if the metric has no instances */
        double value;        /* [out] Metric value */
    } msptiPmSamplingSampleEntry;

    /**
     * @brief Params of @ref msptiPmSamplingEnable.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        uint32_t deviceId;                          /* [in] Target device ID */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [out] The created PM sampling object */
    } msptiPmSamplingEnableParams;

    /**
     * @brief Params of @ref msptiPmSamplingSetConfig.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        uint32_t numMetrics;                        /* [in] Number of metrics to collect */
        const uint32_t *pMetricIds;                 /* [in] Array of metric IDs */
        uint64_t samplingInterval;                  /* [in] Sampling interval in ns, range [1ms, 1s] */
        msptiPmSamplingTriggerMode triggerMode;     /* [in] Trigger mode */
        size_t hardwareBufferSize; /* [in] Internal sample queue capacity in bytes, 0 for default (10000 samples), min
                                      one sample size, max 512MB */
        msptiPmSamplingBufferAppendMode hwBufferAppendMode; /* [in] Append mode when the buffer is full */
    } msptiPmSamplingSetConfigParams;

    /**
     * @brief Params of @ref msptiPmSamplingStart.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
    } msptiPmSamplingStartParams;

    /**
     * @brief Params of @ref msptiPmSamplingStop.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
    } msptiPmSamplingStopParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetNumMetrics.
     */
    typedef struct
    {
        size_t structSize;     /* [in] Size of this struct */
        void *pPriv;           /* [in] Private pointer, passed through as is */
        uint32_t deviceId;     /* [in] Target device ID */
        uint32_t *pNumMetrics; /* [out] Total number of configurable metrics */
    } msptiPmSamplingGetNumMetricsParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetMetricProperties.
     */
    typedef struct
    {
        size_t structSize;                            /* [in] Size of this struct */
        void *pPriv;                                  /* [in] Private pointer, passed through as is */
        uint32_t deviceId;                            /* [in] Target device ID */
        uint32_t metricId;                            /* [in] Metric ID */
        msptiPmSamplingMetricProperties *pProperties; /* [out] Properties of the metric */
    } msptiPmSamplingGetMetricPropertiesParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetCounterDataSize.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        uint32_t numMetrics; /* [in] Number of metrics planned for the image; 0 to reuse the SetConfig metric set */
        const uint32_t *pMetricIds; /* [in] Array of metric IDs; NULL when numMetrics is 0 */
        uint64_t maxSamples;        /* [in] Max number of samples the image can hold */
        size_t *pCounterDataSize;   /* [out] Required image size in bytes */
    } msptiPmSamplingGetCounterDataSizeParams;

    /**
     * @brief Params of @ref msptiPmSamplingCounterDataImageInitialize.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        size_t counterDataImageSize;                /* [in] Image size in bytes, must match GetCounterDataSize output */
        uint8_t *pCounterDataImage;                 /* [in] User-allocated image memory, 16-byte aligned */
    } msptiPmSamplingCounterDataImageInitializeParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetCounterDataInfo.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        uint64_t *pNumTotalSamples;                 /* [out] Total number of sample slots in the image */
        uint64_t *pNumPopulatedSamples;   /* [out] Number of populated samples (queue depth if image is NULL) */
        uint64_t *pNumCompletedSamples;   /* [out] Number of completed samples */
        const uint8_t *pCounterDataImage; /* [in] Optional; query the internal sample queue if NULL */
    } msptiPmSamplingGetCounterDataInfoParams;

    /**
     * @brief Params of @ref msptiPmSamplingDecodeData.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        uint8_t *pCounterDataImage;                 /* [in] Initialized counter data image */
        size_t counterDataImageSize;                /* [in] Image size in bytes */
        bool *pOverflow;                            /* [out] Whether samples were dropped due to buffer overflow */
        msptiPmSamplingDecodeStopReason *pDecodeStopReason; /* [out] Reason why decoding stopped */
    } msptiPmSamplingDecodeDataParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetSampleInfo.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        const uint8_t *pCounterDataImage;           /* [in] Optional; query the internal sample queue if NULL */
        uint64_t sampleIndex;                       /* [in] Sample index */
        uint64_t *pStartTimestamp;                  /* [out] Sample start timestamp in host ns */
        uint64_t *pEndTimestamp;                    /* [out] Sample end timestamp in host ns */
    } msptiPmSamplingGetSampleInfoParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetCounterAvailability.
     */
    typedef struct
    {
        size_t structSize;          /* [in] Size of this struct */
        void *pPriv;                /* [in] Private pointer, passed through as is */
        uint32_t deviceId;          /* [in] Target device ID */
        uint32_t numMetrics;        /* [in] Number of metrics */
        const uint32_t *pMetricIds; /* [in] Array of metric IDs */
        uint8_t *pAvailability;     /* [out] Per-metric availability, 0 unavailable / 1 available */
    } msptiPmSamplingGetCounterAvailabilityParams;

    /**
     * @brief Params of @ref msptiPmSamplingDisable.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
    } msptiPmSamplingDisableParams;

    /**
     * @brief Params of @ref msptiPmSamplingGetSampleEntries.
     */
    typedef struct
    {
        size_t structSize;                          /* [in] Size of this struct */
        void *pPriv;                                /* [in] Private pointer, passed through as is */
        msptiPmSamplingObjectPtr pPmSamplingObject; /* [in] PM sampling object */
        const uint8_t *pCounterDataImage;           /* [in] Optional; query the internal sample queue if NULL */
        uint64_t sampleIndex;                       /* [in] Sample index */
        uint32_t maxEntries;                        /* [in] Capacity of pEntries, ignored if pEntries is NULL */
        msptiPmSamplingSampleEntry *pEntries;       /* [out] Entry array, NULL to only query the entry count */
        uint32_t *pNumEntries;                      /* [out] Actual number of entries of the sample */
    } msptiPmSamplingGetSampleEntriesParams;

    /**
     * @brief Create a PM sampling object and enable PM sampling on the device.
     *
     * Creates the PM sampling object required by later APIs. A device allows
     * at most one enabled PM sampling session at a time. The typical usage
     * is: Enable -> SetConfig -> Start -> (sampling) -> Stop -> decode APIs
     * -> Disable. Start/Stop can be executed repeatedly, and samples of all
     * windows accumulate in the same session; after Stop, SetConfig can be
     * called again to reconfigure and restart.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingEnableParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_DEVICE_OFFLINE if the target device is offline
     * @return MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED if a session already exists on the device
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingEnable(msptiPmSamplingEnableParams *params);

    /**
     * @brief Configure the metric set, sampling interval, and buffer of a
     * PM sampling object.
     *
     * Can be called in Created, Configured, or Stopped state; calling in
     * Running state returns MSPTI_ERROR_INVALID_OPERATION.
     * Reconfiguring a Stopped session clears its pending sample queue,
     * since the metric set may change and old samples do not match the
     * new entry layout.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingSetConfigParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INVALID_OPERATION if the session is Running
     * @return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC if a metric ID is out of range, duplicated, or unavailable on this
     * chip
     * @return MSPTI_ERROR_INVALID_PARAMETER if the interval is out of [1ms, 1s]
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params);

    /**
     * @brief Start the PM sampling window.
     *
     * Can be called in Configured or Stopped state. Samples of all
     * Start/Stop windows accumulate in the same session.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingStartParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INVALID_OPERATION if the session is Running, Created, or has no valid configuration
     * @return MSPTI_ERROR_DEVICE_OFFLINE if the DCMI data source is unavailable on this platform
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingStart(msptiPmSamplingStartParams *params);

    /**
     * @brief Stop the PM sampling window.
     *
     * Can only be called in Running state. Stopping keeps the object,
     * configuration, and already-collected samples, so the session can be
     * restarted or decoded afterwards.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingStopParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INVALID_OPERATION if the session is not Running
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingStop(msptiPmSamplingStopParams *params);

    /**
     * @brief Get the total number of configurable PM sampling metrics.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetNumMetricsParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     */
    msptiResult msptiPmSamplingGetNumMetrics(msptiPmSamplingGetNumMetricsParams *params);

    /**
     * @brief Get the properties of a PM sampling metric.
     *
     * The @p available field of the output reflects whether the metric can
     * be collected on the chip model of the target device.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetMetricPropertiesParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC if the metric ID is out of range
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingGetMetricProperties(msptiPmSamplingGetMetricPropertiesParams *params);

    /**
     * @brief Get the size in bytes required for a counter data image.
     *
     * The counter data image is an opaque user-allocated buffer whose
     * layout is not exposed; it must only be accessed through the APIs of
     * this header. The per-sample entry capacity is derived internally
     * from the metric set, so callers only decide how many samples the
     * image should hold via @p maxSamples. When @p numMetrics is 0 and
     * @p pMetricIds is NULL, the metric set configured by SetConfig is
     * reused. Requests that would exceed the per-image budget are
     * rejected; reduce @p maxSamples and retry.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetCounterDataSizeParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL, fields are
     *         invalid, or the image derived from @p numMetrics/@p maxSamples
     *         would exceed the per-image budget
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INNER if an internal operation fails
     */
    msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params);

    /**
     * @brief Initialize a counter data image for decoding.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingCounterDataImageInitializeParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL, fields are
     *         invalid, or counterDataImageSize does not match the
     *         GetCounterDataSize output
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC if a metric ID is out of range or duplicated
     * @return MSPTI_ERROR_INNER if an internal memory operation fails
     */
    msptiResult msptiPmSamplingCounterDataImageInitialize(msptiPmSamplingCounterDataImageInitializeParams *params);

    /**
     * @brief Get sample statistics of a counter data image.
     *
     * If @p pCounterDataImage is NULL, the statistics of the internal
     * sample queue are returned instead.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetCounterDataInfoParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INNER if an internal memory operation fails
     */
    msptiResult msptiPmSamplingGetCounterDataInfo(msptiPmSamplingGetCounterDataInfoParams *params);

    /**
     * @brief Decode samples into a counter data image.
     *
     * Can be called in Running or Stopped state. Samples written into the
     * image are consumed from the internal sample queue, so each call moves
     * only the samples that have not been decoded before; the image is
     * rewritten from its first slot with the newly consumed samples.
     * Validation failures before queue consumption leave the queue unchanged.
     * Once a batch has been removed from the queue, a subsequent decoding error
     * discards that batch without rollback; retrying cannot recover it. On such
     * failure, the image must not be used as the result of this call, and
     * pOverflow/pDecodeStopReason are not updated. pOverflow reports queue
     * overflow only, not batches discarded due to decoding errors.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingDecodeDataParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if fields or sample data are invalid;
     *         invalid sample data discovered after consumption discards the batch
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW if a consumed batch exceeds
     *         the image capacity; the consumed batch is discarded
     * @return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC if a consumed sample contains
     *         an unconfigured metric ID; the consumed batch is discarded
     * @return MSPTI_ERROR_INNER if an internal operation fails; any already
     *         consumed batch is not restored to the queue
     */
    msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params);

    /**
     * @brief Get the start and end timestamps of a sample by index.
     *
     * If @p pCounterDataImage is NULL, the internal sample queue is
     * queried instead.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetSampleInfoParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INNER if an internal memory operation fails
     */
    msptiResult msptiPmSamplingGetSampleInfo(msptiPmSamplingGetSampleInfoParams *params);

    /**
     * @brief Get the per-metric availability on the target device.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetCounterAvailabilityParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC if a metric ID is out of range
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingGetCounterAvailability(msptiPmSamplingGetCounterAvailabilityParams *params);

    /**
     * @brief Destroy a PM sampling object and disable PM sampling on the
     * device.
     *
     * Can be called in any state. If the session is Running, it is stopped
     * internally (flushing the last sample window) before the object is
     * destroyed.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingDisableParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INNER if an internal operation throws a C++ exception
     */
    msptiResult msptiPmSamplingDisable(msptiPmSamplingDisableParams *params);

    /**
     * @brief Get the value entries of a sample by index.
     *
     * A sample is a list of self-describing entries; each entry carries
     * its metric ID, hardware instance ID, and value, so the entry count
     * of a sample is not fixed and is usually >= the configured metric
     * count. If @p pCounterDataImage is NULL, the engine internal sample
     * queue is queried instead. If @p pEntries is NULL, only
     * @p pNumEntries is filled for a capacity query.
     * If the entry array is too small, its contents are left unchanged and
     * @p pNumEntries reports the required capacity with INVALID_PARAMETER.
     * Other failures leave @p pNumEntries unchanged.
     *
     * Complements @ref msptiPmSamplingGetSampleInfo, which only returns
     * timestamps.
     *
     * @param params [in] Pointer to @ref msptiPmSamplingGetSampleEntriesParams
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p params is NULL or fields are invalid
     * @return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED if @p pPmSamplingObject is invalid
     * @return MSPTI_ERROR_INNER if an internal memory operation fails
     */
    msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params);

#if defined(__GNUC__) && defined(MSPTI_LIB)
#pragma GCC visibility pop
#endif

#if defined(__cplusplus)
}
#endif

#endif
