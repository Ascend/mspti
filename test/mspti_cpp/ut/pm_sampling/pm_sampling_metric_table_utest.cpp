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
#include "gtest/gtest.h"

namespace
{
using Mspti::Common::PlatformType;
using Mspti::PmSampling::PmSamplingMetricTable;

constexpr uint32_t kMetricCount = MSPTI_PM_SAMPLING_METRIC_ID_COUNT;

bool IsV6Excluded(uint32_t metricId)
{
    return metricId == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW || metricId == MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW ||
           metricId == MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW || metricId == MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW;
}
}  // namespace

TEST(PmSamplingMetricTable, AllMetricsWillBeAvailableOn910B)
{
    for (uint32_t metricId = 0; metricId < kMetricCount; ++metricId)
    {
        EXPECT_TRUE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_910B)) << "metricId=" << metricId;
    }
}

TEST(PmSamplingMetricTable, AllMetricsWillBeAvailableOn310B)
{
    for (uint32_t metricId = 0; metricId < kMetricCount; ++metricId)
    {
        EXPECT_TRUE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_310B)) << "metricId=" << metricId;
    }
}

TEST(PmSamplingMetricTable, HccsAndRoceWillBeUnavailableOnV6)
{
    for (uint32_t metricId = 0; metricId < kMetricCount; ++metricId)
    {
        if (IsV6Excluded(metricId))
        {
            EXPECT_FALSE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_V6))
                << "metricId=" << metricId;
        }
    }
}

TEST(PmSamplingMetricTable, OtherMetricsWillBeAvailableOnV6)
{
    for (uint32_t metricId = 0; metricId < kMetricCount; ++metricId)
    {
        if (!IsV6Excluded(metricId))
        {
            EXPECT_TRUE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_V6)) << "metricId=" << metricId;
        }
    }
}

TEST(PmSamplingMetricTable, AllMetricsWillBeUnavailableOnUnknownChipType)
{
    const PlatformType unknownTypes[] = {PlatformType::END_TYPE, static_cast<PlatformType>(99)};
    for (const PlatformType chipType : unknownTypes)
    {
        for (uint32_t metricId = 0; metricId < kMetricCount; ++metricId)
        {
            EXPECT_FALSE(PmSamplingMetricTable::IsAvailable(metricId, chipType))
                << "metricId=" << metricId << " chipType=" << static_cast<int>(chipType);
        }
    }
}

TEST(PmSamplingMetricTable, InvalidMetricIdWillBeUnavailable)
{
    const uint32_t invalidIds[] = {kMetricCount, kMetricCount + 1, MSPTI_PM_SAMPLING_METRIC_ID_FORCE_INT};
    for (const uint32_t metricId : invalidIds)
    {
        EXPECT_FALSE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_910B));
        EXPECT_FALSE(PmSamplingMetricTable::IsAvailable(metricId, PlatformType::CHIP_V6));
    }
}
