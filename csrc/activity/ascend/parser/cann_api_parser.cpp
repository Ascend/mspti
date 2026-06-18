/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
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

#include "csrc/activity/ascend/parser/cann_api_parser.h"

#include <mutex>
#include <vector>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/thread_local.h"

namespace Mspti
{
namespace Parser
{
namespace
{
inline msptiActivityKind level2ApiKind(uint16_t level)
{
    switch (level)
    {
        case MSPROF_REPORT_ACL_LEVEL:
            return MSPTI_ACTIVITY_KIND_ACL_API;
        case MSPROF_REPORT_RUNTIME_LEVEL:
            return MSPTI_ACTIVITY_KIND_RUNTIME_API;
        case MSPROF_REPORT_NODE_BASE_LEVEL:
            return MSPTI_ACTIVITY_KIND_NODE_API;
        default:
            return MSPTI_ACTIVITY_KIND_API;
    }
}
}  // namespace

class CannApiParser::CannApiParserImpl
{
   public:
    CannApiParserImpl() = default;
    ~CannApiParserImpl() = default;
    msptiResult ReportApi(uint32_t agingFlag, const MsprofApi* data)
    {
        UNUSED(agingFlag);
        auto kind = level2ApiKind(data->level);
        if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(kind))
        {
            return MSPTI_SUCCESS;
        }
        const auto& name = (data->level == MSPROF_REPORT_RUNTIME_LEVEL || data->level == MSPROF_REPORT_ACL_LEVEL)
                               ? CannHashCache::GetTypeHashInfo(data->level, data->type)
                               : CannHashCache::GetHashInfo(data->itemId);
        if (name.empty())
        {
            MSPTI_LOGW("Get HashInfo failed. HashId: %lu", data->itemId);
            return MSPTI_SUCCESS;
        }

        msptiActivityApi api{};
        api.kind = kind;
        api.pt.processId = Mspti::Common::Utils::GetPid();
        api.name = name.data();
        api.pt.threadId = data->threadId;
        api.start = Mspti::Common::ContextManager::GetInstance()->GetRealTimeFromSysCnt(data->beginTime);
        api.end = Mspti::Common::ContextManager::GetInstance()->GetRealTimeFromSysCnt(data->endTime);
        api.correlationId = Mspti::Common::ContextManager::GetInstance()->GetCorrelationId(data->threadId);
        if (Mspti::Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&api),
                                                                    sizeof(msptiActivityApi)) != MSPTI_SUCCESS)
        {
            return MSPTI_ERROR_INNER;
        }
        return MSPTI_SUCCESS;
    }
};

CannApiParser& CannApiParser::GetInstance()
{
    static CannApiParser instance;
    return instance;
}

msptiResult CannApiParser::ReportApi(uint32_t agingFlag, const MsprofApi* data)
{
    return pImpl->ReportApi(agingFlag, data);
}

CannApiParser::CannApiParser() : pImpl(std::make_unique<CannApiParserImpl>()) {}

CannApiParser::~CannApiParser() = default;
}  // namespace Parser
}  // namespace Mspti
