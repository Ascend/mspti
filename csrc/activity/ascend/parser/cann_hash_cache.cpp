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

#include "csrc/activity/ascend/parser/cann_hash_cache.h"

#include "csrc/common/utils.h"

namespace Mspti
{
namespace Parser
{

Common::ConcurrentMap<uint64_t, std::string> &CannHashCache::HashInfoMap()
{
    static auto *map = new Common::ConcurrentMap<uint64_t, std::string>();
    return *map;
}

Common::ConcurrentMap<std::pair<uint16_t, uint32_t>, std::string, Common::PairHash> &CannHashCache::TypeHashIdMap()
{
    static auto *map = new Common::ConcurrentMap<std::pair<uint16_t, uint32_t>, std::string, Common::PairHash>();
    return *map;
}

uint64_t CannHashCache::GenHashId(const std::string &hashInfo)
{
    uint64_t hashId = Common::GetHashIdImple(hashInfo);
    HashInfoMap().Insert(hashId, hashInfo);
    return hashId;
}

uint64_t CannHashCache::GenHashId(std::string &&hashInfo)
{
    // DoubleHash耗时和map find的耗时比较
    uint64_t hashId = Common::GetHashIdImple(hashInfo);
    HashInfoMap().Emplace(hashId, std::move(hashInfo));
    return hashId;
}

std::string &CannHashCache::GetHashInfo(uint64_t hashId)
{
    static auto &nullInfo = *new std::string();
    auto guard = HashInfoMap().GetGuard(hashId);
    const auto iter = guard->UnSafeFind(hashId);
    if (iter.second)
    {
        return iter.first->second;
    }
    return nullInfo;
}

std::string &CannHashCache::GetTypeHashInfo(uint16_t level, uint32_t typeId)
{
    static auto &nullInfo = *new std::string();
    std::pair<uint16_t, uint32_t> key = {level, typeId};
    auto guard = TypeHashIdMap().GetGuard(key);
    const auto iter = guard->UnSafeFind(key);
    if (iter.second)
    {
        return iter.first->second;
    }
    return nullInfo;
}

void CannHashCache::RegTypeHashInfo(uint16_t level, uint32_t typeId, std::string &&hashInfo)
{
    std::pair<uint16_t, uint32_t> key = {level, typeId};
    TypeHashIdMap().Emplace(key, std::forward<std::string>(hashInfo));
}

void CannHashCache::RegTypeHashInfo(uint16_t level, uint32_t typeId, const std::string &hashInfo)
{
    std::pair<uint16_t, uint32_t> key = {level, typeId};
    TypeHashIdMap().Insert(key, hashInfo);
}
}  // namespace Parser
}  // namespace Mspti
