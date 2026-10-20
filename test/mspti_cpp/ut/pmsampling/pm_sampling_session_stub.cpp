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

#include <algorithm>
#include <new>
#include <utility>

#include "csrc/common/context_manager.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"

// Minimal single-threaded substitute for isolated result API tests.
// No collector locking/lifecycle behavior is implemented or verified here.
namespace Mspti
{
namespace PmSampling
{
PmSamplingManager *PmSamplingManager::GetInstance()
{
    static PmSamplingManager manager;
    return &manager;
}
// Lifecycle calls deliberately fail here: exercise the C API exception boundary
// without starting collectors or pretending to implement their lifecycle.
msptiResult PmSamplingManager::Enable(uint32_t, msptiPmSamplingObjectPtr *) { throw std::bad_alloc(); }
msptiResult PmSamplingManager::Disable(msptiPmSamplingObjectPtr) { throw std::bad_alloc(); }
msptiResult PmSamplingManager::StartCollectors(PmSamplingObject &) { throw std::bad_alloc(); }
void PmSamplingManager::StopCollectors(PmSamplingObject &) { throw std::bad_alloc(); }
msptiResult PmSamplingObject::SetConfig(uint32_t, const uint32_t *, uint64_t, msptiPmSamplingTriggerMode, size_t,
                                        msptiPmSamplingBufferAppendMode)
{
    throw std::bad_alloc();
}
std::shared_ptr<PmSamplingObject> PmSamplingManager::GetLiveObject(msptiPmSamplingObjectPtr handle)
{
    for (const auto &entry : objects_)
    {
        if (reinterpret_cast<msptiPmSamplingObjectPtr>(entry.second.get()) == handle) return entry.second;
    }
    return nullptr;
}
PmSamplingObject::PmSamplingObject(uint32_t deviceId) : deviceId_(deviceId) {}
PmSamplingObject::State PmSamplingObject::GetState() const { return state_; }
uint64_t PmSamplingObject::QueueCount() const { return queue_.size(); }
bool PmSamplingObject::ConsumeOverflow() { return std::exchange(overflowed_, false); }

uint32_t PmSamplingObject::WorstEntriesPerSample() const
{
    uint32_t count = 0;
    for (auto id : metricIds_)
    {
        count +=
            (id == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW || id == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW) ? 16 : 1;
    }
    return count;
}
void PmSamplingObject::SetImageInfo(const ImageInfo &info)
{
    imageInfo_ = info;
    imageInfoValid_ = true;
}
bool PmSamplingObject::GetImageInfo(ImageInfo &info) const
{
    if (!imageInfoValid_) return false;
    info = imageInfo_;
    return true;
}
size_t PmSamplingObject::DrainSamples(size_t maxCount, std::vector<PmSessionSample> &out)
{
    const auto count = std::min(maxCount, queue_.size());
    out.assign(queue_.begin(), queue_.begin() + count);
    queue_.erase(queue_.begin(), queue_.begin() + count);
    return count;
}
bool PmSamplingObject::GetQueueSample(uint64_t index, PmSessionSample &sample) const
{
    if (index >= queue_.size()) return false;
    sample = queue_[static_cast<size_t>(index)];
    return true;
}
}  // namespace PmSampling

namespace Common
{
// Only metadata queries use this fixed chip type; no hardware is contacted.
ContextManager *ContextManager::GetInstance()
{
    static ContextManager manager;
    return &manager;
}
ContextManager::~ContextManager() = default;
PlatformType ContextManager::GetChipType(uint32_t) { return PlatformType::CHIP_910B; }
}  // namespace Common
}  // namespace Mspti
