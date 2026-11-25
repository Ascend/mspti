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

#ifndef MSPTI_PARSER_CANN_HASH_CACHE_H
#define MSPTI_PARSER_CANN_HASH_CACHE_H

#include "csrc/common/concurrent_map.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{
class CannHashCache
{
   public:
    static std::string& GetHashInfo(uint64_t hashId);
    static uint64_t GenHashId(const std::string& hashInfo);
    static uint64_t GenHashId(std::string&& hashInfo);
    static std::string& GetTypeHashInfo(uint16_t level, uint32_t typeId);
    static void RegTypeHashInfo(uint16_t level, uint32_t typeId, const std::string& hashInfo);
    static void RegTypeHashInfo(uint16_t level, uint32_t typeId, std::string&& hashInfo);

   private:
    CannHashCache() = default;
    static Common::ConcurrentMap<uint64_t, std::string>& HashInfoMap();
    static Common::ConcurrentMap<std::pair<uint16_t, uint32_t>, std::string, Common::PairHash>& TypeHashIdMap();
};
}  // namespace Parser
}  // namespace Mspti

#endif  // MSPTI_PARSER_CANN_HASH_CACHE_H
