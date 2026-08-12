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

#include "csrc/activity/ascend/reporter/overhead_reporter.h"

#include "csrc/activity/activity_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Reporter
{
OverheadRecord::OverheadRecord(msptiActivityOverheadKind kind, msptiActivityObjectKind objectKind)
    : overheadKind_(kind), objectKind_(objectKind), start_(Common::ContextManager::GetInstance()->GetHostTimeStampNs())
{
}

OverheadRecord::~OverheadRecord()
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_OVERHEAD))
    {
        return;
    }
    msptiActivityOverhead activityOverhead{};
    activityOverhead.kind = MSPTI_ACTIVITY_KIND_OVERHEAD;
    activityOverhead.overheadKind = overheadKind_;
    activityOverhead.objectKind = objectKind_;
    activityOverhead.start = start_;
    activityOverhead.end = Common::ContextManager::GetInstance()->GetHostTimeStampNs();
    activityOverhead.correlationId = MSPTI_INVALID_CORRELATION_ID;
    activityOverhead.overheadData = nullptr;

    switch (objectKind_)
    {
        case MSPTI_ACTIVITY_OBJECT_DEVICE:
            activityOverhead.objectId.ds.deviceId = deviceId_;
            activityOverhead.objectId.ds.streamId = MSPTI_INVALID_STREAM_ID;
            break;
        case MSPTI_ACTIVITY_OBJECT_PROCESS:
        case MSPTI_ACTIVITY_OBJECT_THREAD:
            activityOverhead.objectId.pt.processId = Common::Utils::GetPid();
            activityOverhead.objectId.pt.threadId = Common::Utils::GetTid();
            break;
        default:
            activityOverhead.objectId.pt.processId = 0;
            activityOverhead.objectId.pt.threadId = 0;
            MSPTI_LOGW("objectKind %d is not supported", objectKind_);
            break;
    }

    if (Activity::ActivityManager::GetInstance()->Record(Common::ReinterpretConvert<msptiActivity*>(&activityOverhead),
                                                         sizeof(msptiActivityOverhead)) != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("ReportActivityOverhead fail, please check buffer");
    }
}
}  // namespace Reporter
}  // namespace Mspti
