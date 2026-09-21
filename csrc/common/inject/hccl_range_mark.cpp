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
#include "csrc/common/inject/hccl_range_mark.h"

#include <cstring>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/hccl_reporter.h"
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{
using namespace Mspti::Activity;
HcclInnerMark::HcclInnerMark(aclrtStream stream, HcclComm comm, std::shared_ptr<HcclOpDesc> opDesc)
{
    // HcclGetCommName has no size parameter. Allocate its documented maximum on
    // the heap and keep one extra byte for a terminator so a malformed response
    // cannot cause an unbounded read when the name is copied.
    auto commName = std::make_unique<char[]>(COMM_NAME_MAX_LENGTH + 1);
    const HcclResult ret = HcclGetCommName(comm, commName.get());
    if (ret == HCCL_SUCCESS)
    {
        commName[COMM_NAME_MAX_LENGTH] = '\0';
        opDesc->commName.assign(commName.get(), strnlen(commName.get(), COMM_NAME_MAX_LENGTH));
    }
    else
    {
        MSPTI_LOGW("HcclGetCommName failed, ret: %d.", ret);
    }
    Mspti::Parser::MstxParser::GetInstance()->InnerDeviceStartA(stream, markId);
    Mspti::Parser::HcclReporter::GetInstance()->RecordHcclOp(markId, opDesc);
};

HcclInnerMark::~HcclInnerMark() { Mspti::Parser::MstxParser::GetInstance()->InnerDeviceEndA(markId); }

std::unique_ptr<InnerMark> HcclMarkFactory::createMarker(aclrtStream stream, HcclComm comm,
                                                         std::shared_ptr<HcclOpDesc> opDesc)
{
    if (ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_HCCL))
    {
        return std::move(
            static_cast<std::unique_ptr<InnerMark>>(std::make_unique<HcclInnerMark>(stream, comm, opDesc)));
    }
    return std::make_unique<InnerMark>();
}
}  // namespace Parser
}  // namespace Mspti
