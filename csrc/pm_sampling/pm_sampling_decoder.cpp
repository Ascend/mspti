/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "csrc/pm_sampling/pm_sampling_decoder.h"

#include <array>
#include <limits>

#include "csrc/common/plog_manager.h"
#include "securec.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
// Field offsets follow counterDataImage v2 in pm_sampling_contract.h.
constexpr size_t POPULATED_OFFSET = 4 * sizeof(uint32_t);
constexpr size_t MAX_SAMPLES_OFFSET = POPULATED_OFFSET + sizeof(uint64_t);
constexpr size_t ENTRY_COUNT_OFFSET = 2 * sizeof(uint64_t);
using MetricSet = std::array<bool, MSPTI_PM_SAMPLING_METRIC_ID_COUNT>;
struct ImageView
{
    uint32_t numMetrics = 0;
    uint32_t maxEntries = 0;
    uint64_t populated = 0;
    uint64_t maxSamples = 0;
    MetricSet metrics{};
};

// Byte copies avoid alignment and object-lifetime assumptions about user memory.
inline bool CheckMemoryResult(errno_t result, const char *operation)
{
    if (result == EOK)
    {
        return true;
    }
    MSPTI_LOGE("PM sampling image %s failed, error: %d.", operation, result);
    return false;
}

template <typename T>
inline bool Read(const uint8_t *address, T &value)
{
    return CheckMemoryResult(memcpy_s(&value, sizeof(value), address, sizeof(value)), "field read");
}
template <typename T>
inline bool Write(uint8_t *image, size_t size, size_t offset, T value)
{
    if (offset > size || sizeof(value) > size - offset)
    {
        MSPTI_LOGE("PM sampling image field write exceeds buffer bounds.");
        return false;
    }
    return CheckMemoryResult(memcpy_s(image + offset, size - offset, &value, sizeof(value)), "field write");
}

inline bool IsAligned(const uint8_t *image) { return image != nullptr && reinterpret_cast<uintptr_t>(image) % 16 == 0; }

msptiResult ValidateImage(const uint8_t *image, size_t size, ImageView &view)
{
    uint32_t magic = 0, version = 0;
    if (!IsAligned(image) || size < PM_IMAGE_HEADER_SIZE)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!Read(image, magic) || !Read(image + sizeof(uint32_t), version))
    {
        return MSPTI_ERROR_INNER;
    }
    if (magic != PM_IMAGE_MAGIC || version != PM_IMAGE_VERSION)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!Read(image + 2 * sizeof(uint32_t), view.numMetrics) || !Read(image + 3 * sizeof(uint32_t), view.maxEntries) ||
        !Read(image + POPULATED_OFFSET, view.populated) || !Read(image + MAX_SAMPLES_OFFSET, view.maxSamples))
    {
        return MSPTI_ERROR_INNER;
    }
    if (!ValidateImageParams(view.numMetrics, view.maxEntries, view.maxSamples) || view.maxEntries < view.numMetrics ||
        view.populated > view.maxSamples || CalcImageSize(view.numMetrics, view.maxEntries, view.maxSamples) != size)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < view.numMetrics; ++i)
    {
        uint32_t id = 0;
        if (!Read(image + PM_IMAGE_HEADER_SIZE + i * sizeof(uint32_t), id))
        {
            return MSPTI_ERROR_INNER;
        }
        if (id >= view.metrics.size() || view.metrics[id])
        {
            return MSPTI_ERROR_INVALID_PARAMETER;
        }
        view.metrics[id] = true;
    }
    return MSPTI_SUCCESS;
}

msptiResult FindSample(const uint8_t *image, const ImageView &view, uint64_t index, const uint8_t *&sample)
{
    if (index >= view.populated)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    sample = image + SampleAreaOffset(view.numMetrics) + index * SampleStride(view.maxEntries);
    uint32_t count = 0;
    uint64_t start = 0, end = 0;
    if (!Read(sample + ENTRY_COUNT_OFFSET, count) || !Read(sample, start) || !Read(sample + sizeof(uint64_t), end))
    {
        return MSPTI_ERROR_INNER;
    }
    if (count > view.maxEntries || start > end)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t id = 0;
        if (!Read(sample + PM_SAMPLE_FIXED_SIZE + i * PM_SAMPLE_ENTRY_SIZE, id))
        {
            return MSPTI_ERROR_INNER;
        }
        if (id >= view.metrics.size() || !view.metrics[id])
        {
            return MSPTI_ERROR_INVALID_PARAMETER;
        }
    }
    return MSPTI_SUCCESS;
}
}  // namespace

msptiResult PmSamplingDecoder::GetSize(uint32_t numMetrics, uint32_t maxEntries, uint64_t maxSamples, size_t &size)
{
    if (!ValidateImageParams(numMetrics, maxEntries, maxSamples) || maxEntries < numMetrics)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    // pm_sampling_contract.h validates maxEntries <= 1024 and bounds maxSamples
    // by (PM_IMAGE_MAX_SIZE - SampleAreaOffset) / SampleStride before multiplication.
    // CalcImageSize then uses uint64_t arithmetic within the 512 MiB image budget;
    // the size_t check below protects the final conversion, not the multiplication.
    const auto required = CalcImageSize(numMetrics, maxEntries, maxSamples);
    if (required > std::numeric_limits<size_t>::max())
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    size = static_cast<size_t>(required);
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingDecoder::Initialize(uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds,
                                          uint32_t maxEntries, uint64_t maxSamples)
{
    size_t required = 0;
    if (!IsAligned(image) || metricIds.size() > MSPTI_PM_SAMPLING_METRIC_ID_COUNT ||
        GetSize(static_cast<uint32_t>(metricIds.size()), maxEntries, maxSamples, required) != MSPTI_SUCCESS ||
        size != required)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    MetricSet seen{};
    for (auto id : metricIds)
    {
        if (id >= seen.size() || seen[id])
        {
            return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
        }
        seen[id] = true;
    }
    if (!CheckMemoryResult(memset_s(image, size, 0, size), "initialization"))
    {
        return MSPTI_ERROR_INNER;
    }
    // Publish the magic only after every field and the directory have been copied.
    if (!Write(image, size, sizeof(uint32_t), PM_IMAGE_VERSION) ||
        !Write(image, size, 2 * sizeof(uint32_t), static_cast<uint32_t>(metricIds.size())) ||
        !Write(image, size, 3 * sizeof(uint32_t), maxEntries) || !Write(image, size, MAX_SAMPLES_OFFSET, maxSamples) ||
        !CheckMemoryResult(memcpy_s(image + PM_IMAGE_HEADER_SIZE, size - PM_IMAGE_HEADER_SIZE, metricIds.data(),
                                    metricIds.size() * sizeof(uint32_t)),
                           "metric directory write") ||
        !Write(image, size, 0, PM_IMAGE_MAGIC))
    {
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingDecoder::CheckPlan(const uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds,
                                         uint32_t maxEntries, uint64_t maxSamples)
{
    ImageView view;
    const auto result = ValidateImage(image, size, view);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    if (view.numMetrics != metricIds.size() || view.maxEntries != maxEntries || view.maxSamples != maxSamples)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    for (size_t i = 0; i < metricIds.size(); ++i)
    {
        uint32_t id = 0;
        if (!Read(image + PM_IMAGE_HEADER_SIZE + i * sizeof(uint32_t), id))
        {
            return MSPTI_ERROR_INNER;
        }
        if (id != metricIds[i])
        {
            return MSPTI_ERROR_INVALID_PARAMETER;
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingDecoder::WriteSamples(uint8_t *image, size_t size, const std::vector<PmSessionSample> &samples)
{
    ImageView view;
    const auto result = ValidateImage(image, size, view);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    if (samples.size() > view.maxSamples)
    {
        return MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW;
    }
    for (const auto &sample : samples)
    {
        if (sample.entries.size() > view.maxEntries)
        {
            return MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW;
        }
        if (sample.startNs > sample.endNs)
        {
            return MSPTI_ERROR_INVALID_PARAMETER;
        }
        for (const auto &entry : sample.entries)
        {
            if (entry.metricId >= view.metrics.size() || !view.metrics[entry.metricId])
            {
                return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
            }
        }
    }
    const auto offset = SampleAreaOffset(view.numMetrics);
    // Do not expose a partially rewritten batch if a checked copy fails.
    if (!Write(image, size, POPULATED_OFFSET, uint64_t{0}))
    {
        return MSPTI_ERROR_INNER;
    }
    if (!CheckMemoryResult(memset_s(image + offset, size - offset, 0, size - offset), "sample area clearing"))
    {
        return MSPTI_ERROR_INNER;
    }
    size_t target = offset;
    for (const auto &sample : samples)
    {
        if (!Write(image, size, target, sample.startNs) ||
            !Write(image, size, target + sizeof(uint64_t), sample.endNs) ||
            !Write(image, size, target + ENTRY_COUNT_OFFSET, static_cast<uint32_t>(sample.entries.size())))
        {
            return MSPTI_ERROR_INNER;
        }
        size_t entryData = target + PM_SAMPLE_FIXED_SIZE;
        for (const auto &entry : sample.entries)
        {
            if (!Write(image, size, entryData, entry.metricId) ||
                !Write(image, size, entryData + sizeof(uint32_t), entry.instanceId) ||
                !Write(image, size, entryData + 2 * sizeof(uint32_t), entry.value))
            {
                return MSPTI_ERROR_INNER;
            }
            entryData += PM_SAMPLE_ENTRY_SIZE;
        }
        target += SampleStride(view.maxEntries);
    }
    return Write(image, size, POPULATED_OFFSET, static_cast<uint64_t>(samples.size())) ? MSPTI_SUCCESS
                                                                                       : MSPTI_ERROR_INNER;
}

msptiResult PmSamplingDecoder::GetInfo(const uint8_t *image, size_t size, uint64_t &total, uint64_t &populated,
                                       uint64_t &completed)
{
    ImageView view;
    const auto result = ValidateImage(image, size, view);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    total = view.maxSamples;
    populated = view.populated;
    completed = view.populated;  // Only completed session samples are written.
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingDecoder::GetSampleInfo(const uint8_t *image, size_t size, uint64_t index, uint64_t &start,
                                             uint64_t &end)
{
    ImageView view;
    auto result = ValidateImage(image, size, view);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    const uint8_t *sample = nullptr;
    result = FindSample(image, view, index, sample);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    uint64_t sampleStart = 0, sampleEnd = 0;
    if (!Read(sample, sampleStart) || !Read(sample + sizeof(uint64_t), sampleEnd))
    {
        return MSPTI_ERROR_INNER;
    }
    start = sampleStart;
    end = sampleEnd;
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingDecoder::GetSampleEntries(const uint8_t *image, size_t size, uint64_t index, uint32_t capacity,
                                                msptiPmSamplingSampleEntry *entries, uint32_t &count)
{
    ImageView view;
    auto result = ValidateImage(image, size, view);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    const uint8_t *sample = nullptr;
    result = FindSample(image, view, index, sample);
    if (result != MSPTI_SUCCESS)
    {
        return result;
    }
    uint32_t required = 0;
    if (!Read(sample + ENTRY_COUNT_OFFSET, required))
    {
        return MSPTI_ERROR_INNER;
    }
    if (entries == nullptr)
    {
        count = required;
        return MSPTI_SUCCESS;
    }
    if (capacity < required)
    {
        count = required;
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    const auto *entryData = sample + PM_SAMPLE_FIXED_SIZE;
    for (uint32_t i = 0; i < required; ++i)
    {
        msptiPmSamplingSampleEntry entry{};
        if (!Read(entryData, entry.metricId) || !Read(entryData + sizeof(uint32_t), entry.instanceId) ||
            !Read(entryData + 2 * sizeof(uint32_t), entry.value))
        {
            return MSPTI_ERROR_INNER;
        }
        entries[i] = entry;
        entryData += PM_SAMPLE_ENTRY_SIZE;
    }
    count = required;
    return MSPTI_SUCCESS;
}
}  // namespace PmSampling
}  // namespace Mspti
