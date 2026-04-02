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

#include "csrc/common/inject/profapi_inject.h"

#include <functional>
#include "csrc/activity/ascend/parser/kernel_parser.h"
#include "csrc/activity/ascend/parser/cann_api_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/activity/ascend/parser/cann_track_cache.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/common/function_loader.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/utils.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/communication_calculator.h"
#include "csrc/activity/ascend/channel/stars_common.h"
#include "csrc/activity/activity_manager.h"

namespace Mspti {
namespace Inject {
class ProfApiInject {
public:
    ProfApiInject() noexcept
    {
        Mspti::Common::RegisterFunction("libprofapi", "MsprofRegisterProfileCallback");
        Mspti::Common::RegisterFunction("libprofapi", "profSetProfCommand");
        Mspti::Common::RegisterFunction("libprofapi", "MsprofGetHashId");
        Mspti::Common::RegisterFunction("libprofapi", "MsprofRegTypeInfo");
        Mspti::Common::RegisterFunction("libprofapi", "profRegDeviceStateCallback");
        static const std::vector<std::pair<int, VOID_PTR>> CALLBACK_FUNC_LIST = {
            {PROFILE_REPORT_GET_HASH_ID_C_CALLBACK,
                reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiGetHashIdImpl)},
            {PROFILE_REPORT_REG_TYPE_INFO_C_CALLBACK,
                reinterpret_cast<VOID_PTR>(Mspti::Inject::Detail::MsptiRegReportTypeInfoImpl)},
        };
        for (auto& iter : CALLBACK_FUNC_LIST) {
            auto ret = Mspti::Inject::MsprofRegisterProfileCallback(iter.first, iter.second, sizeof(VOID_PTR));
            if (ret != MSPTI_SUCCESS) {
                MSPTI_LOGE("Register callback for type %d failed with error code %d.", iter.first, ret);
            } else {
                MSPTI_LOGI("Register callback for type %d successfully.", iter.first);
            }
        }
    }
    ~ProfApiInject() = default;
};

ProfApiInject g_profApiInject;

int32_t MsprofRegisterProfileCallback(int32_t callbackType, VOID_PTR callback, uint32_t len)
{
    using MsprofRegisterProfileCallbackFunc = std::function<decltype(MsprofRegisterProfileCallback)>;
    static MsprofRegisterProfileCallbackFunc func = nullptr;
    if (func == nullptr) {
        Mspti::Common::GetFunction("libprofapi", __FUNCTION__, func);
    }
    THROW_FUNC_NOTFOUND(func, __FUNCTION__, "libprofapi.so");
    return func(callbackType, callback, len);
}

int32_t profRegDeviceStateCallback(ProfSetDeviceHandle handle)
{
    using profRegDeviceStateCallbackFunc = std::function<decltype(profRegDeviceStateCallback)>;
    static profRegDeviceStateCallbackFunc func = nullptr;
    if (func == nullptr) {
        Mspti::Common::GetFunction("libprofapi", __FUNCTION__, func);
    }
    THROW_FUNC_NOTFOUND(func, __FUNCTION__, "libprofapi.so");
    return func(handle);
}

int32_t profSetProfCommand(VOID_PTR command, uint32_t len)
{
    using profSetProfCommandFunc = std::function<decltype(profSetProfCommand)>;
    static profSetProfCommandFunc func = nullptr;
    if (func == nullptr) {
        Mspti::Common::GetFunction("libprofapi", __FUNCTION__, func);
    }
    THROW_FUNC_NOTFOUND(func, __FUNCTION__, "libprofapi.so");
    return func(command, len);
}

namespace Detail {
int32_t MsprofReporterCallbackImpl(uint32_t moduleId, uint32_t type, VOID_PTR data, uint32_t len)
{
    UNUSED(moduleId);
    UNUSED(type);
    UNUSED(data);
    UNUSED(len);
    return MSPTI_SUCCESS;
}

uint64_t MsptiGetHashIdImpl(const char* hashInfo, size_t len)
{
    if (hashInfo == nullptr) {
        MSPTI_LOGE("GenHashId failed. hashInfo is nullptr");
        return 0;
    }
    return Mspti::Parser::CannHashCache::GenHashId(std::string(hashInfo, len));
}

int8_t MsptiHostFreqIsEnableImpl()
{
    constexpr int8_t enable = 1;
    constexpr int8_t disable = 0;
    return Mspti::Common::ContextManager::GetInstance()->HostFreqIsEnable() ? enable : disable;
}

int32_t MsptiApiReporterCallbackImpl(uint32_t agingFlag, const MsprofApi * const data)
{
    if (!data) {
        MSPTI_LOGE("Report Msprof Api data failed with nullptr.");
        return PROFAPI_ERROR;
    }

    switch (data->level) {
    case MSPROF_REPORT_RUNTIME_LEVEL:
        if (Mspti::Parser::CannApiParser::GetInstance().ReportRtApi(agingFlag, data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Api data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
        break;

    case MSPROF_REPORT_NODE_BASE_LEVEL: {
        if (Mspti::Parser::CannApiParser::GetInstance().ReportRtApi(agingFlag, data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Api data to ParserManager failed.");
            return PROFAPI_ERROR;
        }

        if (data->type == MSPROF_REPORT_NODE_LAUNCH_TYPE &&
            Mspti::Parser::CannTrackCache::GetInstance().AppendNodeLunch(agingFlag == 1, data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Compact data to ParserManager failed.");
            return PROFAPI_ERROR;
        }

        if (Mspti::Parser::ParserManager::GetInstance()->ReportApi(data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Api data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
    } break;

    case MSPROF_REPORT_HCCL_NODE_LEVEL:
        if (Mspti::Parser::CannTrackCache::GetInstance().AppendCommunication(agingFlag, data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Hccl Api data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
        break;
    case MSPROF_REPORT_ACL_LEVEL:
        if (Mspti::Parser::CannApiParser::GetInstance().ReportRtApi(agingFlag, data) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Acl Api data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
        break;
    default:
        break;
    }

    return PROFAPI_ERROR_NONE;
}

int32_t MsptiEventReporterCallbackImpl(uint32_t agingFlag, const MsprofEvent* const event)
{
    UNUSED(agingFlag);
    UNUSED(event);
    return PROFAPI_ERROR_NONE;
}

int32_t MsptiCompactInfoReporterCallbackImpl(uint32_t agingFlag, CONST_VOID_PTR data, uint32_t length)
{
    if (data == nullptr || length != sizeof(struct MsprofCompactInfo)) {
        MSPTI_LOGE("Report Msprof Compact failed with nullptr.");
        return PROFAPI_ERROR;
    }
    const auto* compact = reinterpret_cast<const MsprofCompactInfo*>(data);
    if (compact->level == MSPROF_REPORT_RUNTIME_LEVEL && compact->type == RT_PROFILE_TYPE_TASK_TRACK) {
        if (Mspti::Parser::KernelParser::GetInstance().ReportRtTaskTrack(agingFlag, compact) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Compact data to ParserManager failed.");
            return PROFAPI_ERROR;
        }

        if (Mspti::Parser::CannTrackCache::GetInstance().AppendTsTrack(agingFlag == 1, compact) != MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Compact data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
    }

    if (compact->level == MSPROF_REPORT_NODE_BASE_LEVEL
        && compact->type == MSPROF_REPORT_NODE_HCCL_OP_INFO_TYPE) {
        if (Mspti::Parser::CommunicationCalculator::GetInstance().AppendCompactInfo(
            agingFlag, compact)!= MSPTI_SUCCESS) {
            MSPTI_LOGE("Report Msprof Compact data to ParserManager failed.");
            return PROFAPI_ERROR;
        }
    }

    if (compact->level == MSPROF_REPORT_RUNTIME_LEVEL
        && compact->type == MSPROF_STREAM_EXPAND_SPEC_TYPE) {
        Mspti::Convert::StarsCommon::SetStreamExpandStatus(compact->data.streamExpandInfo.expandStatus);
    }

    return PROFAPI_ERROR_NONE;
}

int32_t MsptiAddiInfoReporterCallbackImpl(uint32_t agingFlag, CONST_VOID_PTR data, uint32_t length)
{
    UNUSED(agingFlag);
    UNUSED(data);
    UNUSED(length);
    return PROFAPI_ERROR_NONE;
}

int32_t MsptiRegReportTypeInfoImpl(uint16_t level, uint32_t typeId, const char* name, size_t len)
{
    if (name == nullptr) {
        MSPTI_LOGE("RegTypeInfo failed. name is nullptr");
        return PROFAPI_ERROR;
    }
    Mspti::Parser::CannHashCache::RegTypeHashInfo(level, typeId, std::string(name, len));
    return PROFAPI_ERROR_NONE;
}

int32_t MsprofDeviceStateImpl(VOID_PTR deviceState, uint32_t len)
{ 
    if (deviceState == nullptr || len != sizeof(ProfSetDevPara)) {
        MSPTI_LOGE("Device state callback receive invalid data.");
        return PROFAPI_ERROR;
    }
    ProfSetDevPara* devPara = reinterpret_cast<ProfSetDevPara*>(deviceState);
    MSPTI_LOGI("Receive device state callback, chipId: %u, devId: %u, isOpen: %d.",
        devPara->chipId, devPara->devId, devPara->isOpen);
    if (devPara->isOpen) {
        Mspti::Activity::ActivityManager::GetInstance()->SetDevice(devPara->devId);
    }
    return MSPTI_SUCCESS;
}
} // namespace Detail
} // namespace Inject
} // namespace Mspti
