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
#include "csrc/activity/ascend/dev_task_manager.h"

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/reporter/overhead_reporter.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/inject/profapi_inject.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"
#include "securec.h"

namespace Mspti
{
namespace Ascend
{

std::map<msptiActivityKind, uint64_t> DevTaskManager::datatype_config_map_ = {
    {MSPTI_ACTIVITY_KIND_KERNEL, MSPTI_CONFIG_KERNEL},
    {MSPTI_ACTIVITY_KIND_API, MSPTI_CONFIG_API},
    {MSPTI_ACTIVITY_KIND_MEMORY, MSPTI_CONFIG_RUNTIME_MEMORY},
    {MSPTI_ACTIVITY_KIND_MEMCPY, MSPTI_CONFIG_RUNTIME_MEMORY},
    {MSPTI_ACTIVITY_KIND_MEMSET, MSPTI_CONFIG_RUNTIME_MEMORY},
    {MSPTI_ACTIVITY_KIND_COMMUNICATION, MSPTI_CONFIG_COMMUNICATION},
    {MSPTI_ACTIVITY_KIND_RUNTIME_API, MSPTI_CONFIG_RUNTIME_API},
    {MSPTI_ACTIVITY_KIND_ACL_API, MSPTI_CONFIG_API},
    {MSPTI_ACTIVITY_KIND_NODE_API, MSPTI_CONFIG_API}};

DevTaskManager* DevTaskManager::GetInstance()
{
    static DevTaskManager instance;
    return &instance;
}

DevTaskManager::~DevTaskManager()
{
    for (auto iter = task_map_.begin(); iter != task_map_.end(); iter++)
    {
        StopAllDevKindProfTask(iter->second);
    }
    task_map_.clear();
    for (auto iter = aicpu_task_map_.begin(); iter != aicpu_task_map_.end(); iter++)
    {
        StopAllDevKindProfTask(iter->second);
    }
    aicpu_task_map_.clear();
}

DevTaskManager::DevTaskManager() { Mspti::Common::ContextManager::GetInstance()->InitHostTimeInfo(); }

msptiResult DevTaskManager::StartAllDevKindProfTask(std::vector<std::unique_ptr<DevProfTask>>& profTasks,
                                                    std::vector<std::unique_ptr<DevProfTask>>& successTasks)
{
    for (auto& profTask : profTasks)
    {
        if (profTask->Start() != MSPTI_SUCCESS)
        {
            return MSPTI_ERROR_INNER;
        }
        successTasks.emplace_back(std::move(profTask));
    }
    profTasks.clear();
    return MSPTI_SUCCESS;
}

msptiResult DevTaskManager::StopAllDevKindProfTask(std::vector<std::unique_ptr<DevProfTask>>& profTasks)
{
    msptiResult ret = MSPTI_SUCCESS;
    for (auto& profTask : profTasks)
    {
        if (profTask->Stop() != MSPTI_SUCCESS)
        {
            ret = MSPTI_ERROR_INNER;
        }
        profTask.reset(nullptr);
    }
    return ret;
}

msptiResult DevTaskManager::StartDevProfTask(uint32_t deviceId, const ActivitySwitchType& kinds)
{
    if (!CheckDeviceOnline(deviceId))
    {
        MSPTI_LOGE("Device: %u is offline.", deviceId);
        return MSPTI_ERROR_INNER;
    }
    Reporter::OverheadRecord overheadRecord(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE, MSPTI_ACTIVITY_OBJECT_DEVICE);
    overheadRecord.SetDeviceId(deviceId);
    MSPTI_LOGI("Start DevProfTask, deviceId: %u.", deviceId);
    if (Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(deviceId) != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Get device: %u channels failed.", deviceId);
        return MSPTI_ERROR_INNER;
    }
    Mspti::Common::ContextManager::GetInstance()->InitDeviceTimeInfo(deviceId);
    // 根据DeviceId配置项，开启CANN软件栈的Profiling任务
    if (StartCANNProfTask(deviceId, kinds) != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Start CANNProfTask failed. deviceId: %u", deviceId);
        return MSPTI_ERROR_INNER;
    }
    std::lock_guard<std::mutex> lk(task_map_mtx_);
    for (int kindIndex = 0; kindIndex < MSPTI_ACTIVITY_KIND_COUNT; kindIndex++)
    {
        if (!kinds[kindIndex])
        {
            continue;
        }
        auto kind = static_cast<msptiActivityKind>(kindIndex);
        auto iter = task_map_.find({deviceId, kind});
        if (iter == task_map_.end())
        {
            auto profTasks = Mspti::Ascend::DevProfTaskFactory::CreateTasks(deviceId, kind);
            decltype(profTasks) successTasks;
            auto ret = StartAllDevKindProfTask(profTasks, successTasks);
            task_map_.emplace(std::make_pair(deviceId, kind), std::move(successTasks));
            if (ret != MSPTI_SUCCESS)
            {
                MSPTI_LOGE("The device %u start DevProfTask failed.", deviceId);
                return ret;
            }
        }
        else
        {
            MSPTI_LOGW("The device: %u, kind: %d is already running.", deviceId, kind);
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult DevTaskManager::StopDevProfTask(uint32_t deviceId, const ActivitySwitchType& kinds)
{
    if (!CheckDeviceOnline(deviceId))
    {
        MSPTI_LOGE("Device: %u is offline.", deviceId);
        return MSPTI_ERROR_INNER;
    }
    Reporter::OverheadRecord overheadRecord(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE, MSPTI_ACTIVITY_OBJECT_DEVICE);
    overheadRecord.SetDeviceId(deviceId);
    MSPTI_LOGI("Stop DevProfTask, deviceId: %u", deviceId);
    if (StopCANNProfTask(deviceId) != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Stop CANNProfTask failed. deviceId: %u", deviceId);
        return MSPTI_ERROR_INNER;
    }
    msptiResult ret = MSPTI_SUCCESS;
    std::lock_guard<std::mutex> lk(task_map_mtx_);
    for (int kindIndex = 0; kindIndex < MSPTI_ACTIVITY_KIND_COUNT; kindIndex++)
    {
        if (!kinds[kindIndex])
        {
            continue;
        }
        auto kind = static_cast<msptiActivityKind>(kindIndex);
        auto iter = task_map_.find({deviceId, kind});
        if (iter != task_map_.end())
        {
            ret = StopAllDevKindProfTask(iter->second);
            task_map_.erase(iter);
        }
    }
    return ret;
}

msptiResult DevTaskManager::FlushDevProfData(uint32_t deviceId, msptiActivityKind kind)
{
    if (!CheckDeviceOnline(deviceId))
    {
        MSPTI_LOGE("Device: %u is offline.", deviceId);
        return MSPTI_ERROR_INNER;
    }
    MSPTI_LOGI("Flush DevProfData, deviceId: %u, kind: %d", deviceId, kind);
    std::lock_guard<std::mutex> lk(task_map_mtx_);
    auto taskIter = task_map_.find({deviceId, kind});
    if (taskIter == task_map_.end())
    {
        MSPTI_LOGW("The device: %u, kind: %d is not running.", deviceId, kind);
        return MSPTI_SUCCESS;
    }
    auto ret = MSPTI_SUCCESS;
    for (auto& profTask : taskIter->second)
    {
        if (profTask->Flush() != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("The device %u, kind: %d flush DevProfTask failed.", deviceId, kind);
            ret = MSPTI_ERROR_INNER;
        }
    }
    return ret;
}

bool DevTaskManager::CheckDeviceOnline(uint32_t deviceId)
{
    std::call_once(get_device_flag_, [this]() { this->InitDeviceList(); });
    return device_set_.find(deviceId) != device_set_.end() ? true : false;
}

void DevTaskManager::InitDeviceList()
{
    uint32_t deviceNum = 0;
    auto ret = DrvGetDevNum(&deviceNum);
    constexpr int32_t DEVICE_MAX_NUM = 64;
    if (ret != DRV_ERROR_NONE || deviceNum > DEVICE_MAX_NUM)
    {
        MSPTI_LOGE("Get device num failed.");
        return;
    }
    uint32_t deviceList[DEVICE_MAX_NUM] = {0};
    ret = DrvGetDevIDs(deviceList, deviceNum);
    if (ret != DRV_ERROR_NONE)
    {
        MSPTI_LOGE("Get device id list failed.");
        return;
    }
    device_set_.insert(std::begin(deviceList), std::end(deviceList));
}

void DevTaskManager::RegisterReportCallback()
{
    static const std::vector<std::pair<int, VOID_PTR>> CALLBACK_FUNC_LIST = {
        {PROFILE_REPORT_GET_HASH_ID_C_CALLBACK, reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiGetHashIdImpl)},
        {PROFILE_HOST_FREQ_IS_ENABLE_C_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiHostFreqIsEnableImpl)},
        {PROFILE_REPORT_API_C_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiApiReporterCallbackImpl)},
        {PROFILE_REPORT_EVENT_C_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiEventReporterCallbackImpl)},
        {PROFILE_REPORT_COMPACT_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiCompactInfoReporterCallbackImpl)},
        {PROFILE_REPORT_ADDITIONAL_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiAddiInfoReporterCallbackImpl)},
        {PROFILE_REPORT_REG_TYPE_INFO_C_CALLBACK,
         reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiRegReportTypeInfoImpl)},
        {PROFILE_DEVICE_STATE_C_CALLBACK, reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsprofDeviceStateImpl)},
    };
    for (auto iter : CALLBACK_FUNC_LIST)
    {
        auto ret = Mspti::Inject::MsprofRegisterProfileCallback(iter.first, iter.second, sizeof(VOID_PTR));
        if (ret != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("Failed to register reporter callback: %d", static_cast<int32_t>(iter.first));
        }
    }
}

void DevTaskManager::UnRegisterReportCallback()
{
    static const std::vector<std::pair<int, VOID_PTR>> CALLBACK_FUNC_LIST = {
        {PROFILE_HOST_FREQ_IS_ENABLE_C_CALLBACK, nullptr}, {PROFILE_REPORT_API_C_CALLBACK, nullptr},
        {PROFILE_REPORT_EVENT_C_CALLBACK, nullptr},        {PROFILE_REPORT_COMPACT_CALLBACK, nullptr},
        {PROFILE_REPORT_ADDITIONAL_CALLBACK, nullptr},     {PROFILE_DEVICE_STATE_C_CALLBACK, nullptr},
    };
    for (auto iter : CALLBACK_FUNC_LIST)
    {
        auto ret = Mspti::Inject::MsprofRegisterProfileCallback(iter.first, iter.second, sizeof(VOID_PTR));
        if (ret != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("Failed to unregister reporter callback: %d", static_cast<int32_t>(iter.first));
        }
    }
}

uint64_t DevTaskManager::GetProfSwitchHi(uint64_t profSwitch) const
{
    uint64_t profSwitchHi = 0ULL;
    // 参考CANN ProfAclMgr::GetProfSwitchHi：MC2(L1 task time) 或 AICPU_HCCL(L0 task time)
    // 场景需要置高位的aicpu通道开关，否则驱动不会开启aicpu通道
    if ((profSwitch & PROF_TASK_TIME_L1) != 0)
    {
        profSwitchHi |= PROF_HI_AICPU_CHANNEL;
    }
    if ((profSwitch & PROF_TASK_TIME) != 0)
    {
        profSwitchHi |= PROF_HI_AICPU_CHANNEL;
    }
    return profSwitchHi;
}

msptiResult DevTaskManager::StartCANNProfTask(uint32_t deviceId, const ActivitySwitchType& kinds)
{
    for (int kindIndex = 0; kindIndex < MSPTI_ACTIVITY_KIND_COUNT; kindIndex++)
    {
        if (!kinds[kindIndex])
        {
            continue;
        }
        auto kind = static_cast<msptiActivityKind>(kindIndex);
        auto cfg_iter = datatype_config_map_.find(kind);
        if (cfg_iter == datatype_config_map_.end())
        {
            MSPTI_LOGW("Device: %u, kind: %d don't need to start cann profiling task.", deviceId,
                       static_cast<int>(kind));
            continue;
        }
        profSwitch_ |= cfg_iter->second;
    }
    if (profSwitch_ == 0)
    {
        MSPTI_LOGW("profswitch is zero, don't need to enable cann prof.");
        return MSPTI_SUCCESS;
    }
    CommandHandle command;
    if (memset_s(&command, sizeof(command), 0, sizeof(command)) != EOK)
    {
        MSPTI_LOGE("memset CommandHandle failed.");
        return MSPTI_ERROR_INNER;
    }
    command.profSwitch = profSwitch_;
    command.profSwitchHi = GetProfSwitchHi(profSwitch_.load());
    command.devNums = 1;
    command.devIdList[0] = deviceId;
    command.modelId = PROF_INVALID_MODE_ID;
    command.type = PROF_COMMANDHANDLE_TYPE_START;
    // 下发开关中包含PROF_TASK_TIME时，需要打开AICPU/AiCustomCpu驱动通道。
    // 设备侧(devprof)会按组名(prof_aicpu_grp/prof_cus_grp)查宿主组并投递EVENT_USR_START，
    // 因此必须先建组+订阅，再下发命令，避免设备侧查不到组返回DRV_ERROR_UNINIT导致事件丢失
    const bool needAicpu = (profSwitch_.load() & PROF_TASK_TIME) != 0;
    if (needAicpu)
    {
        (void)StartAicpuProfTask(deviceId);
    }
    auto ret = Mspti::Inject::profSetProfCommand(static_cast<VOID_PTR>(&command), sizeof(CommandHandle));
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Start Profiling Command failed.");
        if (needAicpu)
        {
            (void)StopAicpuProfTask(deviceId);
        }
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

msptiResult DevTaskManager::StartAicpuProfTask(uint32_t deviceId)
{
    std::lock_guard<std::mutex> lk(task_map_mtx_);
    if (aicpu_task_map_.find(deviceId) != aicpu_task_map_.end())
    {
        MSPTI_LOGW("The device: %u Aicpu DevProfTask is already running.", deviceId);
        return MSPTI_SUCCESS;
    }
    auto profTasks = Mspti::Ascend::DevProfTaskFactory::CreateAicpuTasks(deviceId);
    decltype(profTasks) successTasks;
    auto ret = StartAllDevKindProfTask(profTasks, successTasks);
    aicpu_task_map_.emplace(deviceId, std::move(successTasks));
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("The device %u start Aicpu DevProfTask failed.", deviceId);
        return ret;
    }
    return MSPTI_SUCCESS;
}

msptiResult DevTaskManager::StopAicpuProfTask(uint32_t deviceId)
{
    std::lock_guard<std::mutex> lk(task_map_mtx_);
    auto iter = aicpu_task_map_.find(deviceId);
    if (iter == aicpu_task_map_.end())
    {
        return MSPTI_SUCCESS;
    }
    auto ret = StopAllDevKindProfTask(iter->second);
    aicpu_task_map_.erase(iter);
    return ret;
}

msptiResult DevTaskManager::StopCANNProfTask(uint32_t deviceId)
{
    CommandHandle command;
    if (memset_s(&command, sizeof(command), 0, sizeof(command)) != EOK)
    {
        MSPTI_LOGE("memset CommandHandle failed.");
        return MSPTI_ERROR_INNER;
    }
    if (profSwitch_ == 0)
    {
        MSPTI_LOGW("profswitch is zero, don't need to disable cann prof.");
        return MSPTI_SUCCESS;
    }
    command.profSwitch = profSwitch_;
    command.profSwitchHi = GetProfSwitchHi(profSwitch_.load());
    command.devNums = 1;
    command.devIdList[0] = deviceId;
    command.modelId = PROF_INVALID_MODE_ID;
    command.type = PROF_COMMANDHANDLE_TYPE_STOP;
    auto ret = Mspti::Inject::profSetProfCommand(static_cast<VOID_PTR>(&command), sizeof(CommandHandle));
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Stop Profiling Commond failed.");
        return MSPTI_ERROR_INNER;
    }
    // 与StartCANNProfTask对称，关闭PROF_TASK_TIME对应的AICPU/AiCustomCpu驱动通道
    if ((profSwitch_.load() & PROF_TASK_TIME) != 0)
    {
        return StopAicpuProfTask(deviceId);
    }
    return MSPTI_SUCCESS;
}
}  // namespace Ascend
}  // namespace Mspti
