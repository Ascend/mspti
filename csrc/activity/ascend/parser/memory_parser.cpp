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

#include "csrc/activity/ascend/parser/memory_parser.h"

#include <algorithm>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/object_pool.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Parser
{
namespace
{
constexpr uint32_t DEFAULT_POOL_SIZE = 128;
constexpr uint32_t MEMCPY_POOL_SIZE = 256;

const std::unordered_set<std::string> ACL_RT_MEMORY_API_WHITE_LIST = {"aclrtMalloc",
                                                                      "aclrtMallocAlign32",
                                                                      "aclrtMallocCached",
                                                                      "aclrtMallocWithCfg",
                                                                      "aclrtMallocForTaskScheduler",
                                                                      "aclrtFree",
                                                                      "aclrtFreeWithDevSync",
                                                                      "aclrtMallocHost",
                                                                      "aclrtFreeHost",
                                                                      "aclrtMallocHostWithCfg",
                                                                      "aclrtFreeHostWithDevSync",
                                                                      "aclrtReserveMemAddress",
                                                                      "aclrtReserveMemAddressNoUCMemory",
                                                                      "aclrtReleaseMemAddress",
                                                                      "aclrtMemAllocManaged",
                                                                      "aclrtMallocPhysical",
                                                                      "aclrtFreePhysical"};

const std::unordered_set<std::string> ACL_RT_MEMSET_API_WHITE_LIST = {"aclrtMemset", "aclrtMemsetAsync",
                                                                      "aclrtMemsetD32", "aclrtMemsetD32Async"};

const std::unordered_set<std::string> ACL_RT_MEMCPY_API_WHITE_LIST = {"aclrtMemcpy",
                                                                      "aclrtMemcpyAsync",
                                                                      "aclrtMemcpyAsyncWithCondition",
                                                                      "aclrtMemcpyBatch",
                                                                      "aclrtMemcpyBatchAsync",
                                                                      "aclrtMemcpy2d",
                                                                      "aclrtMemcpy2dAsync",
                                                                      "aclrtMemcpyAsyncWithOffset",
                                                                      "aclrtMemcpyBatchV2",
                                                                      "aclrtMemcpyBatchAsyncV2"};

inline bool IsInApiWhiteList(const std::string &apiName, const std::unordered_set<std::string> &whiteList)
{
    return whiteList.find(apiName) != whiteList.end();
}

enum class RtMemcpyKind
{
    RT_MEMCPY_HOST_TO_HOST = 0,  // host to host
    RT_MEMCPY_HOST_TO_DEVICE,    // host to device
    RT_MEMCPY_DEVICE_TO_HOST,    // device to host
    RT_MEMCPY_DEVICE_TO_DEVICE,  // device to device, 1P && P2P
    RT_MEMCPY_MANAGED,           // managed memory
    RT_MEMCPY_ADDR_DEVICE_TO_DEVICE,
    RT_MEMCPY_HOST_TO_DEVICE_EX,  // host  to device ex (only used for 8 bytes)
    RT_MEMCPY_DEVICE_TO_HOST_EX,  // device to host ex
    RT_MEMCPY_DEFAULT,            // auto infer copy dir
    RT_MEMCPY_RESERVED            // reserved
};

inline msptiActivityMemcpyKind GetMsptiMemcpyKind(RtMemcpyKind rtMemcpykind)
{
    static const std::unordered_map<RtMemcpyKind, msptiActivityMemcpyKind> memoryKindMap = {
        {RtMemcpyKind::RT_MEMCPY_HOST_TO_HOST, MSPTI_ACTIVITY_MEMCPY_KIND_HTOH},
        {RtMemcpyKind::RT_MEMCPY_HOST_TO_DEVICE, MSPTI_ACTIVITY_MEMCPY_KIND_HTOD},
        {RtMemcpyKind::RT_MEMCPY_DEVICE_TO_HOST, MSPTI_ACTIVITY_MEMCPY_KIND_DTOH},
        {RtMemcpyKind::RT_MEMCPY_DEVICE_TO_DEVICE, MSPTI_ACTIVITY_MEMCPY_KIND_DTOD},
        {RtMemcpyKind::RT_MEMCPY_MANAGED, MSPTI_ACTIVITY_MEMCPY_KIND_DEFAULT},
        {RtMemcpyKind::RT_MEMCPY_ADDR_DEVICE_TO_DEVICE, MSPTI_ACTIVITY_MEMCPY_KIND_DTOD},
        {RtMemcpyKind::RT_MEMCPY_HOST_TO_DEVICE_EX, MSPTI_ACTIVITY_MEMCPY_KIND_HTOD},
        {RtMemcpyKind::RT_MEMCPY_DEVICE_TO_HOST_EX, MSPTI_ACTIVITY_MEMCPY_KIND_DTOH},
        {RtMemcpyKind::RT_MEMCPY_DEFAULT, MSPTI_ACTIVITY_MEMCPY_KIND_DEFAULT},
    };
    auto iter = memoryKindMap.find(rtMemcpykind);
    return (iter == memoryKindMap.end() ? MSPTI_ACTIVITY_MEMCPY_KIND_UNKNOWN : iter->second);
}

enum class RuntimeMemMngType : uint16_t
{
    RUNTIME_MEMMNG_MALLOC = 0U,
    RUNTIME_MEMMNG_FREE = 1U,
};

inline msptiActivityMemoryOperationType GetMsptiMemoryOperationType(RuntimeMemMngType rtMemMngType)
{
    return (rtMemMngType == RuntimeMemMngType::RUNTIME_MEMMNG_MALLOC ? MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_ALLOCATION
                                                                     : MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_RELEASE);
}

enum class RtMemoryType
{
    RT_MEMORY_TYPE_UNKNOWN = 0,
    RT_MEMORY_TYPE_HOST = 1,
    RT_MEMORY_TYPE_DEVICE = 2,
    RT_MEMORY_TYPE_MANAGED = 3
};

inline msptiActivityMemoryKind GetMsptiMemoryKind(RtMemoryType rtMemoryType)
{
    switch (rtMemoryType)
    {
        case RtMemoryType::RT_MEMORY_TYPE_HOST:
            return MSPTI_ACTIVITY_MEMORY_HOST;
        case RtMemoryType::RT_MEMORY_TYPE_DEVICE:
            return MSPTI_ACTIVITY_MEMORY_DEVICE;
        case RtMemoryType::RT_MEMORY_TYPE_MANAGED:
            return MSPTI_ACTIVITY_MEMORY_MANAGED;
        default:
            return MSPTI_ACTIVITY_MEMORY_UNKNOWN;
    }
}

inline uint32_t CheckStreamId(uint32_t streamId)
{
    return (streamId == UINT32_MAX ? MSPTI_INVALID_STREAM_ID : streamId);
}

inline bool IsAclRtsApi(uint32_t apiType)
{
    constexpr uint32_t API_TYPE_OFFSET = 16;
    return (apiType >> API_TYPE_OFFSET) == ACL_RTS;
}
}  // namespace

class MemoryParser::MemoryParserImpl
{
    template <typename T>
    using ObjectPtr = typename Common::SimpleObjectPool<T>::Ptr;

    template <typename T>
    using ObjectCache = std::unordered_map<uint32_t, std::deque<ObjectPtr<T>>>;

   public:
    MemoryParserImpl()
        : memoryActivityPool_(DEFAULT_POOL_SIZE),
          memsetActivityPool_(DEFAULT_POOL_SIZE),
          memcpyActivityPool_(MEMCPY_POOL_SIZE) {};
    ~MemoryParserImpl() = default;
    msptiResult ReportMemory(const MsprofCompactInfo &record);
    msptiResult ReportMemcpy(const MsprofCompactInfo &record);
    msptiResult ReportMemset(const MsprofCompactInfo &record);
    msptiResult RecordApi(const MsprofApi &api);

   private:
    template <typename T, typename F>
    msptiResult ReportImpl(Common::SimpleObjectPool<T> &pool, std::mutex &mtx, ObjectCache<T> &cache,
                           msptiActivityKind kind, const MsprofCompactInfo &record, F &&fillFields)
    {
        auto activity = pool.acquire();
        if (UNLIKELY(activity == nullptr))
        {
            MSPTI_LOGE("Acquire object from pool failed for kind %d.", static_cast<int>(kind));
            return MSPTI_ERROR_INNER;
        }
        activity->kind = kind;
        activity->correlationId = Common::ContextManager::GetInstance()->GetCorrelationId();
        activity->start = record.timeStamp;
        activity->end = record.timeStamp;

        std::forward<F>(fillFields)(activity, record);

        {
            std::lock_guard<std::mutex> lock(mtx);
            cache[record.threadId].emplace_back(std::move(activity));
        }
        return MSPTI_SUCCESS;
    }

    template <typename T>
    msptiResult RecordApiImpl(std::mutex &mtx, ObjectCache<T> &cache, const MsprofApi &api, const char *logMsg)
    {
        auto ret = MSPTI_SUCCESS;
        std::lock_guard<std::mutex> lock(mtx);
        auto threadData = cache.find(api.threadId);
        if (threadData == cache.end())
        {
            MSPTI_LOGW("Thread %u not found in cache.", api.threadId);
            return ret;
        }
        auto &data = threadData->second;
        auto cmpLower = [](const ObjectPtr<T> &ptr, uint64_t val) { return ptr->start < val; };
        auto leftIt = std::lower_bound(data.begin(), data.end(), api.beginTime, cmpLower);
        auto cmpUpper = [](uint64_t val, const ObjectPtr<T> &ptr) { return val < ptr->start; };
        auto rightIt = std::upper_bound(data.begin(), data.end(), api.endTime, cmpUpper);

        auto realBeginTime = Common::ContextManager::GetInstance()->GetHostRealTime(api.beginTime);
        auto realEndTime = Common::ContextManager::GetInstance()->GetHostRealTime(api.endTime);

        for (auto it = leftIt; it != rightIt; ++it)
        {
            it->get()->start = realBeginTime;
            it->get()->end = realEndTime;
            if (UNLIKELY(Activity::ActivityManager::GetInstance()->Record(
                             Common::ReinterpretConvert<msptiActivity *>(it->get()), sizeof(T)) != MSPTI_SUCCESS))
            {
                MSPTI_LOGE("Record %s activity fail, please check buffer", logMsg);
                ret = MSPTI_ERROR_INNER;
            }
        }
        auto discardCount = std::distance(data.begin(), leftIt);
        if (discardCount > 0)
        {
            MSPTI_LOGW("Discard %d %s record for thread %u.", discardCount, logMsg, api.threadId);
        }
        data.erase(data.begin(), rightIt);
        return ret;
    }

    Common::SimpleObjectPool<msptiActivityMemory> memoryActivityPool_;
    Common::SimpleObjectPool<msptiActivityMemset> memsetActivityPool_;
    Common::SimpleObjectPool<msptiActivityMemcpy> memcpyActivityPool_;
    std::mutex addrMtx_;
    std::unordered_map<uint64_t, uint64_t> addrBytesMap_;
    std::mutex memoryMtx_;
    ObjectCache<msptiActivityMemory> memoryDataCache_;
    std::mutex memsetMtx_;
    ObjectCache<msptiActivityMemset> memsetDataCache_;
    std::mutex memcpyMtx_;
    ObjectCache<msptiActivityMemcpy> memcpyDataCache_;
};

msptiResult MemoryParser::MemoryParserImpl::ReportMemory(const MsprofCompactInfo &record)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMORY) ||
        !Common::IsRuntimeSupportMemoryReport())
    {
        return MSPTI_SUCCESS;
    }
    auto address = record.data.memMngInfo.address;
    auto size = record.data.memMngInfo.size;
    auto memoryOperationType =
        GetMsptiMemoryOperationType(static_cast<RuntimeMemMngType>(record.data.memMngInfo.memMngType));
    {
        std::lock_guard<std::mutex> lock(addrMtx_);
        auto iter = addrBytesMap_.find(address);
        if (iter == addrBytesMap_.end())
        {
            if (memoryOperationType == MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_RELEASE)
            {
                MSPTI_LOGW("Address %llu release, but not have allocation record.", address);
            }
            else
            {
                addrBytesMap_.insert({address, size});
            }
        }
        else
        {
            if (memoryOperationType == MSPTI_ACTIVITY_MEMORY_OPERATION_TYPE_RELEASE)
            {
                size = iter->second;
                addrBytesMap_.erase(iter);
            }
            else
            {
                MSPTI_LOGW("Address %llu more than one allocation record.", address);
                iter->second = size;
            }
        }
    }

    return ReportImpl<msptiActivityMemory>(
        memoryActivityPool_, memoryMtx_, memoryDataCache_, MSPTI_ACTIVITY_KIND_MEMORY, record,
        [&](auto &activity, const MsprofCompactInfo &record)
        {
            activity->memoryOperationType = memoryOperationType;
            activity->memoryKind = GetMsptiMemoryKind(static_cast<RtMemoryType>(record.data.memMngInfo.memoryType));
            activity->address = address;
            activity->bytes = size;
            activity->processId = Common::Utils::GetPid();
            activity->deviceId = record.data.memMngInfo.deviceId;
            activity->streamId = CheckStreamId(record.data.memMngInfo.streamId);
        });
}

msptiResult MemoryParser::MemoryParserImpl::ReportMemset(const MsprofCompactInfo &record)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMSET) ||
        !Common::IsRuntimeSupportMemoryReport())
    {
        return MSPTI_SUCCESS;
    }

    return ReportImpl<msptiActivityMemset>(
        memsetActivityPool_, memsetMtx_, memsetDataCache_, MSPTI_ACTIVITY_KIND_MEMSET, record,
        [&](auto &activity, const MsprofCompactInfo &record)
        {
            activity->value = static_cast<uint32_t>(record.data.memsetInfo.value);
            activity->bytes = record.data.memsetInfo.bytes;
            activity->deviceId = record.data.memsetInfo.deviceId;
            activity->streamId = CheckStreamId(record.data.memsetInfo.streamId);
            activity->isAsync = (activity->streamId != MSPTI_INVALID_STREAM_ID ? 1 : 0);
        });
}

msptiResult MemoryParser::MemoryParserImpl::ReportMemcpy(const MsprofCompactInfo &record)
{
    if (!Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMCPY) ||
        !Common::IsRuntimeSupportMemoryReport())
    {
        return MSPTI_SUCCESS;
    }

    return ReportImpl<msptiActivityMemcpy>(
        memcpyActivityPool_, memcpyMtx_, memcpyDataCache_, MSPTI_ACTIVITY_KIND_MEMCPY, record,
        [&](auto &activity, const MsprofCompactInfo &record)
        {
            activity->copyKind = GetMsptiMemcpyKind(static_cast<RtMemcpyKind>(record.data.memcpyInfo.copyKind));
            activity->bytes = record.data.memcpyInfo.bytes;
            activity->deviceId = record.data.memcpyInfo.deviceId;
            activity->streamId = CheckStreamId(record.data.memcpyInfo.streamId);
            activity->isAsync = (activity->streamId != MSPTI_INVALID_STREAM_ID ? 1 : 0);
        });
}

msptiResult MemoryParser::MemoryParserImpl::RecordApi(const MsprofApi &api)
{
    if (!IsAclRtsApi(api.type) || !Common::IsRuntimeSupportMemoryReport())
    {
        return MSPTI_SUCCESS;
    }

    const auto &apiName = CannHashCache::GetTypeHashInfo(api.level, api.type);

    if (Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMORY) &&
        IsInApiWhiteList(apiName, ACL_RT_MEMORY_API_WHITE_LIST))
    {
        return RecordApiImpl<msptiActivityMemory>(memoryMtx_, memoryDataCache_, api, "Memory");
    }
    else if (Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMSET) &&
             IsInApiWhiteList(apiName, ACL_RT_MEMSET_API_WHITE_LIST))
    {
        return RecordApiImpl<msptiActivityMemset>(memsetMtx_, memsetDataCache_, api, "Memset");
    }
    else if (Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MEMCPY) &&
             IsInApiWhiteList(apiName, ACL_RT_MEMCPY_API_WHITE_LIST))
    {
        return RecordApiImpl<msptiActivityMemcpy>(memcpyMtx_, memcpyDataCache_, api, "Memcpy");
    }
    return MSPTI_SUCCESS;
}

MemoryParser::MemoryParser() : pImpl(std::make_unique<MemoryParserImpl>()) {}

MemoryParser &MemoryParser::GetInstance()
{
    static MemoryParser instance;
    return instance;
}

msptiResult MemoryParser::ReportMemory(const MsprofCompactInfo &record) { return pImpl->ReportMemory(record); }

msptiResult MemoryParser::ReportMemset(const MsprofCompactInfo &record) { return pImpl->ReportMemset(record); }

msptiResult MemoryParser::ReportMemcpy(const MsprofCompactInfo &record) { return pImpl->ReportMemcpy(record); }

msptiResult MemoryParser::RecordApi(const MsprofApi &api) { return pImpl->RecordApi(api); }
}  // namespace Parser
}  // namespace Mspti
