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

#ifndef MSPTI_PARSER_COMMUNICATION_CALCULATOR_H
#define MSPTI_PARSER_COMMUNICATION_CALCULATOR_H

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "csrc/activity/ascend/entity/communication_op_desc.h"
#include "csrc/activity/ascend/parser/cann_track_cache.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Parser
{
class CommunicationCalculator
{
   public:
    msptiResult AppendCompactInfo(bool agingFlag, const MsprofCompactInfo* data);

    msptiResult AppendApi2TaskInfo(const ApiEvent& api2TaskInfo);

    static CommunicationCalculator& GetInstance();

    void AppendCommunicationTask(ApiEvent& apiEvent);

    int64_t GetPendingCommunicationCount();

    void Clear();

   private:
    msptiResult ReportCommunication(uint64_t dstKey, const std::unique_ptr<CommunicationOpDesc>& hcclOp);

    msptiResult Record(const DeviceTask& taskTime);

    CommunicationCalculator() = default;

   private:
    std::mutex hcclTaskMutex_;

    // 通过eventId找communication算子
    std::unordered_map<uint64_t, std::unique_ptr<CommunicationOpDesc>> eventId2Communication_;

    // 记录每个DstKey对应的CommunicationId、是否是最后一个task，以及是否为 aging 任务
    struct CommunicationTaskRef
    {
        uint64_t eventId{0};
        bool isLast{false};
        bool agingFlag{true};
        // True if this entry holds one pendingCommunicationCount_ unit. Guard every
        // fetch_sub with it so phantom or already-consumed entries can't drive it negative.
        bool pendingCounted{false};
    };
    std::unordered_map<uint64_t, CommunicationTaskRef> communicationTask2Op_;

    std::mutex communicationOpInfoMutex_;
    std::unordered_map<std::uint64_t, std::map<std::uint64_t, std::unique_ptr<CommunicationOpDesc>>>
        communicationOpInfoQueue_;
    std::unordered_map<uint64_t, std::unique_ptr<CommunicationOpDesc>> taskId2AdditionInfo_;

    std::atomic<int64_t> pendingCommunicationCount_{0};
};
}  // namespace Parser
}  // namespace Mspti

#endif  // MSPTI_PARSER_COMMUNICATION_CALCULATOR_H
