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

#include "csrc/pm_sampling/pm_sampling_metric_table.h"

#include <array>
#include <unordered_map>
#include <unordered_set>

namespace Mspti
{
namespace PmSampling
{
namespace
{

// 与 mspti_pm_sampling.h 的 MSPTI_PM_SAMPLING_METRIC_ID_* 一一对应，
// 顺序不可调整；新增指标须同步扩展公共头枚举与本表
const std::array<PmMetricInfo, MSPTI_PM_SAMPLING_METRIC_ID_COUNT> kMetricTable = {{
    {"npu_util", MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"aicore_util", MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"aivector_util", MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"aicube_util", MSPTI_PM_SAMPLING_METRIC_GROUP_UTILIZATION, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"hbm_bw_util", MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"hbm_mem_util", MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT, "%"},
    {"hccs_tx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"hccs_rx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"pcie_tx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"pcie_rx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"roce_tx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"roce_rx_bw", MSPTI_PM_SAMPLING_METRIC_GROUP_NETWORK, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_THROUGHPUT, "bytes/s"},
    {"power", MSPTI_PM_SAMPLING_METRIC_GROUP_POWER, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_POWER, "W"},
    {"temperature", MSPTI_PM_SAMPLING_METRIC_GROUP_THERMAL, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE, "℃"},
    {"aicore_freq", MSPTI_PM_SAMPLING_METRIC_GROUP_FREQUENCY, MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_FREQUENCY, "MHz"},
}};

const PmMetricInfo kInvalidMetricInfo = {"", MSPTI_PM_SAMPLING_METRIC_GROUP_MEMORY,
                                         MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_RATIO, ""};

// 芯片×指标预期支持矩阵：列出各芯片不支持的指标，未列出者默认可采；
// 新增芯片型号或不支持项时只扩展本表
const std::unordered_map<Common::PlatformType, std::unordered_set<uint32_t>> kChipMetricExclusions = {
    {Common::PlatformType::CHIP_V6,
     {MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW,
      MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW, MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW}},
};

}  // namespace

const PmMetricInfo &PmSamplingMetricTable::Get(uint32_t metricId)
{
    if (metricId >= MSPTI_PM_SAMPLING_METRIC_ID_COUNT)
    {
        return kInvalidMetricInfo;
    }
    return kMetricTable[metricId];
}

bool PmSamplingMetricTable::IsAvailable(uint32_t metricId, Common::PlatformType chipType)
{
    if (metricId >= MSPTI_PM_SAMPLING_METRIC_ID_COUNT)
    {
        return false;
    }
    if (chipType != Common::PlatformType::CHIP_910B && chipType != Common::PlatformType::CHIP_310B &&
        chipType != Common::PlatformType::CHIP_V6)
    {
        return false;
    }
    // 芯片×指标预期支持矩阵见 kChipMetricExclusions
    const auto it = kChipMetricExclusions.find(chipType);
    if (it == kChipMetricExclusions.end())
    {
        return true;
    }
    return it->second.find(metricId) == it->second.end();
}

}  // namespace PmSampling
}  // namespace Mspti
