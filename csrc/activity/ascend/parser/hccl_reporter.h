/* -------------------------------------------------------------------------
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This file is part of the MindStudio project.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *    http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#ifndef MSPTI_PARSER_HCCL_REPORTER_H
#define MSPTI_PARSER_HCCL_REPORTER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "csrc/activity/ascend/entity/hccl_op_desc.h"
#include "csrc/common/inject/hccl_inject.h"
#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Parser
{
class HcclReporter
{
   public:
    msptiResult RecordHcclMarker(const msptiActivityMarker* markActivity);
    msptiResult RecordHcclOp(uint32_t markId, std::shared_ptr<HcclOpDesc> opDesc);
    msptiResult ReportHcclActivity(std::shared_ptr<HcclOpDesc> markActivity);
    static HcclReporter* GetInstance();

   private:
    ~HcclReporter();
    msptiResult RecordStartMarker(const msptiActivityMarker* markActivity);
    msptiResult ReportHcclData(const msptiActivityMarker* markActivity);
    const char* GetSharedHcclName(const std::string& hcclName);

   private:
    std::mutex markMutex_;
    std::mutex nameMutex_;
    std::atomic<uint64_t> startMarkMissCount_{0};
    std::atomic<uint64_t> endMarkMissCount_{0};
    static std::unordered_map<std::string, std::string> commNameCache_;  // 缓存通信域名称，用于延长生命周期
    static std::unordered_map<uint64_t, std::shared_ptr<HcclOpDesc>> markId2HcclOp_;
};
}  // namespace Parser
}  // namespace Mspti

#endif  // MSPTI_PARSER_HCCL_REPORTER_H
