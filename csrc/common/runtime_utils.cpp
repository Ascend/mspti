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

#include <cctype>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

inline bool IsAsciiDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

// Parse one run of ASCII digits starting at pos; advances pos past the digits.
bool ParseNumericPart(const std::string& str, size_t& pos, std::string& part)
{
    const size_t start = pos;
    while (pos < str.size() && IsAsciiDigit(str[pos]))
    {
        ++pos;
    }
    if (pos == start)
    {
        return false;
    }
    part.assign(str, start, pos - start);
    return true;
}

// Parse leading dot-separated numeric parts at string start, e.g. "9.2.0" with
// partCount == 3 fills parts with {"9", "2", "0"}.
// partCount explicitly specifies how many parts to parse; parts is an out-param
// resized internally, so the caller only declares an empty vector and does not
// need to pre-allocate its size. Returns false when partCount is 0 or the
// prefix does not contain that many numeric parts.
bool ParseLeadingNumericParts(const std::string& str, std::vector<std::string>& parts, size_t partCount)
{
    if (partCount == 0)
    {
        parts.clear();
        return false;
    }
    parts.clear();
    parts.resize(partCount);
    size_t pos = 0;
    for (size_t i = 0; i < partCount; ++i)
    {
        if (!ParseNumericPart(str, pos, parts[i]))
        {
            return false;
        }
        if (i + 1 < partCount)
        {
            if (pos >= str.size() || str[pos] != '.')
            {
                return false;
            }
            ++pos;
        }
    }
    return true;
}
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
        std::vector<std::string> parts(2);
        std::pair<int32_t, int32_t> version = {0, 0};
        if (ParseLeadingNumericParts(versionStr, parts, parts.size()) && Utils::StrToI32(version.first, parts[0]) &&
            Utils::StrToI32(version.second, parts[1]))
        {
            constexpr std::pair<int32_t, int32_t> MIN_VERSION = {9, 2};
            return version >= MIN_VERSION;
        }
        return false;
    }();
    return isSupport;
}

uint32_t ParseMsptiVersion(const std::string& versionStr)
{
    constexpr uint32_t INVALID_VERSION = 0;
    if (versionStr.empty())
    {
        return INVALID_VERSION;
    }
    std::vector<std::string> parts(3);
    if (!ParseLeadingNumericParts(versionStr, parts, parts.size()))
    {
        MSPTI_LOGE("mspti version str: %s is invalid.", versionStr.c_str());
        return INVALID_VERSION;
    }
    uint32_t major{0};
    uint32_t minor{0};
    uint32_t patch{0};
    if (!Common::Utils::StrToU32(major, parts[0]) || !Common::Utils::StrToU32(minor, parts[1]) ||
        !Common::Utils::StrToU32(patch, parts[2]))
    {
        MSPTI_LOGE("mspti version str: %s is invalid.", versionStr.c_str());
        return INVALID_VERSION;
    }
    // major * 10000 + minor * 100 + patch is computed in uint32_t; e.g. major = UINT32_MAX
    // would silently wrap around. Compute in 64 bits and reject on overflow.
    const uint64_t version = static_cast<uint64_t>(major) * 10000 + static_cast<uint64_t>(minor) * 100 + patch;
    if (version > UINT32_MAX)
    {
        MSPTI_LOGE("mspti version str: %s is invalid.", versionStr.c_str());
        return INVALID_VERSION;
    }
    return static_cast<uint32_t>(version);
}
}  // namespace Common
}  // namespace Mspti
