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

#include "csrc/activity/ascend/parser/kernel_parser.h"

#include <mutex>
#include <queue>
#include <tuple>
#include <unordered_map>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/channel/stars_common.h"
#include "csrc/activity/ascend/entity/device_task.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/object_pool.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{
namespace
{
constexpr uint32_t DEFAULT_POOL_SIZE = 1000;
const std::string KERNEL_PREFIX = "KERNEL";

inline const char* GetValidKernelTypeName(uint32_t taskType)
{
    const auto& typeInfo = CannHashCache::GetTypeHashInfo(MSPROF_REPORT_RUNTIME_LEVEL, taskType);
    return Common::Utils::StartsWith(typeInfo, KERNEL_PREFIX) ? typeInfo.c_str() : "";
}
}  // namespace

class KernelParser::KernelParserImpl
{
    using msptiActivityKernelPtr = Common::SimpleObjectPool<msptiActivityKernel>::Ptr;
    using DeviceTaskPtr = Common::SimpleObjectPool<DeviceTask>::Ptr;

   public:
    KernelParserImpl()
        : activityPool(DEFAULT_POOL_SIZE),
          deviceTaskPool(DEFAULT_POOL_SIZE),
          kernel_map_(DEFAULT_POOL_SIZE),
          unaging_kernel_map_(DEFAULT_POOL_SIZE),
          device_kernel_map_(DEFAULT_POOL_SIZE)
    {
        hostTasks_.reserve(DEFAULT_POOL_SIZE);
    };
    ~KernelParserImpl()
    {
        for (auto& item : kernel_map_)
        {
            auto& kernelList = item.second;
            const auto& dstKey = Common::ContextManager::DecodeDstKey(item.first);
            MSPTI_LOGW("Release kernel: deviceId:%u, streamId:%u, taskId:%u, kernelCount:%lu", std::get<0>(dstKey),
                       std::get<1>(dstKey), std::get<2>(dstKey), kernelList.size());
        }
    }
    msptiResult ReportRtTaskTrack(uint32_t agingFlag, const MsprofCompactInfo* data);
    // 驱动数据均为单线程读取，函数内数据无需额外加锁
    msptiResult ReportSocLog(uint32_t deviceId, const HalLogData& originData);

   private:
    msptiResult DealUnAgingRtTaskTrack(const DeviceTask& task);
    msptiResult DealAgingRtTaskTrack(const DeviceTask& task);
    bool ParseDeviceTask(uint32_t deviceId, const SocLog& socLog, DeviceTaskPtr& task);
    bool IsGraphTask(uint64_t dstKey);
    msptiResult DealCacheHostTask();

   private:
    std::mutex hostTaskMutex_;
    std::vector<HostTask> hostTasks_{};
    std::vector<HostTask> dealHostTasks_{};

    Common::SimpleObjectPool<msptiActivityKernel> activityPool;
    Common::SimpleObjectPool<DeviceTask> deviceTaskPool;

    std::unordered_map<uint64_t, std::queue<msptiActivityKernelPtr>> kernel_map_{};
    std::unordered_map<uint64_t, msptiActivityKernelPtr> unaging_kernel_map_{};
    std::unordered_map<uint64_t, DeviceTaskPtr> device_kernel_map_{};
};

msptiResult KernelParser::KernelParserImpl::ReportRtTaskTrack(uint32_t agingFlag, const MsprofCompactInfo* data)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL))
    {
        return MSPTI_SUCCESS;
    }
    auto taskType = static_cast<uint32_t>(data->data.runtimeTrack.taskType);
    if (strcmp(GetValidKernelTypeName(taskType), "") == 0)
    {
        return MSPTI_SUCCESS;
    }
    Common::ContextManager::GetInstance()->UpdateAndReportCorrelationId(data->threadId);
    std::lock_guard<std::mutex> lk(hostTaskMutex_);
    hostTasks_.emplace_back(data->threadId, data->data.runtimeTrack.deviceId, data->data.runtimeTrack.streamId,
                            data->data.runtimeTrack.taskType, data->data.runtimeTrack.kernelName,
                            Common::ContextManager::GetInstance()->GetCorrelationId(), data->data.runtimeTrack.taskInfo,
                            agingFlag == 1);
    return MSPTI_SUCCESS;
}

msptiResult KernelParser::KernelParserImpl::ReportSocLog(uint32_t deviceId, const HalLogData& originData)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL))
    {
        return MSPTI_SUCCESS;
    }
    DealCacheHostTask();
    if (originData.type == ACSQ_LOG)
    {
        DeviceTaskPtr task;
        if (!ParseDeviceTask(deviceId, originData.acsq, task))
        {
            return MSPTI_SUCCESS;
        }
        auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(task->deviceId),
                                                           static_cast<uint16_t>(task->streamId), task->taskId);
        if (IsGraphTask(dstKey))
        {
            return DealUnAgingRtTaskTrack(*task);
        }
        else
        {
            return DealAgingRtTaskTrack(*task);
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult KernelParser::KernelParserImpl::DealCacheHostTask()
{
    {
        std::lock_guard<std::mutex> lk(hostTaskMutex_);
        if (hostTasks_.empty())
        {
            return MSPTI_SUCCESS;
        }
        dealHostTasks_.reserve(hostTasks_.size());
        dealHostTasks_.swap(hostTasks_);
    }
    for (const auto& hostTask : dealHostTasks_)
    {
        // GetKernelBasicInfo
        uint16_t deviceId = hostTask.deviceId;
        uint16_t streamId =
            Convert::StarsCommon::GetStreamId(hostTask.streamId, static_cast<uint16_t>(hostTask.taskInfo));
        uint32_t taskId = Convert::StarsCommon::GetHostTaskId(hostTask.streamId, hostTask.taskInfo, deviceId);
        bool agingFlag = hostTask.agingFlag;
        const char* kernelTypeName = GetValidKernelTypeName(static_cast<uint32_t>(hostTask.taskType));
        auto kernel = activityPool.acquire();
        kernel->ds.deviceId = deviceId;
        kernel->ds.streamId = streamId;
        kernel->kind = MSPTI_ACTIVITY_KIND_KERNEL;
        kernel->name = CannHashCache::GetHashInfo(hostTask.kernelHash).c_str();
        kernel->type = kernelTypeName;
        kernel->correlationId = hostTask.corrleationId;
        uint64_t dstKey = Common::ContextManager::EncodeDstKey(deviceId, streamId, taskId);
        // judgeGraph
        if (agingFlag)
        {
            kernel_map_[dstKey].push(std::move(kernel));
        }
        else
        {
            unaging_kernel_map_.emplace(dstKey, std::move(kernel));
        }
    }
    dealHostTasks_.clear();
    return MSPTI_SUCCESS;
}

msptiResult KernelParser::KernelParserImpl::DealAgingRtTaskTrack(const DeviceTask& task)
{
    auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(task.deviceId),
                                                       static_cast<uint16_t>(task.streamId), task.taskId);
    auto it = kernel_map_.find(dstKey);
    if (it == kernel_map_.end())
    {
        return MSPTI_SUCCESS;
    }
    auto& kernelList = it->second;
    if (kernelList.empty())
    {
        MSPTI_LOGE("The cache kernel list data is empty.");
        kernel_map_.erase(it);
        return MSPTI_ERROR_INNER;
    }
    auto& kernel = kernelList.front();
    kernel->start = task.start;
    kernel->end = task.end;
    auto result = Activity::ActivityManager::GetInstance()->Record(
        Common::ReinterpretConvert<msptiActivity*>(kernel.get()), sizeof(msptiActivityKernel));
    kernelList.pop();
    if (kernelList.empty())
    {
        kernel_map_.erase(it);
    }
    return result;
}

msptiResult KernelParser::KernelParserImpl::DealUnAgingRtTaskTrack(const DeviceTask& task)
{
    auto dstKey = Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(task.deviceId),
                                                       static_cast<uint16_t>(task.streamId), task.taskId);
    auto it = unaging_kernel_map_.find(dstKey);
    if (it == unaging_kernel_map_.end())
    {
        return MSPTI_SUCCESS;
    }
    auto& kernel = it->second;
    kernel->ds.deviceId = task.deviceId;
    kernel->ds.streamId = task.streamId;
    kernel->start = task.start;
    kernel->end = task.end;
    return Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(kernel.get()),
                                                            sizeof(msptiActivityKernel));
}

bool KernelParser::KernelParserImpl::ParseDeviceTask(uint32_t deviceId, const SocLog& socLog, DeviceTaskPtr& task)
{
    uint16_t streamId = socLog.streamId;
    uint32_t taskId = socLog.taskId;
    auto dstKey =
        Common::ContextManager::EncodeDstKey(static_cast<uint16_t>(deviceId), static_cast<uint16_t>(streamId), taskId);
    if (!kernel_map_.count(dstKey) && !unaging_kernel_map_.count(dstKey))
    {
        return false;
    }

    if (socLog.funcType == STARS_FUNC_TYPE_BEGIN)
    {
        DeviceTaskPtr temp = deviceTaskPool.acquire();
        temp->start = socLog.timestamp;
        temp->streamId = streamId;
        temp->taskId = taskId;
        temp->deviceId = deviceId;
        device_kernel_map_.emplace(dstKey, std::move(temp));
        return false;
    }
    else if (socLog.funcType == STARS_FUNC_TYPE_END)
    {
        auto it = device_kernel_map_.find(dstKey);
        if (it == device_kernel_map_.end())
        {
            return false;
        }
        task = std::move(it->second);  // copy
        device_kernel_map_.erase(it);
        std::vector<uint64_t> timeFromSysCnt =
            Common::ContextManager::GetInstance()->GetRealTimeFromSysCnt(deviceId, {task->start, socLog.timestamp});
        task->start = timeFromSysCnt[0];
        task->end = timeFromSysCnt[1];
        return true;
    }
    return false;
}

inline bool KernelParser::KernelParserImpl::IsGraphTask(uint64_t dstKey)
{
    return unaging_kernel_map_.find(dstKey) != unaging_kernel_map_.end();
}

// KernelParser
KernelParser& KernelParser::GetInstance()
{
    static KernelParser instance;
    return instance;
}

KernelParser::KernelParser() : pImpl(std::make_unique<KernelParserImpl>()) {};

KernelParser::~KernelParser() = default;

msptiResult KernelParser::ReportRtTaskTrack(uint32_t agingFlag, const MsprofCompactInfo* data)
{
    return pImpl->ReportRtTaskTrack(agingFlag, data);
}

msptiResult KernelParser::ReportStarsSocLog(uint32_t deviceId, const HalLogData& originData)
{
    return pImpl->ReportSocLog(deviceId, originData);
}
}  // namespace Parser
}  // namespace Mspti
