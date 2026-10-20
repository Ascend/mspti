/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of the Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

/**
 * @file  mspti_pm_sampling_api.cpp
 * @brief PM Sampling 对象式 C API 实现。
 * 每个入口统一做 params/structSize 前向兼容校验后转发 manager/object/decoder。
 */

#include <algorithm>
#include <array>
#include <limits>

#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/include/mspti_pm_sampling.h"
#include "csrc/pm_sampling/pm_sampling_contract.h"
#include "csrc/pm_sampling/pm_sampling_decoder.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"
#include "csrc/pm_sampling/pm_sampling_metric_table.h"
#include "csrc/pm_sampling/pm_sampling_object.h"
#include "securec.h"

using Mspti::PmSampling::PmSamplingManager;
using Mspti::PmSampling::PmSamplingMetricTable;
using Mspti::PmSampling::PmSamplingObject;

namespace
{

// params 前向兼容：structSize 必须覆盖编译期已知字段
template <typename T>
bool CheckParams(const T *params)
{
    return params != nullptr && params->structSize >= sizeof(T);
}

using namespace Mspti::PmSampling;
using ImageInfo = PmSamplingObject::ImageInfo;

// All public PM Sampling C entry points share the same exception boundary.
template <typename Params, typename Operation>
msptiResult WithApi(Params *params, Operation operation)
{
    if (!CheckParams(params))
    {
        return MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT;
    }
    try
    {
        return operation();
    }
    catch (...)
    {
        // No C++ exception may cross a public C entry point.
        MSPTI_LOGE("PM sampling API failed with an internal exception.");
        return MSPTI_ERROR_INNER;
    }
}

// Hold the shared session until the operation completes, including during Disable.
// SetConfig and image access on the same session still require caller serialization.
// Queue methods synchronize sampling.
template <typename Params, typename Operation>
msptiResult WithSession(Params *params, Operation operation)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       auto *manager = PmSamplingManager::GetInstance();
                       auto object = manager == nullptr ? nullptr : manager->GetLiveObject(params->pPmSamplingObject);
                       if (object == nullptr)
                       {
                           return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED;
                       }
                       return operation(*object);
                   });
}

msptiResult CheckMetrics(const PmSamplingObject &object, const std::vector<uint32_t> &ids, uint32_t maxEntries)
{
    const auto &configured = object.MetricIds();
    if (configured.empty())
    {
        return MSPTI_ERROR_INVALID_OPERATION;
    }
    std::array<bool, MSPTI_PM_SAMPLING_METRIC_ID_COUNT> seen{};
    for (auto id : ids)
    {
        if (id >= seen.size() || seen[id])
        {
            return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
        }
        seen[id] = true;
    }
    if (ids.size() != configured.size() || maxEntries < object.WorstEntriesPerSample())
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    for (auto id : configured)
    {
        if (id >= seen.size() || !seen[id])
        {
            return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult GetPlan(const PmSamplingObject &object, ImageInfo &plan)
{
    size_t size = 0;
    if (!object.GetImageInfo(plan) || plan.metricIds.size() > MSPTI_PM_SAMPLING_METRIC_ID_COUNT ||
        PmSamplingDecoder::GetSize(static_cast<uint32_t>(plan.metricIds.size()), plan.maxEntriesPerSample,
                                   plan.maxSamples, size) != MSPTI_SUCCESS ||
        size != plan.imageSize)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    return MSPTI_SUCCESS;
}

msptiResult CheckImage(const PmSamplingObject &object, const uint8_t *image, ImageInfo &plan)
{
    if (GetPlan(object, plan) != MSPTI_SUCCESS)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    // Queries have no size input. Callers must retain the initialized allocation;
    // an old, unchanged header with a different layout is rejected before payload reads.
    return PmSamplingDecoder::CheckPlan(image, plan.imageSize, plan.metricIds, plan.maxEntriesPerSample,
                                        plan.maxSamples);
}
}  // namespace

extern "C" msptiResult msptiPmSamplingEnable(msptiPmSamplingEnableParams *params)
{
    return WithApi(params, [&]() -> msptiResult
                   { return PmSamplingManager::GetInstance()->Enable(params->deviceId, &params->pPmSamplingObject); });
}

extern "C" msptiResult msptiPmSamplingSetConfig(msptiPmSamplingSetConfigParams *params)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       auto obj = PmSamplingManager::GetInstance()->GetLiveObject(params->pPmSamplingObject);
                       if (obj == nullptr)
                       {
                           return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED;
                       }
                       return obj->SetConfig(params->numMetrics, params->pMetricIds, params->samplingInterval,
                                             params->triggerMode, params->hardwareBufferSize,
                                             params->hwBufferAppendMode);
                   });
}

extern "C" msptiResult msptiPmSamplingStart(msptiPmSamplingStartParams *params)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       auto obj = PmSamplingManager::GetInstance()->GetLiveObject(params->pPmSamplingObject);
                       if (obj == nullptr)
                       {
                           return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED;
                       }
                       return PmSamplingManager::GetInstance()->StartCollectors(*obj);
                   });
}

extern "C" msptiResult msptiPmSamplingStop(msptiPmSamplingStopParams *params)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       auto obj = PmSamplingManager::GetInstance()->GetLiveObject(params->pPmSamplingObject);
                       if (obj == nullptr)
                       {
                           return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED;
                       }
                       if (obj->GetState() != PmSamplingObject::State::Running)
                       {
                           // Stop 仅允许在 Running 状态调用（Disable 路径内部停采集不受此限制）
                           return MSPTI_ERROR_INVALID_OPERATION;
                       }
                       PmSamplingManager::GetInstance()->StopCollectors(*obj);
                       return MSPTI_SUCCESS;
                   });
}

extern "C" msptiResult msptiPmSamplingGetNumMetrics(msptiPmSamplingGetNumMetricsParams *params)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       if (params->pNumMetrics == nullptr)
                       {
                           return MSPTI_ERROR_INVALID_PARAMETER;
                       }
                       *params->pNumMetrics = MSPTI_PM_SAMPLING_METRIC_ID_COUNT;
                       return MSPTI_SUCCESS;
                   });
}

extern "C" msptiResult msptiPmSamplingGetMetricProperties(msptiPmSamplingGetMetricPropertiesParams *params)
{
    return WithApi(params,
                   [&]() -> msptiResult
                   {
                       if (params->pProperties == nullptr)
                       {
                           return MSPTI_ERROR_INVALID_PARAMETER;
                       }
                       if (params->metricId >= MSPTI_PM_SAMPLING_METRIC_ID_COUNT)
                       {
                           return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
                       }
                       const auto &info = PmSamplingMetricTable::Get(params->metricId);
                       if (snprintf_s(params->pProperties->shortName, sizeof(params->pProperties->shortName),
                                      sizeof(params->pProperties->shortName) - 1, "%s", info.shortName) < 0)
                       {
                           return MSPTI_ERROR_INNER;
                       }
                       if (snprintf_s(params->pProperties->unit, sizeof(params->pProperties->unit),
                                      sizeof(params->pProperties->unit) - 1, "%s", info.unit) < 0)
                       {
                           return MSPTI_ERROR_INNER;
                       }
                       params->pProperties->metricGroup = info.group;
                       params->pProperties->dataType = info.dataType;
                       params->pProperties->available = PmSamplingMetricTable::IsAvailable(
                           params->metricId,
                           Mspti::Common::ContextManager::GetInstance()->GetChipType(params->deviceId));
                       return MSPTI_SUCCESS;
                   });
}

extern "C" msptiResult msptiPmSamplingGetCounterDataSize(msptiPmSamplingGetCounterDataSizeParams *params)
{
    return WithSession(params,
                       [&](PmSamplingObject &object) -> msptiResult
                       {
                           size_t size = 0;
                           if (params->pCounterDataSize == nullptr ||
                               params->numMetrics > MSPTI_PM_SAMPLING_METRIC_ID_COUNT ||
                               (params->numMetrics == 0) != (params->pMetricIds == nullptr))
                           {
                               return MSPTI_ERROR_INVALID_PARAMETER;
                           }
                           ImageInfo plan;
                           if (params->numMetrics == 0)
                           {
                               plan.metricIds = object.MetricIds();
                           }
                           else
                           {
                               plan.metricIds.assign(params->pMetricIds, params->pMetricIds + params->numMetrics);
                           }
                           plan.maxEntriesPerSample = object.WorstEntriesPerSample();
                           auto result = CheckMetrics(object, plan.metricIds, plan.maxEntriesPerSample);
                           if (result != MSPTI_SUCCESS)
                           {
                               return result;
                           }
                           result = PmSamplingDecoder::GetSize(static_cast<uint32_t>(plan.metricIds.size()),
                                                               plan.maxEntriesPerSample, params->maxSamples, size);
                           if (result != MSPTI_SUCCESS)
                           {
                               return result;
                           }
                           plan.maxSamples = params->maxSamples;
                           plan.imageSize = size;
                           object.SetImageInfo(plan);
                           *params->pCounterDataSize = size;
                           return MSPTI_SUCCESS;
                       });
}

extern "C" msptiResult msptiPmSamplingCounterDataImageInitialize(
    msptiPmSamplingCounterDataImageInitializeParams *params)
{
    return WithSession(params,
                       [&](PmSamplingObject &object) -> msptiResult
                       {
                           ImageInfo plan;
                           if (GetPlan(object, plan) != MSPTI_SUCCESS || params->counterDataImageSize != plan.imageSize)
                           {
                               return MSPTI_ERROR_INVALID_PARAMETER;
                           }
                           auto result = CheckMetrics(object, plan.metricIds, plan.maxEntriesPerSample);
                           return result != MSPTI_SUCCESS
                                      ? result
                                      : PmSamplingDecoder::Initialize(params->pCounterDataImage, plan.imageSize,
                                                                      plan.metricIds, plan.maxEntriesPerSample,
                                                                      plan.maxSamples);
                       });
}

extern "C" msptiResult msptiPmSamplingGetCounterDataInfo(msptiPmSamplingGetCounterDataInfoParams *params)
{
    return WithSession(params,
                       [&](PmSamplingObject &object) -> msptiResult
                       {
                           if (params->pNumTotalSamples == nullptr || params->pNumPopulatedSamples == nullptr ||
                               params->pNumCompletedSamples == nullptr)
                           {
                               return MSPTI_ERROR_INVALID_PARAMETER;
                           }
                           if (params->pCounterDataImage == nullptr)
                           {
                               // No fixed image slots in the direct queue view: report its current completed sample
                               // count.
                               const auto count = object.QueueCount();
                               *params->pNumTotalSamples = count;
                               *params->pNumPopulatedSamples = count;
                               *params->pNumCompletedSamples = count;
                               return MSPTI_SUCCESS;
                           }
                           ImageInfo plan;
                           auto result = CheckImage(object, params->pCounterDataImage, plan);
                           return result != MSPTI_SUCCESS
                                      ? result
                                      : PmSamplingDecoder::GetInfo(
                                            params->pCounterDataImage, plan.imageSize, *params->pNumTotalSamples,
                                            *params->pNumPopulatedSamples, *params->pNumCompletedSamples);
                       });
}

extern "C" msptiResult msptiPmSamplingDecodeData(msptiPmSamplingDecodeDataParams *params)
{
    return WithSession(
        params,
        [&](PmSamplingObject &object) -> msptiResult
        {
            if (params->pOverflow == nullptr || params->pDecodeStopReason == nullptr)
            {
                return MSPTI_ERROR_INVALID_PARAMETER;
            }
            const auto state = object.GetState();
            if (state != PmSamplingObject::State::Running && state != PmSamplingObject::State::Stopped)
            {
                return MSPTI_ERROR_INVALID_OPERATION;
            }
            ImageInfo plan;
            // Check the explicit accessible size before reading any bytes from the image.
            if (GetPlan(object, plan) != MSPTI_SUCCESS || params->counterDataImageSize != plan.imageSize)
            {
                return MSPTI_ERROR_INVALID_PARAMETER;
            }
            auto result = PmSamplingDecoder::CheckPlan(params->pCounterDataImage, plan.imageSize, plan.metricIds,
                                                       plan.maxEntriesPerSample, plan.maxSamples);
            if (result != MSPTI_SUCCESS)
            {
                return result;
            }
            result = CheckMetrics(object, plan.metricIds, plan.maxEntriesPerSample);
            if (result != MSPTI_SUCCESS)
            {
                return result;
            }
            std::vector<PmSessionSample> samples;
            samples.reserve(static_cast<size_t>(plan.maxSamples));  // Allocate before destructive consumption.
            const auto count = object.DrainSamples(static_cast<size_t>(plan.maxSamples), samples);
            if (count != samples.size())
            {
                MSPTI_LOGE("PM sampling queue returned an inconsistent sample count.");
                return MSPTI_ERROR_INNER;
            }
            // Collector contract: complete, bounded samples matching the configured metrics.
            // DrainSamples must be exception-safe. This API cannot roll back malformed producer data.
            result = PmSamplingDecoder::WriteSamples(params->pCounterDataImage, plan.imageSize, samples);
            if (result != MSPTI_SUCCESS)
            {
                return result;
            }
            *params->pOverflow = object.ConsumeOverflow();
            *params->pDecodeStopReason = count == plan.maxSamples
                                             ? MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL
                                             : MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS;
            return MSPTI_SUCCESS;
        });
}

extern "C" msptiResult msptiPmSamplingGetSampleInfo(msptiPmSamplingGetSampleInfoParams *params)
{
    return WithSession(params,
                       [&](PmSamplingObject &object) -> msptiResult
                       {
                           if (params->pStartTimestamp == nullptr || params->pEndTimestamp == nullptr)
                           {
                               return MSPTI_ERROR_INVALID_PARAMETER;
                           }
                           if (params->pCounterDataImage == nullptr)
                           {
                               PmSessionSample sample;
                               if (!object.GetQueueSample(params->sampleIndex, sample) || sample.startNs > sample.endNs)
                               {
                                   return MSPTI_ERROR_INVALID_PARAMETER;
                               }
                               *params->pStartTimestamp = sample.startNs;
                               *params->pEndTimestamp = sample.endNs;
                               return MSPTI_SUCCESS;
                           }
                           ImageInfo plan;
                           auto result = CheckImage(object, params->pCounterDataImage, plan);
                           return result != MSPTI_SUCCESS
                                      ? result
                                      : PmSamplingDecoder::GetSampleInfo(params->pCounterDataImage, plan.imageSize,
                                                                         params->sampleIndex, *params->pStartTimestamp,
                                                                         *params->pEndTimestamp);
                       });
}

extern "C" msptiResult msptiPmSamplingGetCounterAvailability(msptiPmSamplingGetCounterAvailabilityParams *params)
{
    return WithApi(
        params,
        [&]() -> msptiResult
        {
            if (params->pAvailability == nullptr || (params->numMetrics != 0 && params->pMetricIds == nullptr))
            {
                return MSPTI_ERROR_INVALID_PARAMETER;
            }
            const auto chipType = Mspti::Common::ContextManager::GetInstance()->GetChipType(params->deviceId);
            for (uint32_t i = 0; i < params->numMetrics; ++i)
            {
                if (params->pMetricIds[i] >= MSPTI_PM_SAMPLING_METRIC_ID_COUNT)
                {
                    return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
                }
                params->pAvailability[i] = PmSamplingMetricTable::IsAvailable(params->pMetricIds[i], chipType) ? 1 : 0;
            }
            return MSPTI_SUCCESS;
        });
}

extern "C" msptiResult msptiPmSamplingDisable(msptiPmSamplingDisableParams *params)
{
    return WithApi(
        params, [&]() -> msptiResult { return PmSamplingManager::GetInstance()->Disable(params->pPmSamplingObject); });
}

extern "C" msptiResult msptiPmSamplingGetSampleEntries(msptiPmSamplingGetSampleEntriesParams *params)
{
    return WithSession(params,
                       [&](PmSamplingObject &object) -> msptiResult
                       {
                           if (params->pNumEntries == nullptr)
                           {
                               return MSPTI_ERROR_INVALID_PARAMETER;
                           }
                           if (params->pCounterDataImage == nullptr)
                           {
                               PmSessionSample sample;
                               if (!object.GetQueueSample(params->sampleIndex, sample) ||
                                   sample.entries.size() > std::numeric_limits<uint32_t>::max())
                               {
                                   return MSPTI_ERROR_INVALID_PARAMETER;
                               }
                               const auto count = static_cast<uint32_t>(sample.entries.size());
                               if (params->pEntries == nullptr)
                               {
                                   *params->pNumEntries = count;
                                   return MSPTI_SUCCESS;
                               }
                               if (params->maxEntries < sample.entries.size())
                               {
                                   *params->pNumEntries = count;
                                   return MSPTI_ERROR_INVALID_PARAMETER;
                               }
                               std::transform(sample.entries.begin(), sample.entries.end(), params->pEntries,
                                              [](const PmSampleEntry &e) {
                                                  return msptiPmSamplingSampleEntry{e.metricId, e.instanceId, e.value};
                                              });
                               *params->pNumEntries = count;
                               return MSPTI_SUCCESS;
                           }
                           ImageInfo plan;
                           auto result = CheckImage(object, params->pCounterDataImage, plan);
                           return result != MSPTI_SUCCESS
                                      ? result
                                      : PmSamplingDecoder::GetSampleEntries(params->pCounterDataImage, plan.imageSize,
                                                                            params->sampleIndex, params->maxEntries,
                                                                            params->pEntries, *params->pNumEntries);
                       });
}
