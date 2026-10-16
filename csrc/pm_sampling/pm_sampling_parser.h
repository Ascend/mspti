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
 * @file  pm_sampling_parser.h
 * @brief PM Sampling 解析器（每设备一个）：DCMI 原始样本 -> 会话窗口样本。
 *
 * DCMI 点查为瞬时值，解析方透传不差分。Parser 按会话指标集组装窗口样本：
 *   sample = {startNs = 上一 tick 时间戳, endNs = 当前 tick 时间戳, entries}
 *   entries 按会话指标顺序产出，逐实例一条（HCCS 逐链路 0~15），
 *   本窗口无数据的指标产出一条 {metricId, 0, 0.0}。
 * 首个 tick 仅作为窗口基准，不产出。
 */

#ifndef MSPTI_PM_SAMPLING_PM_SAMPLING_PARSER_H
#define MSPTI_PM_SAMPLING_PM_SAMPLING_PARSER_H

#include <cstdint>
#include <vector>

#include "csrc/pm_sampling/pm_sampling_contract.h"

namespace Mspti
{
namespace PmSampling
{

class PmSamplingParser
{
   public:
    // metricIds 为会话配置的指标集（决定产出的 entry 集合与顺序）
    explicit PmSamplingParser(const std::vector<uint32_t> &metricIds);

    // DCMI 原始样本入口（采集线程 -> manager -> parser，全程持 manager 锁）。
    // 组装 prev -> cur 窗口样本并经 manager 送入会话队列；时间倒退时重置基准。
    void OnDcmiSample(const PmDcmiSample &sample);

    // 会话停止时清空窗口基准（瞬时值语义，最后一段不产出）
    void Flush();

   private:
    // 用 cur 组装 [prev.tsNs, cur.tsNs] 窗口样本并投递
    void EmitWindow(const PmDcmiSample &cur);

    std::vector<uint32_t> metricIds_;
    PmDcmiSample prev_;
    bool hasPrev_ = false;
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_PM_SAMPLING_PARSER_H
