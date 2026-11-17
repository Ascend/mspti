/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "csrc/activity/ascend/parser/device_task_calculator.h"

#include <utility>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/channel/stars_common.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"

namespace Mspti
{
namespace Parser
{
void DeviceTaskCalculator::RegisterCallBack(const DeviceTask& task,
                                            const DeviceTaskCalculator::CompleteFunc& completeFunc)
{
    std::lock_guard<std::mutex> lockGuard(assembleTaskMutex_);
    assembleTaskQueue_.emplace_back(task, completeFunc);
}

msptiResult DeviceTaskCalculator::ReportStarsSocLog(uint32_t deviceId, const HalLogData& originData)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION))
    {
        return MSPTI_SUCCESS;
    }
    DealCacheDeviceTask();
    if (originData.type == ACSQ_LOG)
    {
        return AssembleTasksTimeWithSocLog(deviceId, originData.acsq);
    }

    if (originData.type == FFTS_LOG)
    {
        return AssembleSubTasksTimeWithFftsLog(deviceId, originData.ffts);
    }
    MSPTI_LOGW("stars log from device %u is not ffts or stars, funcType is %u", deviceId, originData.type);
    return MSPTI_SUCCESS;
}

msptiResult DeviceTaskCalculator::AssembleTasksTimeWithSocLog(uint32_t deviceId, const SocLog& socLog)
{
    uint16_t streamId = socLog.streamId;
    uint32_t taskId = socLog.taskId;
    auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(deviceId), streamId, taskId);
    CompleteFunc callback;
    // 线程局部缓冲复用 subTasks 容量，避免每个 END 事件重新分配；回调不得重入本函数。
    thread_local DeviceTask completedTask;
    {
        // 多 device 的消费线程会并发进入；assembleTasks_/completeFunc_ 必须串行访问
        std::lock_guard<std::mutex> lk(assembleTaskMutex_);
        auto iter = assembleTasks_.find(dstKey);
        if (iter == assembleTasks_.end())
        {
            return MSPTI_SUCCESS;
        }
        auto& deviceTask = iter->second;
        if (socLog.funcType == STARS_FUNC_TYPE_BEGIN)
        {
            deviceTask.start = socLog.timestamp;
            return MSPTI_SUCCESS;
        }
        if (socLog.funcType != STARS_FUNC_TYPE_END)
        {
            return MSPTI_SUCCESS;
        }
        uint64_t startRealTime = deviceTask.start;
        uint64_t endRealTime = socLog.timestamp;
        Common::ContextManager::GetInstance()->GetDeviceRealTime(deviceId, startRealTime, endRealTime);
        deviceTask.start = startRealTime;
        deviceTask.end = endRealTime;

        auto callbackIter = completeFunc_.find(dstKey);
        if (callbackIter == completeFunc_.end() || !callbackIter->second)
        {
            // 注册表与任务表应同增同删，不一致时丢弃，避免调用空 std::function
            MSPTI_LOGE("Missing complete callback, drop device task, deviceId=%u, streamId=%u, taskId=%u", deviceId,
                       streamId, taskId);
            assembleTasks_.erase(iter);
            completeFunc_.erase(dstKey);
            return MSPTI_ERROR_INNER;
        }
        if (deviceTask.agingFlag)
        {
            // aging task 用完即删，任务与回调都 move 出去
            callback = std::move(callbackIter->second);
            completedTask = std::move(deviceTask);
            completeFunc_.erase(callbackIter);
            assembleTasks_.erase(iter);
        }
        else
        {
            // 非 aging task 留在 map 里复用，只能拷贝
            callback = callbackIter->second;
            completedTask = deviceTask;
        }
    }
    // 回调会再取 CommunicationCalculator 锁，必须在 assembleTaskMutex_ 之外执行
    return callback(completedTask);
}

msptiResult DeviceTaskCalculator::AssembleSubTasksTimeWithFftsLog(uint32_t deviceId, const FftsLog& fftsLog)
{
    uint16_t streamId = fftsLog.streamId;
    uint32_t taskId = fftsLog.taskId;
    auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(deviceId), streamId, taskId);
    auto dstsKey = std::make_tuple(static_cast<uint16_t>(deviceId), streamId, taskId, fftsLog.subTaskId);

    std::lock_guard<std::mutex> lk(assembleTaskMutex_);
    auto iter = assembleTasks_.find(dstKey);
    if (iter == assembleTasks_.end())
    {
        return MSPTI_SUCCESS;
    }
    // 先校验 funcType 再插入，避免畸形数据在 assembleSubTasks_ 里留下永久条目
    if (fftsLog.funcType != FFTS_PLUS_TYPE_START && fftsLog.funcType != FFTS_PLUS_TYPE_END)
    {
        MSPTI_LOGW_EVERY_N(Common::LOG_EVERY_N_FAILURE,
                           "Unknown ffts funcType %u, drop sub task, deviceId=%u, streamId=%u, taskId=%u, subTaskId=%u",
                           static_cast<uint32_t>(fftsLog.funcType), deviceId, streamId, taskId, fftsLog.subTaskId);
        return MSPTI_SUCCESS;
    }
    auto it = assembleSubTasks_.find(dstsKey);
    if (it == assembleSubTasks_.end())
    {
        it = assembleSubTasks_.emplace(dstsKey, SubTask{}).first;
    }
    auto& subTask = it->second;
    if (fftsLog.funcType == FFTS_PLUS_TYPE_START)
    {
        subTask.start = fftsLog.timestamp;
        subTask.streamId = streamId;
        subTask.taskId = taskId;
        subTask.subTaskId = fftsLog.subTaskId;
    }
    else
    {
        uint64_t startRealTime = subTask.start;
        uint64_t endRealTime = fftsLog.timestamp;
        Common::ContextManager::GetInstance()->GetDeviceRealTime(deviceId, startRealTime, endRealTime);
        subTask.start = startRealTime;
        subTask.end = endRealTime;
        iter->second.isFfts = true;
        iter->second.subTasks.emplace_back(subTask);
        assembleSubTasks_.erase(it);
    }
    return MSPTI_SUCCESS;
}

void DeviceTaskCalculator::Clear()
{
    std::lock_guard<std::mutex> lk(assembleTaskMutex_);
    const auto taskCount = assembleTasks_.size();
    const auto subTaskCount = assembleSubTasks_.size();
    const auto callbackCount = completeFunc_.size();
    const auto pendingCount = assembleTaskQueue_.size();
    assembleTasks_.clear();
    assembleSubTasks_.clear();
    completeFunc_.clear();
    assembleTaskQueue_.clear();
    dealAssembleTaskQueue_.clear();
    if (taskCount > 0 || subTaskCount > 0 || callbackCount > 0 || pendingCount > 0)
    {
        MSPTI_LOGI("DeviceTaskCalculator cleared, tasks=%zu, subTasks=%zu, callbacks=%zu, pending=%zu", taskCount,
                   subTaskCount, callbackCount, pendingCount);
    }
}

void DeviceTaskCalculator::DealCacheDeviceTask()
{
    std::lock_guard<std::mutex> lk(assembleTaskMutex_);
    if (assembleTaskQueue_.empty())
    {
        return;
    }
    dealAssembleTaskQueue_.reserve(assembleTaskQueue_.size());
    dealAssembleTaskQueue_.swap(assembleTaskQueue_);
    for (const auto& it : dealAssembleTaskQueue_)
    {
        auto& deviceTask = it.first;
        auto& callbackFunc = it.second;
        uint64_t dstKey = Common::ContextManager::EncodeDstKey(
            static_cast<uint16_t>(deviceTask.deviceId), static_cast<uint16_t>(deviceTask.streamId), deviceTask.taskId);
        assembleTasks_.emplace(dstKey, deviceTask);
        completeFunc_.emplace(dstKey, callbackFunc);
    }
    dealAssembleTaskQueue_.clear();
}
}  // namespace Parser
}  // namespace Mspti
