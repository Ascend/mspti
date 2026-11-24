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

#ifndef EXTERNAL_CORRELATION_REPORTER_H
#define EXTERNAL_CORRELATION_REPORTER_H

#include <map>
#include <memory>
#include <mutex>
#include <stack>
#include <unordered_map>

#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Reporter
{
class ExternalCorrelationReporter
{
   public:
    static ExternalCorrelationReporter* GetInstance();

    // tid identifies the source CANN API thread, which may differ from the reporter thread.
    msptiResult ReportExternalCorrelationId(uint64_t correlationId, uint32_t tid);
    msptiResult PushExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t id);
    msptiResult PopExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t* lastId);

   private:
    ~ExternalCorrelationReporter();
    struct ThreadStacks
    {
        std::mutex mutex;
        std::map<msptiExternalCorrelationKind, std::stack<uint64_t>> stacks;
    };

    // Thread registration ends on TLS destruction, even if an in-flight
    // report still owns the ThreadStacks object.
    struct ThreadRegistration
    {
        ThreadRegistration();
        ~ThreadRegistration();

        uint32_t tid;
        std::shared_ptr<ThreadStacks> stacks;
    };

    static std::shared_ptr<ThreadStacks> GetCallingThreadStacks();
    void RegisterThread(uint32_t tid, const std::shared_ptr<ThreadStacks>& stacks);
    void UnregisterThread(uint32_t tid, const std::shared_ptr<ThreadStacks>& stacks);

    std::mutex mapMtx_;
    // TLS deregistration prevents stale TID reuse while preserving weak ownership.
    std::unordered_map<uint32_t, std::weak_ptr<ThreadStacks>> externalCorrelationByThread_;
};
}  // namespace Reporter
}  // namespace Mspti

#endif  // EXTERNAL_CORRELATION_REPORTER_H
