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

#include "csrc/activity/ascend/dev_prof_task.h"

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/plog_manager.h"
#include "securec.h"

namespace Mspti
{
namespace Ascend
{

const std::map<Mspti::Common::PlatformType, std::map<msptiActivityKind, std::set<AI_DRV_CHANNEL>>>
    DevProfTaskFactory::kindToChannel_map_ = {
        {Mspti::Common::PlatformType::CHIP_910B,
         {
             {MSPTI_ACTIVITY_KIND_MARKER, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_KERNEL, {PROF_CHANNEL_STARS_SOC_LOG}},
             {MSPTI_ACTIVITY_KIND_HCCL, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_COMMUNICATION, {PROF_CHANNEL_TS_FW, PROF_CHANNEL_STARS_SOC_LOG}},
         }},
        {Mspti::Common::PlatformType::CHIP_310B,
         {
             {MSPTI_ACTIVITY_KIND_MARKER, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_KERNEL, {PROF_CHANNEL_STARS_SOC_LOG}},
             {MSPTI_ACTIVITY_KIND_HCCL, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_COMMUNICATION, {PROF_CHANNEL_TS_FW, PROF_CHANNEL_STARS_SOC_LOG}},
         }},
        {Mspti::Common::PlatformType::CHIP_V6,
         {
             {MSPTI_ACTIVITY_KIND_MARKER, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_KERNEL, {PROF_CHANNEL_STARS_SOC_LOG}},
             {MSPTI_ACTIVITY_KIND_HCCL, {PROF_CHANNEL_TS_FW}},
             {MSPTI_ACTIVITY_KIND_COMMUNICATION, {PROF_CHANNEL_TS_FW, PROF_CHANNEL_STARS_SOC_LOG}},
         }}};

std::unique_ptr<DevProfTask> DevProfTaskFactory::CreateDevChannelTask(uint32_t deviceId, AI_DRV_CHANNEL channelId)
{
    switch (channelId)
    {
        case PROF_CHANNEL_TS_FW:
            return std::make_unique<DevProfTaskTsFw>(deviceId);
            break;
        case PROF_CHANNEL_STARS_SOC_LOG:
            return std::make_unique<DevProfTaskStars>(deviceId);
            break;
        default:
            return std::make_unique<DevProfTaskDefault>(deviceId);
            break;
    }
}

std::vector<std::unique_ptr<DevProfTask>> DevProfTaskFactory::CreateTasks(uint32_t deviceId, msptiActivityKind kind)
{
    std::vector<std::unique_ptr<DevProfTask>> profTasks;
    auto platform = Mspti::Common::ContextManager::GetInstance()->GetChipType(deviceId);
    auto devIter = kindToChannel_map_.find(platform);
    if (devIter == kindToChannel_map_.end())
    {
        MSPTI_LOGE("The platform: %d of device: %u is not support.", static_cast<int>(platform), deviceId);
        return profTasks;
    }
    auto kindIter = devIter->second.find(kind);
    if (kindIter == devIter->second.end())
    {
        MSPTI_LOGW("The kind: %d of device: %u is not support.", kind, deviceId);
        return profTasks;
    }
    const auto& channelTypes = kindIter->second;
    for (const auto& channelType : channelTypes)
    {
        auto task = CreateDevChannelTask(deviceId, channelType);
        profTasks.emplace_back(std::move(task));
    }
    return profTasks;
}

msptiResult DevProfTask::Start()
{
    if (started_)
    {
        MSPTI_LOGW("DevProfTask has already started, deviceId: %u, channel: %d.", deviceId_, channelId_);
        return MSPTI_SUCCESS;
    }
    started_ = true;
    // StartTask 的错误不上报：调用方（DevTaskManager）按全部成功管理 task，
    // 保证每一次 Start 都有配对的 Stop 来平衡 ref_cnt 和 reader，不过多引入错误分支
    (void)StartTask();
    return MSPTI_SUCCESS;
}

msptiResult DevProfTask::Stop()
{
    if (!started_)
    {
        return MSPTI_SUCCESS;
    }
    started_ = false;
    (void)StopTask();
    return MSPTI_SUCCESS;
}

msptiResult DevProfTask::Flush()
{
    if (!CanFlush())
    {
        MSPTI_LOGW("Task device: %u, channel: %d is not start or flush is not support.", deviceId_, channelId_);
        return MSPTI_SUCCESS;
    }
    auto ret = Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(deviceId_, channelId_);
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("FlushDrvBuff failed while flush data from device: %u, channel id: %u.", deviceId_, channelId_);
    }
    return ret;
}

// DevProfTaskTsFw的引用计数，保证在第一次使能时，Start Device任务
// 最后一次反使能时，Stop Device任务
std::map<uint32_t, uint32_t> DevProfTaskTsFw::ref_cnts_;
std::mutex DevProfTaskTsFw::cnt_mtx_;
msptiResult DevProfTaskTsFw::StartTask()
{
    uint32_t refCnt = 0;
    {
        std::lock_guard<std::mutex> lk(cnt_mtx_);
        auto iter = ref_cnts_.find(deviceId_);
        if (iter == ref_cnts_.end())
        {
            auto ret = ref_cnts_.insert({deviceId_, refCnt});
            if (!ret.second)
            {
                MSPTI_LOGE("Insert tsfw task cnt failed.");
                return MSPTI_ERROR_INNER;
            }
            iter = ret.first;
        }
        else
        {
            refCnt = iter->second;
        }
        iter->second++;
    }
    if (refCnt == 0)
    {
        if (!Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(deviceId_, channelId_))
        {
            return MSPTI_SUCCESS;
        }
        static const uint32_t SAMPLE_PERIOD = 20;
        Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->AddReader(deviceId_, channelId_);
        TsTsFwProfileConfigT configP;
        if (memset_s(&configP, sizeof(configP), 0, sizeof(configP)) != EOK)
        {
            return MSPTI_ERROR_INNER;
        }
        configP.period = SAMPLE_PERIOD;
        configP.tsKeypoint = 1;
        configP.tsBlockdim = TS_PROFILE_COMMAND_TYPE_PROFILING_ENABLE;
        ProfStartParaT profStartPara;
        profStartPara.channelType = PROF_CHANNEL_TYPE_TS;
        if (Common::ContextManager::GetInstance()->GetChipType(deviceId_) == Common::PlatformType::CHIP_V6)
        {
            profStartPara.samplePeriod = 0;
        }
        else
        {
            profStartPara.samplePeriod = 20;
        }
        profStartPara.realTime = PROFILE_REAL_TIME;
        profStartPara.userData = &configP;
        profStartPara.userDataSize = static_cast<unsigned int>(sizeof(TsTsFwProfileConfigT));
        int ret = ProfDrvStart(deviceId_, channelId_, &profStartPara);
        if (ret != 0)
        {
            MSPTI_LOGE("Failed to start TsTrackJob for device: %u, channel id: %u", deviceId_, channelId_);
            return MSPTI_ERROR_INNER;
        }
        MSPTI_EVENT("Succeed to start TsTrackJob for device: %u, channel id: %u", deviceId_, channelId_);
    }
    return MSPTI_SUCCESS;
}

msptiResult DevProfTaskTsFw::StopTask()
{
    uint32_t refCnt = 0;
    {
        std::lock_guard<std::mutex> lk(cnt_mtx_);
        auto iter = ref_cnts_.find(deviceId_);
        if (iter == ref_cnts_.end())
        {
            MSPTI_LOGW("The TsFw DevProfTask was not start. DeviceId: %u", deviceId_);
            return MSPTI_SUCCESS;
        }
        refCnt = --iter->second;
    }
    if (refCnt == 0)
    {
        int ret = ProfStop(deviceId_, channelId_);
        if (ret != 0)
        {
            MSPTI_LOGE("Failed to stop TsTrackJob for device: %u, channel id: %u", deviceId_, channelId_);
            return MSPTI_ERROR_INNER;
        }
        Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(deviceId_, channelId_);
        MSPTI_EVENT("Succeed to stop TsTrackJob for device: %u, channel id: %u", deviceId_, channelId_);
    }
    return MSPTI_SUCCESS;
}

bool DevProfTaskTsFw::CanFlush()
{
    std::lock_guard<std::mutex> lk(cnt_mtx_);
    auto iter = ref_cnts_.find(deviceId_);
    if (iter == ref_cnts_.end())
    {
        MSPTI_LOGW("The TsFw DevProfTask was not start. DeviceId: %u", deviceId_);
        return false;
    }
    return iter->second > 0;
}

// DevProfTaskStars的引用计数，保证在第一次使能时，Start Device任务
// 最后一次反使能时，Stop Device任务
std::map<uint32_t, uint32_t> DevProfTaskStars::ref_cnts_;
std::mutex DevProfTaskStars::cnt_mtx_;
msptiResult DevProfTaskStars::StartTask()
{
    uint32_t refCnt = 0;
    {
        std::lock_guard<std::mutex> lk(cnt_mtx_);
        auto iter = ref_cnts_.find(deviceId_);
        if (iter == ref_cnts_.end())
        {
            auto ret = ref_cnts_.insert({deviceId_, refCnt});
            if (!ret.second)
            {
                MSPTI_LOGE("Insert stars task cnt failed.");
                return MSPTI_ERROR_INNER;
            }
            iter = ret.first;
        }
        else
        {
            refCnt = iter->second;
        }
        iter->second++;
    }
    if (refCnt == 0)
    {
        if (!Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(deviceId_, channelId_))
        {
            return MSPTI_SUCCESS;
        }
        Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->AddReader(deviceId_, channelId_);
        StarsSocLogConfigT configP;
        if (memset_s(&configP, sizeof(StarsSocLogConfigT), 0, sizeof(StarsSocLogConfigT)) != EOK)
        {
            return MSPTI_ERROR_INNER;
        }
        configP.acsq_task = TS_PROFILE_COMMAND_TYPE_PROFILING_ENABLE;
        configP.ffts_thread_task = TS_PROFILE_COMMAND_TYPE_PROFILING_ENABLE;
        ProfStartParaT profStartPara;
        profStartPara.channelType = PROF_CHANNEL_TYPE_TS;
        if (Common::ContextManager::GetInstance()->GetChipType(deviceId_) == Common::PlatformType::CHIP_V6)
        {
            profStartPara.samplePeriod = 0;
        }
        else
        {
            profStartPara.samplePeriod = 20;
        }
        profStartPara.realTime = PROFILE_REAL_TIME;
        profStartPara.userData = &configP;
        profStartPara.userDataSize = static_cast<unsigned int>(sizeof(StarsSocLogConfigT));
        int ret = ProfDrvStart(deviceId_, channelId_, &profStartPara);
        if (ret != 0)
        {
            MSPTI_LOGE("Failed to start ProfStarsJob for device: %u, channel id: %u", deviceId_, channelId_);
            return MSPTI_ERROR_INNER;
        }
        MSPTI_EVENT("Succeed to start ProfStarsJob for device: %u, channel id: %u", deviceId_, channelId_);
    }
    return MSPTI_SUCCESS;
}

msptiResult DevProfTaskStars::StopTask()
{
    uint32_t refCnt = 0;
    {
        std::lock_guard<std::mutex> lk(cnt_mtx_);
        auto iter = ref_cnts_.find(deviceId_);
        if (iter == ref_cnts_.end())
        {
            MSPTI_LOGW("The Stars DevProfTask was not start. DeviceId: %u", deviceId_);
            return MSPTI_SUCCESS;
        }
        refCnt = --iter->second;
    }
    if (refCnt == 0)
    {
        int ret = ProfStop(deviceId_, channelId_);
        if (ret != 0)
        {
            MSPTI_LOGE("Failed to stop ProfStarsJob for device: %u, channel id: %u", deviceId_, channelId_);
            return MSPTI_ERROR_INNER;
        }
        Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(deviceId_, channelId_);
        MSPTI_EVENT("Succeed to stop ProfStarsJob for device: %u, channel id: %u", deviceId_, channelId_);
    }
    return MSPTI_SUCCESS;
}

bool DevProfTaskStars::CanFlush()
{
    std::lock_guard<std::mutex> lk(cnt_mtx_);
    auto iter = ref_cnts_.find(deviceId_);
    if (iter == ref_cnts_.end())
    {
        MSPTI_LOGW("The Stars DevProfTask was not start. DeviceId: %u", deviceId_);
        return false;
    }
    return iter->second > 0;
}
}  // namespace Ascend
}  // namespace Mspti
