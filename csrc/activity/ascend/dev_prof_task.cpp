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

#include "csrc/activity/ascend/dev_prof_task.h"

#include <pthread.h>

#include <chrono>

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"
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
        case PROF_CHANNEL_AICPU:
            return std::make_unique<DevProfTaskAicpu>(deviceId);
            break;
        case PROF_CHANNEL_CUS_AICPU:
            return std::make_unique<DevProfTaskAiCustomCpu>(deviceId);
            break;
        default:
            return std::make_unique<DevProfTaskDefault>(deviceId);
            break;
    }
}

std::vector<std::unique_ptr<DevProfTask>> DevProfTaskFactory::CreateAicpuTasks(uint32_t deviceId)
{
    std::vector<std::unique_ptr<DevProfTask>> profTasks;
    profTasks.emplace_back(CreateDevChannelTask(deviceId, PROF_CHANNEL_AICPU));
    profTasks.emplace_back(CreateDevChannelTask(deviceId, PROF_CHANNEL_CUS_AICPU));
    return profTasks;
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
    const auto &channelTypes = kindIter->second;
    for (const auto &channelType : channelTypes)
    {
        auto task = CreateDevChannelTask(deviceId, channelType);
        profTasks.emplace_back(std::move(task));
    }
    return profTasks;
}

msptiResult DevProfTask::Start()
{
    if (!t_.joinable())
    {
        StartTask();
        t_ = std::thread(std::bind(&DevProfTask::Run, this));
    }
    return MSPTI_SUCCESS;
}

msptiResult DevProfTask::Stop()
{
    {
        std::unique_lock<std::mutex> lck(cv_mtx_);
        task_run_ = true;
        cv_.notify_one();
    }
    if (t_.joinable())
    {
        t_.join();
    }
    return MSPTI_SUCCESS;
}

void DevProfTask::Run()
{
    pthread_setname_np(pthread_self(), "DevProfTask");
    {
        std::unique_lock<std::mutex> lk(cv_mtx_);
        cv_.wait(lk, [&]() { return task_run_; });
    }
    StopTask();
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

DevProfTaskAicpuBase::DevProfTaskAicpuBase(uint32_t deviceId, AI_DRV_CHANNEL channelId, const std::string &eventGrpName)
    : DevProfTask(deviceId, channelId), eventGrpName_(eventGrpName)
{
}

DevProfTaskAicpuBase::~DevProfTaskAicpuBase()
{
    eventThreadRun_ = false;
    if (eventThread_.joinable())
    {
        eventThread_.join();
    }
    if (attachedDevice_.load())
    {
        try
        {
            (void)HalEschedDettachDevice(deviceId_);
        }
        catch (...)
        {
            MSPTI_LOGW("Dettach device %u failed.", deviceId_);
        }
        attachedDevice_ = false;
    }
    StopChannel();
}

msptiResult DevProfTaskAicpuBase::StartTask()
{
    if (Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(deviceId_, channelId_))
    {
        return StartChannel();
    }
    // 通道尚未生效：订阅设备事件，等事件触发/通道生效后再开启通道
    MSPTI_LOGI("Aicpu channel %u is invalid, wait for event to start it, device: %u.", channelId_, deviceId_);
    eventThreadRun_ = true;
    eventThread_ = std::thread(&DevProfTaskAicpuBase::EventThreadRun, this);
    return MSPTI_SUCCESS;
}

msptiResult DevProfTaskAicpuBase::StopTask()
{
    eventThreadRun_ = false;
    if (eventThread_.joinable())
    {
        eventThread_.join();
    }
    if (attachedDevice_.load())
    {
        try
        {
            (void)HalEschedDettachDevice(deviceId_);
        }
        catch (...)
        {
            MSPTI_LOGW("Dettach device %u failed.", deviceId_);
        }
        attachedDevice_ = false;
    }
    StopChannel();
    return MSPTI_SUCCESS;
}

bool DevProfTaskAicpuBase::CanFlush() { return channelStarted_.load(); }

msptiResult DevProfTaskAicpuBase::StartChannel()
{
    std::lock_guard<std::mutex> lk(channelMtx_);
    if (channelStarted_.load())
    {
        return MSPTI_SUCCESS;
    }
    if (!Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(deviceId_, channelId_))
    {
        return MSPTI_SUCCESS;
    }
    // AICPU通道为peripheral类型，当前不下发userData配置
    static const uint32_t AICPU_SAMPLE_PERIOD = 10;
    ProfStartParaT profStartPara;
    if (memset_s(&profStartPara, sizeof(profStartPara), 0, sizeof(profStartPara)) != EOK)
    {
        return MSPTI_ERROR_INNER;
    }
    auto addReaderRet = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->AddReader(deviceId_, channelId_);
    if (addReaderRet != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Failed to add reader for device: %u, channel id: %u, ret: %d.", deviceId_, channelId_,
                   static_cast<int32_t>(addReaderRet));
        return MSPTI_ERROR_INNER;
    }
    readerAdded_ = true;
    profStartPara.channelType = PROF_CHANNEL_TYPE_PERIPHERAL;
    profStartPara.samplePeriod = AICPU_SAMPLE_PERIOD;
    profStartPara.realTime = PROFILE_REAL_TIME;
    profStartPara.userData = nullptr;
    profStartPara.userDataSize = 0;
    int ret = ProfDrvStart(deviceId_, channelId_, &profStartPara);
    if (ret != 0)
    {
        MSPTI_LOGE("Failed to start Aicpu job for device: %u, channel id: %u", deviceId_, channelId_);
        // 启动失败：回滚本次已创建的Reader，避免其残留在ChannelPool中
        (void)Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(deviceId_, channelId_);
        readerAdded_ = false;
        return MSPTI_ERROR_INNER;
    }
    channelStarted_ = true;
    MSPTI_EVENT("Succeed to start Aicpu job for device: %u, channel id: %u", deviceId_, channelId_);
    return MSPTI_SUCCESS;
}

void DevProfTaskAicpuBase::StopChannel()
{
    std::lock_guard<std::mutex> lk(channelMtx_);
    if (!channelStarted_.load() && !readerAdded_)
    {
        return;
    }
    if (channelStarted_.load())
    {
        int ret = ProfStop(deviceId_, channelId_);
        if (ret != 0)
        {
            MSPTI_LOGE("Failed to stop Aicpu job for device: %u, channel id: %u", deviceId_, channelId_);
        }
    }
    if (readerAdded_)
    {
        (void)Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(deviceId_, channelId_);
        readerAdded_ = false;
    }
    channelStarted_ = false;
    MSPTI_EVENT("Succeed to stop Aicpu job for device: %u, channel id: %u", deviceId_, channelId_);
}

void DevProfTaskAicpuBase::EventThreadRun()
{
    pthread_setname_np(pthread_self(), "MsptiAicpuEvent");
    try
    {
        if (QueryDevPid() != MSPTI_SUCCESS)
        {
            MSPTI_LOGW("Unable to query device pid, device: %u, channel: %u.", deviceId_, channelId_);
            return;
        }
        if (HalEschedAttachDevice(deviceId_) != 0)
        {
            MSPTI_LOGE("Call halEschedAttachDevice failed, device: %u.", deviceId_);
            return;
        }
        attachedDevice_ = true;

        uint32_t grpId = 0;
        if (QueryGroupId(grpId) != MSPTI_SUCCESS)
        {
            MsptiEschedGrpParaT grpPara;
            if (memset_s(&grpPara, sizeof(grpPara), 0, sizeof(grpPara)) != EOK)
            {
                return;
            }
            grpPara.type = MSPTI_GRP_TYPE_BIND_CP_CPU;
            grpPara.threadNum = 1;
            if (strcpy_s(grpPara.grpName, sizeof(grpPara.grpName), eventGrpName_.c_str()) != EOK)
            {
                MSPTI_LOGE("Copy grp name: %s failed.", eventGrpName_.c_str());
                return;
            }
            if (HalEschedCreateGrpEx(deviceId_, &grpPara, &grpId) != 0)
            {
                MSPTI_LOGE("Call halEschedCreateGrpEx failed, device: %u.", deviceId_);
                (void)HalEschedDettachDevice(deviceId_);
                attachedDevice_ = false;
                return;
            }
            uint64_t eventBitmap = 1ULL << static_cast<uint64_t>(MSPTI_EVENT_USR_START);
            if (HalEschedSubscribeEvent(deviceId_, grpId, 0, eventBitmap) != 0)
            {
                MSPTI_LOGE("Call halEschedSubscribeEvent failed, device: %u.", deviceId_);
                (void)HalEschedDettachDevice(deviceId_);
                attachedDevice_ = false;
                return;
            }
        }
        WaitEvent(grpId);
    }
    catch (const std::exception &e)
    {
        MSPTI_LOGE("Aicpu event thread exception: %s, device: %u, channel: %u.", e.what(), deviceId_, channelId_);
    }
    catch (...)
    {
        MSPTI_LOGE("Aicpu event thread unknown exception, device: %u, channel: %u.", deviceId_, channelId_);
    }
}

void DevProfTaskAicpuBase::WaitEvent(uint32_t grpId)
{
    constexpr int32_t DRV_EVENT_TIMEOUT = 100;
    MsptiEventInfoT event;
    if (memset_s(&event, sizeof(event), 0, sizeof(event)) != EOK)
    {
        return;
    }
    event.comm.eventId = MSPTI_EVENT_MAX_NUM;
    bool onceFlag = true;
    int32_t timeout = 1;  // first timeout needs to check whether the channel is valid
    while (eventThreadRun_.load())
    {
        int err = HalEschedWaitEvent(deviceId_, grpId, 0, timeout, &event);
        timeout = DRV_EVENT_TIMEOUT;
        if (err == 0)
        {
            if (event.comm.eventId != MSPTI_EVENT_USR_START)
            {
                MSPTI_LOGE("Receive unexpected event, device: %u, channel: %u, eventId: %d.", deviceId_, channelId_,
                           event.comm.eventId);
                return;
            }
            if (!TryStartChannelWhenValid())
            {
                MSPTI_LOGE("Failed to start Aicpu channel, device: %u, channel: %u.", deviceId_, channelId_);
            }
            return;
        }
        if (err == MSPTI_DRV_ERROR_WAIT_TIMEOUT || err == MSPTI_DRV_ERROR_NO_EVENT)
        {
            // 仅首次超时时重查一次通道是否已生效，之后依赖事件
            if (!onceFlag)
            {
                continue;
            }
            onceFlag = false;
            if (TryStartChannelWhenValid())
            {
                return;
            }
            continue;
        }
        MSPTI_LOGW("Wait event failed, device: %u, channel: %u, ret: %d.", deviceId_, channelId_, err);
        return;
    }
}

bool DevProfTaskAicpuBase::TryStartChannelWhenValid()
{
    if (Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(deviceId_) == MSPTI_SUCCESS &&
        Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(deviceId_, channelId_))
    {
        MSPTI_LOGI("Channel is valid, device: %u, channel: %u.", deviceId_, channelId_);
        return StartChannel() == MSPTI_SUCCESS;
    }
    return false;
}

msptiResult DevProfTaskAicpuBase::QueryGroupId(uint32_t &grpId)
{
    MsptiEschedQueryGidOutputT gidOut;
    if (memset_s(&gidOut, sizeof(gidOut), 0, sizeof(gidOut)) != EOK)
    {
        return MSPTI_ERROR_INNER;
    }
    MsptiEschedQueryGidInputT gidIn;
    if (memset_s(&gidIn, sizeof(gidIn), 0, sizeof(gidIn)) != EOK)
    {
        return MSPTI_ERROR_INNER;
    }
    MsptiEschedOutputInfoT outPut = {&gidOut, sizeof(MsptiEschedQueryGidOutputT)};
    MsptiEschedInputInfoT inPut = {&gidIn, sizeof(MsptiEschedQueryGidInputT)};
    gidIn.pid = static_cast<int32_t>(Common::Utils::GetPid());
    if (strcpy_s(gidIn.grpName, sizeof(gidIn.grpName), eventGrpName_.c_str()) != EOK)
    {
        MSPTI_LOGE("Copy grp name: %s failed.", eventGrpName_.c_str());
        return MSPTI_ERROR_INNER;
    }
    if (HalEschedQueryInfo(deviceId_, MSPTI_QUERY_TYPE_LOCAL_GRP_ID, &inPut, &outPut) == 0)
    {
        grpId = gidOut.grpId;
        return MSPTI_SUCCESS;
    }
    return MSPTI_ERROR_INNER;
}

msptiResult DevProfTaskAicpuBase::QueryDevPid()
{
    constexpr uint32_t waitTimeMs = 20;
    constexpr int32_t waitCount = 80;
    int32_t devPid = 0;
    MsptiHalQueryDevpidInfoT info;
    if (memset_s(&info, sizeof(info), 0, sizeof(info)) != EOK)
    {
        return MSPTI_ERROR_INNER;
    }
    info.hostPid = static_cast<int32_t>(Common::Utils::GetPid());
    info.devId = deviceId_;
    info.procType = MSPTI_DEVDRV_PROCESS_CP1;
    for (int32_t i = 0; i < waitCount && eventThreadRun_.load(); ++i)
    {
        if (HalQueryDevpid(info, &devPid) == 0)
        {
            MSPTI_LOGI("Query devPid succ, device: %u, devPid: %d.", deviceId_, devPid);
            return MSPTI_SUCCESS;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(waitTimeMs));
    }
    return MSPTI_ERROR_INNER;
}
}  // namespace Ascend
}  // namespace Mspti
