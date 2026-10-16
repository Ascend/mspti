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

#include "csrc/pm_sampling/pm_sampling_parser.h"

#include "csrc/common/plog_manager.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
constexpr uint32_t kUtilFreqSentinel = 0xFFFFFFFFU;
constexpr int32_t kIntSentinel = -1;
}  // namespace

PmSamplingParser::PmSamplingParser(const std::vector<uint32_t> &metricIds) : metricIds_(metricIds) {}

void PmSamplingParser::OnDcmiSample(const PmDcmiSample &sample)
{
    if (!hasPrev_)
    {
        prev_ = sample;
        hasPrev_ = true;
        return;
    }
    if (sample.tsNs < prev_.tsNs)
    {
        // 时间倒退（时钟校准等）：重置窗口基准，避免负窗口
        MSPTI_LOGW("PmSampling parser: timestamp fallback, reset window base");
        prev_ = sample;
        return;
    }
    EmitWindow(sample);
    prev_ = sample;
}

void PmSamplingParser::EmitWindow(const PmDcmiSample &cur)
{
    PmSessionSample sample;
    sample.startNs = prev_.tsNs;
    sample.endNs = cur.tsNs;
    sample.entries.reserve(metricIds_.size());

    for (uint32_t id : metricIds_)
    {
        switch (id)
        {
            case MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.npuUtil != kUtilFreqSentinel ? static_cast<double>(cur.npuUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.aicoreUtil != kUtilFreqSentinel ? static_cast<double>(cur.aicoreUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.aivectorUtil != kUtilFreqSentinel ? static_cast<double>(cur.aivectorUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.aicubeUtil != kUtilFreqSentinel ? static_cast<double>(cur.aicubeUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.hbmBwUtil != kUtilFreqSentinel ? static_cast<double>(cur.hbmBwUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL:
                sample.entries.push_back(
                    {id, 0, cur.hbmMemUtil != kUtilFreqSentinel ? static_cast<double>(cur.hbmMemUtil) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW:
            case MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW:
            {
                // HCCS 逐链路透传
                const PmBwType type = (id == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW) ? PM_BW_HCCS_TX : PM_BW_HCCS_RX;
                bool found = false;
                for (const auto &bw : cur.bandwidths)
                {
                    if (bw.type == type)
                    {
                        sample.entries.push_back({id, bw.instanceId, bw.bytesPerSec});
                        found = true;
                    }
                }
                if (!found)
                {
                    sample.entries.push_back({id, 0, 0.0});
                }
                break;
            }
            case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW:
            case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW:
            case MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW:
            case MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW:
            {
                PmBwType type = PM_BW_HCCS_TX;
                switch (id)
                {
                    case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW:
                        type = PM_BW_PCIE_TX;
                        break;
                    case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW:
                        type = PM_BW_PCIE_RX;
                        break;
                    case MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW:
                        type = PM_BW_ROCE_TX;
                        break;
                    default:
                        type = PM_BW_ROCE_RX;
                        break;
                }
                bool found = false;
                for (const auto &bw : cur.bandwidths)
                {
                    if (bw.type == type)
                    {
                        sample.entries.push_back({id, bw.instanceId, bw.bytesPerSec});
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    sample.entries.push_back({id, 0, 0.0});
                }
                break;
            }
            case MSPTI_PM_SAMPLING_METRIC_ID_POWER:
                // DCMI 原始单位 0.1W，归一化为 W 输出
                sample.entries.push_back(
                    {id, 0, cur.power != kIntSentinel ? static_cast<double>(cur.power) / 10.0 : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE:
                sample.entries.push_back(
                    {id, 0, cur.temperature != kIntSentinel ? static_cast<double>(cur.temperature) : 0.0});
                break;
            case MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ:
                sample.entries.push_back(
                    {id, 0, cur.aicoreFreq != kUtilFreqSentinel ? static_cast<double>(cur.aicoreFreq) : 0.0});
                break;
            default:
                break;
        }
    }

    // 全部指标均未会话配置时不产出（防御：parser 仅在有需求的会话上创建）
    if (sample.entries.empty())
    {
        return;
    }
    PmSamplingManager::GetInstance()->DeliverLocked(cur.deviceId, std::move(sample));
}

void PmSamplingParser::Flush()
{
    hasPrev_ = false;
    prev_ = PmDcmiSample{};
}

}  // namespace PmSampling
}  // namespace Mspti
