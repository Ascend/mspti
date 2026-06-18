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
#ifndef RUNTIME_API_PARSER_H
#define RUNTIME_API_PARSER_H

#include <memory>

#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/entity/soclog.h"
#include "csrc/common/inject/profapi_inject.h"
#include "csrc/include/mspti_activity.h"
#include "csrc/include/mspti_result.h"

namespace Mspti
{
namespace Parser
{
class CannApiParser
{
   public:
    static CannApiParser &GetInstance();
    msptiResult ReportApi(uint32_t agingFlag, const MsprofApi *data);

   private:
    CannApiParser();
    ~CannApiParser();
    class CannApiParserImpl;
    std::unique_ptr<CannApiParserImpl> pImpl;
};
}  // namespace Parser
}  // namespace Mspti

#endif
