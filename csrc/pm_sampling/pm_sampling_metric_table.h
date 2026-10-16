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
 * @file  pm_sampling_metric_table.h
 * @brief PM Sampling 静态指标表：全 DCMI 来源指标（与公共头 metricId 枚举一一对应）。
 */

#ifndef MSPTI_PM_SAMPLING_PM_SAMPLING_METRIC_TABLE_H
#define MSPTI_PM_SAMPLING_PM_SAMPLING_METRIC_TABLE_H

#include "csrc/common/context_manager.h"
#include "csrc/include/mspti_pm_sampling.h"

namespace Mspti
{
namespace PmSampling
{

struct PmMetricInfo
{
    const char *shortName;
    msptiPmSamplingMetricGroup group;
    msptiPmSamplingMetricDataType dataType;
    const char *unit;
};

class PmSamplingMetricTable
{
   public:
    // 返回 metricId 的静态属性；metricId 越界时返回占位项。
    static const PmMetricInfo &Get(uint32_t metricId);

    // metricId 在指定芯片型号上是否可采；指标均由 DCMI 点查支撑，未知芯片型号不可采。
    static bool IsAvailable(uint32_t metricId, Common::PlatformType chipType);
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_PM_SAMPLING_METRIC_TABLE_H
