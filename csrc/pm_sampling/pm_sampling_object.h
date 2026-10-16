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
 * @file  pm_sampling_object.h
 * @brief PM Sampling 会话对象：状态机（Created→Configured→Running→Stopped）+
 *        会话样本队列 + counterDataImage 镜像计划。
 *
 * 单设备单会话（唯一性由 PmSamplingManager 保证）。样本队列由采集链路
 * （PmSamplingParser 产出的窗口样本）填充，由 DecodeData 批量读入镜像。
 */

#ifndef MSPTI_PM_SAMPLING_PM_SAMPLING_OBJECT_H
#define MSPTI_PM_SAMPLING_PM_SAMPLING_OBJECT_H

#include <deque>
#include <mutex>
#include <vector>

#include "csrc/include/mspti_pm_sampling.h"
#include "csrc/pm_sampling/pm_sampling_contract.h"

namespace Mspti
{
namespace PmSampling
{

class PmSamplingObject
{
   public:
    enum class State
    {
        Created = 0,
        Configured,
        Running,
        Stopped,
    };

    // 一次镜像申请的参数记录：GetCounterDataSize 时存入，CounterDataImageInitialize/DecodeData 复用
    struct ImageInfo
    {
        std::vector<uint32_t> metricIds;
        uint32_t maxEntriesPerSample = 0;
        uint64_t maxSamples = 0;
        size_t imageSize = 0;
    };

    explicit PmSamplingObject(uint32_t deviceId);

    uint32_t DeviceId() const { return deviceId_; }

    // ---- 配置（Created / Configured / Stopped 状态可调；Stopped 重配置清空样本队列）----
    msptiResult SetConfig(uint32_t numMetrics, const uint32_t *pMetricIds, uint64_t samplingInterval,
                          msptiPmSamplingTriggerMode triggerMode, size_t hardwareBufferSize,
                          msptiPmSamplingBufferAppendMode appendMode);

    const std::vector<uint32_t> &MetricIds() const { return metricIds_; }
    uint32_t NumMetrics() const { return static_cast<uint32_t>(metricIds_.size()); }
    uint64_t IntervalNs() const { return intervalNs_; }

    // ---- 状态机 ----
    State GetState() const;
    msptiResult Start();  // Configured / Stopped -> Running；Running 时报 INVALID_OPERATION
    msptiResult Stop();   // Running -> Stopped；非 Running 时报 INVALID_OPERATION
    bool IsRunning() const;

    // ---- 样本队列 ----
    // 采集链路产出窗口样本（entries 逐实例一条）。仅 Running 时入队，
    // 队列满时按 appendMode 处理并置溢出标志。
    void EnqueueSample(PmSessionSample &&sample);
    // 解析侧唯一消费入口：锁内拷出队头至多 maxCount 个样本并从队列删除，返回实际取走数
    size_t DrainSamples(size_t maxCount, std::vector<PmSessionSample> &out);
    // 锁内按索引读取单个队列样本（直读路径，不消费）
    bool GetQueueSample(uint64_t index, PmSessionSample &sample) const;
    uint64_t QueueCount() const;
    // 是否发生过溢出丢弃（读取即清除）
    bool ConsumeOverflow();

    // ---- counterDataImage 镜像计划 ----
    // GetCounterDataSize 时记录（含指标集、容量与镜像大小）；已存在则覆盖
    void SetImageInfo(const ImageInfo &imageInfo);
    // 无有效计划返回 false
    bool GetImageInfo(ImageInfo &imageInfo) const;

    // 单样本最坏 entry 数（HCCS 指标按 16 链路计），队列容量估算用
    uint32_t WorstEntriesPerSample() const;

   private:
    size_t MaxQueueSamples() const;

    uint32_t deviceId_;
    mutable std::mutex mtx_;
    State state_ = State::Created;
    std::vector<uint32_t> metricIds_;
    uint64_t intervalNs_ = 0;
    size_t hardwareBufferSize_ = 0;
    msptiPmSamplingBufferAppendMode appendMode_ = MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST;
    std::deque<PmSessionSample> queue_;
    bool overflowed_ = false;
    bool imageInfoValid_ = false;
    ImageInfo imageInfo_;
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_PM_SAMPLING_OBJECT_H
