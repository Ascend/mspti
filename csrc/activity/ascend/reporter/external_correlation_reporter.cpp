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

#include "csrc/activity/ascend/reporter/external_correlation_reporter.h"

#include <atomic>
#include <vector>

#include "csrc/activity/activity_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/shutdown_flag.h"
#include "csrc/common/utils.h"
#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Reporter
{

ExternalCorrelationReporter* ExternalCorrelationReporter::GetInstance()
{
    static ExternalCorrelationReporter instance;
    return &instance;
}

ExternalCorrelationReporter::~ExternalCorrelationReporter() { Common::SetShuttingDown(true); }

ExternalCorrelationReporter::ThreadRegistration::ThreadRegistration()
    : tid(Common::Utils::GetTid()), stacks(std::make_shared<ThreadStacks>())
{
}

ExternalCorrelationReporter::ThreadRegistration::~ThreadRegistration()
{
    // An in-flight reporter may still own the stacks. It must not keep this
    // source TID registered after its thread has exited.
    // During process shutdown the singleton may already be in destruction.
    if (!Common::IsShuttingDown())
    {
        ExternalCorrelationReporter::GetInstance()->UnregisterThread(tid, stacks);
    }
}

std::shared_ptr<ExternalCorrelationReporter::ThreadStacks> ExternalCorrelationReporter::GetCallingThreadStacks()
{
    static thread_local ThreadRegistration registration;
    return registration.stacks;
}

void ExternalCorrelationReporter::RegisterThread(uint32_t tid, const std::shared_ptr<ThreadStacks>& stacks)
{
    std::lock_guard<std::mutex> lock(mapMtx_);
    // O(1) average insertion; no full registry sweep on repeated Push/Pop.
    externalCorrelationByThread_[tid] = stacks;
}

void ExternalCorrelationReporter::UnregisterThread(uint32_t tid, const std::shared_ptr<ThreadStacks>& stacks)
{
    std::lock_guard<std::mutex> lock(mapMtx_);
    auto iter = externalCorrelationByThread_.find(tid);
    // A delayed destructor must never remove a newer registration for the same TID.
    if (iter != externalCorrelationByThread_.end() && iter->second.lock() == stacks)
    {
        externalCorrelationByThread_.erase(iter);
    }
}

msptiResult ExternalCorrelationReporter::ReportExternalCorrelationId(uint64_t correlationId, uint32_t tid)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION))
    {
        return MSPTI_SUCCESS;
    }

    std::shared_ptr<ThreadStacks> stacks;
    bool expiredRegistration = false;
    {
        std::lock_guard<std::mutex> lock(mapMtx_);
        auto iter = externalCorrelationByThread_.find(tid);
        if (iter != externalCorrelationByThread_.end())
        {
            stacks = iter->second.lock();
            if (!stacks)
            {
                expiredRegistration = true;
                externalCorrelationByThread_.erase(iter);
            }
        }
    }
    if (!stacks)
    {
        if (expiredRegistration)
        {
            static std::atomic_flag warnedExpired = ATOMIC_FLAG_INIT;
            if (!warnedExpired.test_and_set(std::memory_order_relaxed))
            {
                MSPTI_LOGW("External correlation state expired for source tid %u; no external record emitted.", tid);
            }
        }
        else
        {
            // A missing registration is normal when no external scope was pushed.
            static std::atomic_flag loggedMissing = ATOMIC_FLAG_INIT;
            if (!loggedMissing.test_and_set(std::memory_order_relaxed))
            {
                MSPTI_LOGD("No external correlation state for source tid %u; this may be expected.", tid);
            }
        }
        return MSPTI_SUCCESS;
    }

    // Snapshot the source thread's active scopes without holding either lock
    // while ActivityManager::Record calls into the buffer / user callbacks.
    std::vector<std::pair<msptiExternalCorrelationKind, uint64_t>> active;
    {
        std::lock_guard<std::mutex> lock(stacks->mutex);
        for (const auto& pair : stacks->stacks)
        {
            if (!pair.second.empty())
            {
                active.emplace_back(pair.first, pair.second.top());
            }
        }
    }
    for (const auto& pair : active)
    {
        msptiActivityExternalCorrelation result{};
        result.kind = MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION;
        result.externalKind = pair.first;
        result.externalId = pair.second;
        result.correlationId = correlationId;
        if (Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&result),
                                                             sizeof(msptiActivityExternalCorrelation)) != MSPTI_SUCCESS)
        {
            return MSPTI_ERROR_INNER;
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult ExternalCorrelationReporter::PushExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t id)
{
    auto stacks = GetCallingThreadStacks();
    {
        std::lock_guard<std::mutex> lock(stacks->mutex);
        stacks->stacks[kind].push(id);
    }
    RegisterThread(Common::Utils::GetTid(), stacks);
    return MSPTI_SUCCESS;
}

msptiResult ExternalCorrelationReporter::PopExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t* lastId)
{
    if (lastId == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    auto stacks = GetCallingThreadStacks();
    bool empty = false;
    {
        std::lock_guard<std::mutex> lock(stacks->mutex);
        auto iter = stacks->stacks.find(kind);
        if (iter == stacks->stacks.end())
        {
            return MSPTI_ERROR_QUEUE_EMPTY;
        }
        *lastId = iter->second.top();
        iter->second.pop();
        if (iter->second.empty())
        {
            stacks->stacks.erase(iter);
        }
        empty = stacks->stacks.empty();
    }
    if (empty)
    {
        std::lock_guard<std::mutex> lock(mapMtx_);
        auto iter = externalCorrelationByThread_.find(Common::Utils::GetTid());
        if (iter != externalCorrelationByThread_.end() && iter->second.lock() == stacks)
        {
            externalCorrelationByThread_.erase(iter);
        }
    }
    return MSPTI_SUCCESS;
}
}  // namespace Reporter
}  // namespace Mspti
