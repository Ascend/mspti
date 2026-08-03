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

#include "csrc/common/runtime_utils.h"

#include <regex>
#include <utility>

#include "csrc/common/inject/acl_inject.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"
#include "csrc/include/mspti_activity.h"
#include "csrc/include/mspti_result.h"

namespace Mspti
{
namespace Common
{
namespace
{
#pragma pack(1)
struct TraceData
{
    uint64_t indexId;
    uint64_t modelId;
    uint16_t tagId;
};
}  // namespace

uint32_t GetDeviceId()
{
    int32_t deviceId = 0;
    if (aclrtGetDevice(&deviceId) != MSPTI_SUCCESS)
    {
        return MSPTI_INVALID_DEVICE_ID;
    }
    return static_cast<uint32_t>(deviceId);
}

uint32_t GetStreamId(AclrtStream stm)
{
    int32_t streamId = 0;
    if (aclrtStreamGetId(stm, &streamId) != MSPTI_SUCCESS)
    {
        return MSPTI_INVALID_STREAM_ID;
    }
    return static_cast<uint32_t>(streamId);
}

AclError ProfTrace(uint64_t indexId, uint64_t modelId, uint16_t tagId, AclrtStream stream)
{
    TraceData traceData{.indexId = indexId, .modelId = modelId, .tagId = tagId};
    return aclrtProfTrace((void*)&traceData, sizeof(TraceData), stream);
}

std::string GetCANNModuleVersion(const std::string& module)
{
    try
    {
        constexpr size_t ACL_PKG_VERSION_MAX_SIZE = 128;
        char versionStr[ACL_PKG_VERSION_MAX_SIZE] = {0};
        auto ret = aclsysGetVersionStr(const_cast<char*>(module.c_str()), versionStr);
        return ret == MSPTI_SUCCESS ? std::string(versionStr) : "";
    }
    catch (const std::exception& e)
    {
        MSPTI_LOGE("GetCANNModuleVersion failed, %s.", e.what());
        return "";
    }
}

bool IsRuntimeSupportMemoryReport()
{
    static bool isSupport = []() -> bool
    {
        std::string versionStr = GetCANNModuleVersion("runtime");
        if (versionStr.empty())
        {
            return false;
        }
        static const std::regex reg(R"(^(\d+)\.(\d+))");
        std::smatch match;
        std::pair<int32_t, int32_t> version = {0, 0};
        if (std::regex_search(versionStr, match, reg) && Utils::StrToI32(version.first, match[1].str()) &&
            Utils::StrToI32(version.second, match[2].str()))
        {
            constexpr std::pair<int32_t, int32_t> MIN_VERSION = {9, 2};
            return version >= MIN_VERSION;
        }
        return false;
    }();
    return isSupport;
}
}  // namespace Common
}  // namespace Mspti
