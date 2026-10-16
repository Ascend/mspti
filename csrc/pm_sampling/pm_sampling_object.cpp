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

#include "csrc/pm_sampling/pm_sampling_object.h"

#include <iterator>

#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/pm_sampling/pm_sampling_metric_table.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
constexpr uint64_t kMinIntervalNs = 1000000ULL;     // 1ms
constexpr uint64_t kMaxIntervalNs = 1000000000ULL;  // 1s
// hardwareBufferSize 未配置（0）时的默认队列样本容量
constexpr size_t kDefaultMaxQueueSamples = 10000;
constexpr size_t kMaxQueueBytes = 512ULL * 1024 * 1024; /* 与镜像上界 PM_IMAGE_MAX_SIZE 对齐 */
constexpr uint32_t kHccsLinkNum = 16;
}  // namespace

static uint32_t WorstEntriesOf(const uint32_t *ids, uint32_t num)
{
    uint32_t worst = 0;
    for (uint32_t i = 0; i < num; ++i)
    {
        // HCCS 指标逐链路上报，单指标 entry 数为链路数（kHccsLinkNum）；其余指标无实例概念
        if (ids[i] == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW || ids[i] == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW)
        {
            worst += kHccsLinkNum;
        }
        else
        {
            worst += 1;
        }
    }
    return worst;
}

static size_t SampleBytesOf(const uint32_t *ids, uint32_t num)
{
    return static_cast<size_t>(PM_SAMPLE_FIXED_SIZE) +
           static_cast<size_t>(WorstEntriesOf(ids, num)) * PM_SAMPLE_ENTRY_SIZE;
}

PmSamplingObject::PmSamplingObject(uint32_t deviceId) : deviceId_(deviceId) {}

msptiResult PmSamplingObject::SetConfig(uint32_t numMetrics, const uint32_t *pMetricIds, uint64_t samplingInterval,
                                        msptiPmSamplingTriggerMode triggerMode, size_t hardwareBufferSize,
                                        msptiPmSamplingBufferAppendMode appendMode)
{
    std::lock_guard<std::mutex> lk(mtx_);
    // Stopped 允许重配置（重配置后旧样本与新 entry 布局不再匹配，清空队列）
    if (state_ != State::Created && state_ != State::Configured && state_ != State::Stopped)
    {
        MSPTI_LOGE("SetConfig is not allowed in state %d, device %u", static_cast<int>(state_), deviceId_);
        return MSPTI_ERROR_INVALID_OPERATION;
    }
    if (numMetrics == 0 || pMetricIds == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (samplingInterval < kMinIntervalNs || samplingInterval > kMaxIntervalNs)
    {
        MSPTI_LOGE("Sampling interval %llu ns is out of [1ms, 1s], device %u",
                   static_cast<unsigned long long>(samplingInterval), deviceId_);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (triggerMode != MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (appendMode != MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST &&
        appendMode != MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (hardwareBufferSize > kMaxQueueBytes)
    {
        MSPTI_LOGE("Queue buffer size %zu bytes exceeds %zu MB, device %u", hardwareBufferSize,
                   kMaxQueueBytes / (1024 * 1024), deviceId_);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (hardwareBufferSize != 0 && hardwareBufferSize < SampleBytesOf(pMetricIds, numMetrics))
    {
        MSPTI_LOGE("Queue buffer size %zu bytes is less than one sample size, device %u", hardwareBufferSize,
                   deviceId_);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    // 指标合法性校验：越界拒绝；芯片不可采的指标拒绝
    for (uint32_t i = 0; i < numMetrics; ++i)
    {
        uint32_t id = pMetricIds[i];
        if (id >= MSPTI_PM_SAMPLING_METRIC_ID_COUNT)
        {
            MSPTI_LOGE("Invalid metric id %u, device %u", id, deviceId_);
            return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
        }
        if (!PmSamplingMetricTable::IsAvailable(id, Common::ContextManager::GetInstance()->GetChipType(deviceId_)))
        {
            MSPTI_LOGE("Metric %u is unavailable on this chip, device %u", id, deviceId_);
            return MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC;
        }
    }
    const bool reconfigured = (state_ == State::Stopped);
    metricIds_.assign(pMetricIds, pMetricIds + numMetrics);
    intervalNs_ = samplingInterval;
    hardwareBufferSize_ = hardwareBufferSize;
    appendMode_ = appendMode;
    if (reconfigured)
    {
        queue_.clear();
        MSPTI_LOGI("PmSampling reconfigured in Stopped state, queue cleared, device %u", deviceId_);
    }
    state_ = State::Configured;
    MSPTI_LOGI("PmSampling SetConfig, device %u, numMetrics %u, intervalNs %llu", deviceId_, numMetrics,
               static_cast<unsigned long long>(samplingInterval));
    return MSPTI_SUCCESS;
}

PmSamplingObject::State PmSamplingObject::GetState() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return state_;
}

msptiResult PmSamplingObject::Start()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (state_ != State::Configured && state_ != State::Stopped)
    {
        MSPTI_LOGE("Start is not allowed in state %d, device %u", static_cast<int>(state_), deviceId_);
        return MSPTI_ERROR_INVALID_OPERATION;
    }
    state_ = State::Running;
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingObject::Stop()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (state_ != State::Running)
    {
        MSPTI_LOGE("Stop is not allowed in state %d, device %u", static_cast<int>(state_), deviceId_);
        return MSPTI_ERROR_INVALID_OPERATION;
    }
    state_ = State::Stopped;
    return MSPTI_SUCCESS;
}

bool PmSamplingObject::IsRunning() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return state_ == State::Running;
}

uint32_t PmSamplingObject::WorstEntriesPerSample() const
{
    return WorstEntriesOf(metricIds_.data(), static_cast<uint32_t>(metricIds_.size()));
}

size_t PmSamplingObject::MaxQueueSamples() const
{
    if (hardwareBufferSize_ == 0)
    {
        return kDefaultMaxQueueSamples;
    }
    return hardwareBufferSize_ / SampleBytesOf(metricIds_.data(), static_cast<uint32_t>(metricIds_.size())) + 1;
}

void PmSamplingObject::EnqueueSample(PmSessionSample &&sample)
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (state_ != State::Running)
    {
        return;
    }
    const size_t maxSamples = MaxQueueSamples();
    if (queue_.size() >= maxSamples)
    {
        overflowed_ = true;
        if (appendMode_ == MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST)
        {
            // 保留最旧：丢弃新样本
            return;
        }
        // 保留最新：弹出最旧
        queue_.pop_front();
    }
    queue_.push_back(std::move(sample));
}

size_t PmSamplingObject::DrainSamples(size_t maxCount, std::vector<PmSessionSample> &out)
{
    std::lock_guard<std::mutex> lk(mtx_);
    const size_t n = std::min(maxCount, queue_.size());
    // 样本取出后即删除，entries 直接移动，免去逐条深拷贝
    out.assign(std::make_move_iterator(queue_.begin()),
               std::make_move_iterator(queue_.begin() + static_cast<std::ptrdiff_t>(n)));
    queue_.erase(queue_.begin(), queue_.begin() + static_cast<std::ptrdiff_t>(n));
    return n;
}

bool PmSamplingObject::GetQueueSample(uint64_t index, PmSessionSample &sample) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (index >= queue_.size())
    {
        return false;
    }
    sample = queue_[static_cast<size_t>(index)];
    return true;
}

uint64_t PmSamplingObject::QueueCount() const
{
    std::lock_guard<std::mutex> lk(mtx_);
    return static_cast<uint64_t>(queue_.size());
}

bool PmSamplingObject::ConsumeOverflow()
{
    std::lock_guard<std::mutex> lk(mtx_);
    const bool ovf = overflowed_;
    overflowed_ = false;
    return ovf;
}

void PmSamplingObject::SetImageInfo(const ImageInfo &imageInfo)
{
    std::lock_guard<std::mutex> lk(mtx_);
    imageInfoValid_ = true;
    imageInfo_ = imageInfo;
}

bool PmSamplingObject::GetImageInfo(ImageInfo &imageInfo) const
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (!imageInfoValid_)
    {
        return false;
    }
    imageInfo = imageInfo_;
    return true;
}

}  // namespace PmSampling
}  // namespace Mspti
