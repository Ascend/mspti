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

#include "csrc/activity/ascend/parser/parser_manager.h"

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/activity/ascend/reporter/external_correlation_reporter.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{

ParserManager* ParserManager::GetInstance()
{
    static ParserManager instance;
    return &instance;
}

msptiResult ParserManager::ReportApi(const MsprofApi* const data)
{
    if (!data)
    {
        return MSPTI_ERROR_INNER;
    }
    const auto& name = CannHashCache::GetHashInfo(data->itemId);
    if (name.empty())
    {
        MSPTI_LOGW("Get HashInfo failed. HashId: %lu", data->itemId);
        return MSPTI_SUCCESS;
    }

    if (!Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_API))
    {
        return MSPTI_SUCCESS;
    }

    msptiActivityApi api{};
    api.kind = MSPTI_ACTIVITY_KIND_API;
    api.pt.processId = Mspti::Common::Utils::GetPid();
    api.name = name.data();
    api.pt.threadId = data->threadId;
    api.start = Mspti::Common::ContextManager::GetInstance()->GetHostRealTime(data->beginTime);
    api.end = Mspti::Common::ContextManager::GetInstance()->GetHostRealTime(data->endTime);
    api.correlationId = Mspti::Common::ContextManager::GetInstance()->GetCorrelationId(data->threadId);
    if (Mspti::Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&api),
                                                                sizeof(msptiActivityApi)) != MSPTI_SUCCESS)
    {
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

void ParserManager::ReportStepTrace(uint32_t deviceId, const StepTraceBasic* stepTrace)
{
    if (!stepTrace)
    {
        return;
    }
    switch (stepTrace->tagId)
    {
        case STEP_TRACE_TAG_MARKEX:
            MstxParser::GetInstance()->ReportMarkDataToActivity(deviceId, stepTrace);
            break;
        default:
            break;
    }
}

msptiResult ParserManager::StartAnalysisTask(msptiActivityKind kind) { return GetAnalysisTask(kind)->StartTask(); }

msptiResult ParserManager::StartAnalysisTasks(const std::array<std::atomic<bool>, MSPTI_ACTIVITY_KIND_COUNT>& kinds)
{
    for (int kindIndex = 0; kindIndex < MSPTI_ACTIVITY_KIND_COUNT; kindIndex++)
    {
        if (!kinds[kindIndex])
        {
            continue;
        }
        auto kind = static_cast<msptiActivityKind>(kindIndex);
        if (StartAnalysisTask(kind) != MSPTI_SUCCESS)
        {
            return MSPTI_ERROR_INNER;
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult ParserManager::StopAnalysisTask(msptiActivityKind kind) { return GetAnalysisTask(kind)->StopTask(); }

msptiResult ParserManager::StopAnalysisTasks(const std::array<std::atomic<bool>, MSPTI_ACTIVITY_KIND_COUNT>& kinds)
{
    for (int kindIndex = 0; kindIndex < MSPTI_ACTIVITY_KIND_COUNT; kindIndex++)
    {
        if (!kinds[kindIndex])
        {
            continue;
        }
        auto kind = static_cast<msptiActivityKind>(kindIndex);
        if (StopAnalysisTask(kind) != MSPTI_SUCCESS)
        {
            return MSPTI_ERROR_INNER;
        }
    }
    return MSPTI_SUCCESS;
}

Mspti::Parser::ProfTask* ParserManager::GetAnalysisTask(msptiActivityKind kind)
{
    switch (kind)
    {
        case MSPTI_ACTIVITY_KIND_COMMUNICATION:
            return &CannTrackCache::GetInstance();
        default:
            return &Mspti::Parser::NullProfTask::GetInstance();
    }
}
}  // namespace Parser
}  // namespace Mspti
