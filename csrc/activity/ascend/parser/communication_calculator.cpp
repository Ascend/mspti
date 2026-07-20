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

#include "csrc/activity/ascend/parser/communication_calculator.h"

#include <algorithm>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/stars_common.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{
CommunicationCalculator& CommunicationCalculator::GetInstance()
{
    static CommunicationCalculator instance;
    return instance;
}

msptiResult CommunicationCalculator::AppendCompactInfo(bool agingFlag, const MsprofCompactInfo* data)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION))
    {
        return MSPTI_SUCCESS;
    }
    auto& msprofHcclOpInfo = data->data.hcclopInfo;
    std::unique_ptr<CommunicationOpDesc> communication;
    Mspti::Common::MsptiMakeUniquePtr(communication);
    if (!UNLIKELY(communication))
    {
        MSPTI_LOGE("fail to malloc msptiActivityCommunication, return MSPTI_ERROR_INNER");
        return MSPTI_ERROR_INNER;
    }
    communication->agingFlag = agingFlag;
    communication->dataType = data->data.hcclopInfo.dataType;
    communication->groupNameHash = msprofHcclOpInfo.groupName;
    communication->algTypeHash = msprofHcclOpInfo.algType;
    communication->count = msprofHcclOpInfo.count;
    std::lock_guard<std::mutex> lk(communicationOpInfoMutex_);
    communicationOpInfoQueue_[data->threadId].insert(std::make_pair(data->timeStamp, std::move(communication)));
    return MSPTI_SUCCESS;
}

void TransApiEvent2CommTask(const ApiEvent& api2TaskInfo, CommunicationTask& commTask)
{
    auto track = api2TaskInfo.compactInfo.data.runtimeTrack;
    auto taskId = Convert::StarsCommon::GetHostTaskId(track.streamId, track.taskInfo, track.deviceId);
    auto streamId = Convert::StarsCommon::GetStreamId(track.streamId, static_cast<uint16_t>(track.taskInfo));
    commTask.start = 0;
    commTask.end = 0;
    commTask.hostStartTime = api2TaskInfo.api.beginTime;
    commTask.hostEndTime = api2TaskInfo.api.endTime;
    commTask.streamId = streamId;
    commTask.taskId = taskId;
    commTask.deviceId = track.deviceId;
    commTask.agingFlag = api2TaskInfo.agingFlag;
}

std::unique_ptr<CommunicationOpDesc> TransApiEvent2CommOpDesc(const ApiEvent& api2TaskInfo)
{
    std::unique_ptr<CommunicationOpDesc> desc;
    Mspti::Common::MsptiMakeUniquePtr(desc);
    if (!UNLIKELY(desc))
    {
        return desc;
    }
    desc->correlationId = api2TaskInfo.correlationId;
    desc->opNameHash = api2TaskInfo.api.itemId;
    desc->hostStartTime = api2TaskInfo.api.beginTime;
    desc->hostEndTime = api2TaskInfo.api.endTime;
    desc->threadId = api2TaskInfo.api.threadId;
    desc->opNameHash = api2TaskInfo.api.itemId;
    for (auto& communicationTask : api2TaskInfo.children)
    {
        std::unique_ptr<CommunicationTask> commTask;
        Mspti::Common::MsptiMakeUniquePtr(commTask);
        if (UNLIKELY(commTask == nullptr))
        {
            MSPTI_LOGE("can not TransApiEvent2CommTask, commTask is nullptr!");
            continue;
        }
        TransApiEvent2CommTask(communicationTask, *commTask);
        desc->agingFlag = commTask->agingFlag;
        desc->streamId = commTask->streamId;
        desc->tasks.emplace_back(std::move(commTask));
    }
    return desc;
}

msptiResult CommunicationCalculator::AppendApi2TaskInfo(const ApiEvent& api2TaskInfo)
{
    if (api2TaskInfo.children.empty())
    {
        MSPTI_LOGW("target Api has not communication tasks");
        return MSPTI_SUCCESS;
    }

    auto commOp = TransApiEvent2CommOpDesc(api2TaskInfo);
    if (commOp->tasks.empty())
    {
        MSPTI_LOGW("commOp has not communication tasks");
        return MSPTI_SUCCESS;
    }
    auto lastHostTask =
        std::max_element(commOp->tasks.begin(), commOp->tasks.end(),
                         [](const std::unique_ptr<CommunicationTask>& a, const std::unique_ptr<CommunicationTask>& b)
                         { return a->hostStartTime < b->hostStartTime; });
    auto& lastTask = *lastHostTask;
    auto lastTaskDstKey =
        Common::ContextManager::EncodeDstKey(lastTask->deviceId, lastTask->streamId, lastTask->taskId);

    std::lock_guard<std::mutex> lk(hcclTaskMutex_);
    communicationTask2Op_[lastTaskDstKey].second = true;
    commOp->startTime = UINT64_MAX;
    eventId2Communication_[api2TaskInfo.eventId] = std::move(commOp);
    return MSPTI_SUCCESS;
}

void AssembleTaskInfo(msptiActivityCommunication& communication, const CommunicationOpDesc* additionOpDesc,
                      const CommunicationOpDesc* OpTaskDesc)
{
    communication.kind = MSPTI_ACTIVITY_KIND_COMMUNICATION;
    if (additionOpDesc != nullptr)
    {
        communication.dataType = static_cast<msptiCommunicationDataType>(additionOpDesc->dataType);
        communication.count = additionOpDesc->count;
        communication.algType = CannHashCache::GetHashInfo(additionOpDesc->algTypeHash).c_str();
        communication.commName = CannHashCache::GetHashInfo(additionOpDesc->groupNameHash).c_str();
    }

    if (OpTaskDesc != nullptr)
    {
        communication.ds.deviceId = OpTaskDesc->deviceId;
        communication.ds.streamId = OpTaskDesc->streamId;
        communication.start = OpTaskDesc->startTime;
        communication.end = OpTaskDesc->endTime;
        communication.name = CannHashCache::GetHashInfo(OpTaskDesc->opNameHash).c_str();
        communication.correlationId = OpTaskDesc->correlationId;
    }
}

msptiResult CommunicationCalculator::ReportCommunication(uint64_t dstKey,
                                                         const std::unique_ptr<CommunicationOpDesc>& commOp)
{
    {
        std::lock_guard<std::mutex> lk(communicationOpInfoMutex_);
        if (!commOp->agingFlag && taskId2AdditionInfo_.count(dstKey))
        {
            auto& addationOpDesc = taskId2AdditionInfo_[dstKey];
            msptiActivityCommunication record{};
            AssembleTaskInfo(record, addationOpDesc.get(), commOp.get());
            Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&record),
                                                             sizeof(msptiActivityCommunication));
        }
        else
        {
            auto& timeStampOpInfoMap = communicationOpInfoQueue_[commOp->threadId];
            if (!timeStampOpInfoMap.empty())
            {
                auto startOpInfo = timeStampOpInfoMap.lower_bound(commOp->hostStartTime);
                auto endOpInfo = timeStampOpInfoMap.upper_bound(commOp->hostEndTime);
                if (startOpInfo == timeStampOpInfoMap.end() || startOpInfo == endOpInfo)
                {
                    MSPTI_LOGW(
                        "can not find addition info for communication op, threadId: %lu, hostStartTime: %lu, "
                        "hostEndTime: %lu",
                        commOp->threadId, commOp->hostStartTime, commOp->hostEndTime);
                    return MSPTI_ERROR_INNER;
                }
                auto& addationOpDesc = startOpInfo->second;
                msptiActivityCommunication record{};
                AssembleTaskInfo(record, addationOpDesc.get(), commOp.get());
                Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&record),
                                                                 sizeof(msptiActivityCommunication));
                if (!commOp->agingFlag)
                {
                    taskId2AdditionInfo_.emplace(dstKey, std::move(addationOpDesc));
                }
                timeStampOpInfoMap.erase(startOpInfo, endOpInfo);
            }
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult CommunicationCalculator::Record(const DeviceTask& taskTime)
{
    auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(taskTime.deviceId),
                                                       static_cast<uint16_t>(taskTime.streamId), taskTime.taskId);
    std::lock_guard<std::mutex> lk(hcclTaskMutex_);
    auto iter = communicationTask2Op_.find(dstKey);
    if (iter == communicationTask2Op_.end())
    {
        return MSPTI_SUCCESS;
    }

    auto commEventIter = eventId2Communication_.find(iter->second.first);
    if (commEventIter == eventId2Communication_.end())
    {
        return MSPTI_SUCCESS;
    }
    auto& commOp = commEventIter->second;
    commOp->agingFlag = taskTime.agingFlag;
    commOp->deviceId = taskTime.deviceId;
    uint64_t startTime = 0;
    if (taskTime.isFfts && !taskTime.subTasks.empty())
    {
        auto startIt = std::min_element(taskTime.subTasks.begin(), taskTime.subTasks.end(),
                                        [](const SubTask& a, const SubTask& b) { return a.start < b.start; });
        startTime = (*startIt).start;
    }
    else
    {
        startTime = taskTime.start;
    }
    commOp->startTime = std::min(commOp->startTime, startTime);

    if (iter->second.second)
    {
        if (taskTime.isFfts && !taskTime.subTasks.empty())
        {
            auto endIt = std::max_element(taskTime.subTasks.begin(), taskTime.subTasks.end(),
                                          [](const SubTask& a, const SubTask& b) { return a.end < b.end; });
            commOp->endTime = (*endIt).end;
        }
        else
        {
            commOp->endTime = taskTime.end;
        }
        ReportCommunication(dstKey, commOp);
        if (commOp->agingFlag)
        {
            eventId2Communication_.erase(commEventIter);
            communicationTask2Op_.erase(iter);
        }
        else
        {
            // start时间根据最小值进行判断, 图模式场景下commop常驻，不更新startTime会导致后续startTime异常
            commOp->startTime = UINT64_MAX;
        }
    }
    return MSPTI_SUCCESS;
}

void CommunicationCalculator::AppendCommunicationTask(ApiEvent& apiEvent)
{
    CommunicationTask commTask;
    TransApiEvent2CommTask(apiEvent, commTask);

    auto dstKey = Common::ContextManager::EncodeDstKey(commTask.deviceId, commTask.streamId, commTask.taskId);
    DeviceTask task(0, 0, commTask.streamId, commTask.taskId, commTask.deviceId, false, commTask.agingFlag);
    DeviceTaskCalculator::GetInstance().RegisterCallBack(task, [this](const DeviceTask& task) { return Record(task); });
    std::lock_guard<std::mutex> lk(hcclTaskMutex_);
    communicationTask2Op_[dstKey] = {apiEvent.parentEventId, false};
}
}  // namespace Parser
}  // namespace Mspti
