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

#ifndef MSPTI_ACTIVITY_ASCEND_PARSER_MEMORY_PARSER_H
#define MSPTI_ACTIVITY_ASCEND_PARSER_MEMORY_PARSER_H

#include <memory>

#include "csrc/common/inject/profapi_inject.h"
#include "csrc/include/mspti_result.h"

namespace Mspti
{
namespace Parser
{

class MemoryParser
{
   public:
    static MemoryParser &GetInstance();

    // Memory Activity
    msptiResult ReportMemory(const MsprofCompactInfo &record);
    // Memset Activity
    msptiResult ReportMemset(const MsprofCompactInfo &record);
    // Memcpy Activity
    msptiResult ReportMemcpy(const MsprofCompactInfo &record);
    // Record Api Event
    msptiResult RecordApi(const MsprofApi &api);

   private:
    MemoryParser();
    ~MemoryParser() = default;
    class MemoryParserImpl;
    std::unique_ptr<MemoryParserImpl> pImpl;
};

}  // namespace Parser
}  // namespace Mspti

#endif  // MSPTI_ACTIVITY_ASCEND_PARSER_MEMORY_PARSER_H
