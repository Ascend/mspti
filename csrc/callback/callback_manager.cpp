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
#include "csrc/callback/callback_manager.h"

#include <vector>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/mstx_inject.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace Callback
{
namespace
{
inline bool IsValidCBDomain(msptiCallbackDomain domain)
{
    return domain > MSPTI_CB_DOMAIN_INVALID && domain < MSPTI_CB_DOMAIN_SIZE;
}

inline bool IsValidCBId(msptiCallbackId cbid) { return cbid < sizeof(CallbackManager::BitMap) * 8; }

inline bool HasLdPreload()
{
    static auto hasLdPreload = []() -> bool
    {
        const std::string ld = Common::Utils::GetEnv("LD_PRELOAD");
        return ld.find("libmspti.so") != std::string::npos;
    }();
    return hasLdPreload;
}

constexpr std::array<const char*, MSPTI_CBID_RUNTIME_SIZE> RUNTIME_DOMAIN_CALLBACKS = {
    nullptr,                              // MSPTI_CBID_RUNTIME_INVALID
    "aclrtSetDevice",                     // MSPTI_CBID_RUNTIME_DEVICE_SET
    "aclrtResetDevice",                   // MSPTI_CBID_RUNTIME_DEVICE_RESET
    "aclrtSetDeviceEx",                   // MSPTI_CBID_RUNTIME_DEVICE_SET_EX
    "aclrtCreateContextEx",               // MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX
    "aclrtCreateContext",                 // MSPTI_CBID_RUNTIME_CONTEXT_CREATED
    "aclrtDestroyContext",                // MSPTI_CBID_RUNTIME_CONTEXT_DESTROY
    "aclrtCreateStream",                  // MSPTI_CBID_RUNTIME_STREAM_CREATED
    "aclrtDestroyStream",                 // MSPTI_CBID_RUNTIME_STREAM_DESTROY
    "aclrtSynchronizeStream",             // MSPTI_CBID_RUNTIME_STREAM_SYNCHRONIZED
    "aclrtLaunchKernel",                  // MSPTI_CBID_RUNTIME_LAUNCH
    "aclrtLaunchKernel",                  // MSPTI_CBID_RUNTIME_CPU_LAUNCH
    "aclrtLaunchKernel",                  // MSPTI_CBID_RUNTIME_AICPU_LAUNCH
    "aclrtLaunchKernel",                  // MSPTI_CBID_RUNTIME_AIV_LAUNCH
    "aclrtLaunchKernel",                  // MSPTI_CBID_RUNTIME_FFTS_LAUNCH
    "aclrtMalloc",                        // MSPTI_CBID_RUNTIME_MALLOC
    "aclrtFree",                          // MSPTI_CBID_RUNTIME_FREE
    "aclrtMallocHost",                    // MSPTI_CBID_RUNTIME_MALLOC_HOST
    "aclrtFreeHost",                      // MSPTI_CBID_RUNTIME_FREE_HOST
    "aclrtMallocCached",                  // MSPTI_CBID_RUNTIME_MALLOC_CACHED
    "aclrtMemFlush",                      // MSPTI_CBID_RUNTIME_FLUSH_CACHE
    "aclrtMemInvalidate",                 // MSPTI_CBID_RUNTIME_INVALID_CACHE
    "aclrtMemcpy",                        // MSPTI_CBID_RUNTIME_MEMCPY
    "aclrtMemcpy",                        // MSPTI_CBID_RUNTIME_MEMCPY_HOST
    "aclrtMemcpyAsync",                   // MSPTI_CBID_RUNTIME_MEMCPY_ASYNC
    "aclrtMemcpy2D",                      // MSPTI_CBID_RUNTIME_MEM_CPY2D
    "aclrtMemcpy2DAsync",                 // MSPTI_CBID_RUNTIME_MEM_CPY2D_ASYNC
    "aclrtMemSet",                        // MSPTI_CBID_RUNTIME_MEM_SET
    "aclrtMemSetAsync",                   // MSPTI_CBID_RUNTIME_MEM_SET_ASYNC
    "aclrtGetMemInfo",                    // MSPTI_CBID_RUNTIME_MEM_GET_INFO
    "aclrtReserveMemAddress",             // MSPTI_CBID_RUNTIME_RESERVE_MEM_ADDRESS
    "aclrtReleaseMemAddress",             // MSPTI_CBID_RUNTIME_RELEASE_MEM_ADDRESS
    "aclrtMallocPhysical",                // MSPTI_CBID_RUNTIME_MALLOC_PHYSICAL
    "aclrtFreePhysical",                  // MSPTI_CBID_RUNTIME_FREE_PHYSICAL
    "aclrtMemExportToShareableHandle",    // MSPTI_CBID_RUNTIME_MEM_EXPORT_TO_SHAREABLE_HANDLE
    "aclrtMemImportFromShareableHandle",  // MSPTI_CBID_RUNTIME_MEM_IMPORT_FROM_SHAREABLE_HANDLE
    "aclrtMemSetPidToShareableHandle",    // MSPTI_CBID_RUNTIME_MEM_SET_PID_TO_SHAREABLE_HANDLE
};

constexpr std::array<const char*, MSPTI_CBID_HCCL_SIZE> HCCL_DOMAIN_CALLBACKS = {
    nullptr,              // MSPTI_CBID_HCCL_INVALID
    "HcclAllReduce",      // MSPTI_CBID_HCCL_ALLREDUCE
    "HcclBroadcast",      // MSPTI_CBID_HCCL_BROADCAST
    "HcclAllGather",      // MSPTI_CBID_HCCL_ALLGATHER
    "HcclReduceScatter",  // MSPTI_CBID_HCCL_REDUCE_SCATTER
    "HcclReduce",         // MSPTI_CBID_HCCL_REDUCE
    "HcclAllToAll",       // MSPTI_CBID_HCCL_ALL_TO_ALL
    "HcclAllToAllV",      // MSPTI_CBID_HCCL_ALL_TO_ALLV
    "HcclBarrier",        // MSPTI_CBID_HCCL_BARRIER
    "HcclScatter",        // MSPTI_CBID_HCCL_SCATTER
    "HcclSend",           // MSPTI_CBID_HCCL_SEND
    "HcclRecv",           // MSPTI_CBID_HCCL_RECV
    "HcclBatchSendRecv",  // MSPTI_CBID_HCCL_SENDRECV
};

inline uint32_t GetDomainCallbackCount(msptiCallbackDomain domain)
{
    switch (domain)
    {
        case MSPTI_CB_DOMAIN_RUNTIME:
            return RUNTIME_DOMAIN_CALLBACKS.size();
        case MSPTI_CB_DOMAIN_HCCL:
            return HCCL_DOMAIN_CALLBACKS.size();
        default:
            return 0;
    }
}

inline bool GetCallbackNameByDomain(msptiCallbackDomain domain, msptiCallbackId cbid, const char** name)
{
    if (name == nullptr)
    {
        return false;
    }
    switch (domain)
    {
        case MSPTI_CB_DOMAIN_RUNTIME:
            if (cbid <= MSPTI_CBID_RUNTIME_INVALID || cbid >= RUNTIME_DOMAIN_CALLBACKS.size())
            {
                return false;
            }
            *name = RUNTIME_DOMAIN_CALLBACKS[cbid];
            break;
        case MSPTI_CB_DOMAIN_HCCL:
            if (cbid <= MSPTI_CBID_HCCL_INVALID || cbid >= HCCL_DOMAIN_CALLBACKS.size())
            {
                return false;
            }
            *name = HCCL_DOMAIN_CALLBACKS[cbid];
            break;
        default:
            return false;
    }
    return *name != nullptr;
}
}  // namespace

CallbackManager* CallbackManager::GetInstance()
{
    static CallbackManager instance;
    return &instance;
}

msptiResult CallbackManager::Init(msptiSubscriberHandle* subscriber, msptiCallbackFunc callback, void* userdata)
{
    if (subscriber == nullptr)
    {
        MSPTI_LOGE("subscriber cannot be nullptr.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    std::lock_guard<std::mutex> lock(subscriber_mutex_);
    if (init_.load())
    {
        MSPTI_LOGE("subscriber cannot be register repeat.");
        return MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED;
    }
    std::shared_ptr<msptiSubscriber_st> new_ptr;
    Mspti::Common::MsptiMakeSharedPtr(new_ptr);
    if (UNLIKELY(!new_ptr))
    {
        MSPTI_LOGE("Failed to init subscriber.");
        return MSPTI_ERROR_INNER;
    }
    for (auto& bitmap : cbid_map_)
    {
        bitmap.store(0, std::memory_order_relaxed);
    }
    new_ptr->handle = callback;
    new_ptr->userdata = userdata;
    *subscriber = new_ptr.get();
    std::atomic_store(&subscriber_ptr_, new_ptr);
    init_.store(true);
    Mspti::Ascend::DevTaskManager::GetInstance()->RegisterReportCallback();
    Mspti::Common::ContextManager::GetInstance()->StartSyncTime();
    MsptiMstxApi::MsptiEnableMstxFunc();
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    Mspti::Activity::ActivityManager::GetInstance()->StartActivityMgrThread();
    MSPTI_LOGI("CallbackManager Init success.");
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::UnInit(msptiSubscriberHandle subscriber)
{
    std::lock_guard<std::mutex> lock(subscriber_mutex_);
    if (!init_.load())
    {
        MSPTI_LOGW("CallbackManager is not init, no need to UnInit.");
        return MSPTI_SUCCESS;
    }
    if (std::atomic_load(&subscriber_ptr_).get() != subscriber)
    {
        MSPTI_LOGE("CallbackManager UnInit subscriber was not subscribe.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    std::atomic_exchange(&subscriber_ptr_, std::shared_ptr<msptiSubscriber_st>(nullptr));
    for (auto& bitmap : cbid_map_)
    {
        bitmap.store(0, std::memory_order_relaxed);
    }
    MsptiMstxApi::MsptiDisableMstxFunc();
    Mspti::Common::ContextManager::GetInstance()->StopSyncTime();
    Mspti::Ascend::DevTaskManager::GetInstance()->UnRegisterReportCallback();
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
    Mspti::Activity::ActivityManager::GetInstance()->StopActivityMgrThread();
    init_.store(false);
    MSPTI_LOGI("CallbackManager UnInit success.");
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::Register(msptiCallbackDomain domain, msptiCallbackId cbid)
{
    if (!IsValidCBDomain(domain) || !IsValidCBId(cbid))
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    auto idx = static_cast<size_t>(domain);
    uint64_t mask = 1ULL << static_cast<int>(cbid);
    cbid_map_[idx].fetch_or(mask, std::memory_order_relaxed);
    MSPTI_LOGI("CallbackManager Register domain: %d, cbid: %d.", domain, cbid);
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::UnRegister(msptiCallbackDomain domain, msptiCallbackId cbid)
{
    if (!IsValidCBDomain(domain) || !IsValidCBId(cbid))
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    auto idx = static_cast<size_t>(domain);
    uint64_t mask = ~(1ULL << static_cast<int>(cbid));
    cbid_map_[idx].fetch_and(mask, std::memory_order_relaxed);
    MSPTI_LOGI("CallbackManager UnRegister domain: %d, cbid: %d.", domain, cbid);
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::PreCheck(msptiSubscriberHandle subscriber)
{
    if (!HasLdPreload())
    {
        MSPTI_LOGE("Enable callbackDomain requires libmspti.so in LD_PRELOAD.");
        return MSPTI_ERROR_WITHOUT_LD_PRELOAD;
    }
    if (!init_.load())
    {
        MSPTI_LOGW("CallbackManager is not initialized.");
        return MSPTI_ERROR_NOT_INITIALIZED;
    }
    if (std::atomic_load(&subscriber_ptr_).get() != subscriber)
    {
        MSPTI_LOGE("subscriber is not subscribe.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::EnableCallback(uint32_t enable, msptiSubscriberHandle subscriber,
                                            msptiCallbackDomain domain, msptiCallbackId cbid)
{
    auto preCheckRet = PreCheck(subscriber);
    if (preCheckRet != MSPTI_SUCCESS)
    {
        return preCheckRet;
    }
    if (!IsValidCBDomain(domain))
    {
        MSPTI_LOGE("domain: %d is invalid.", domain);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    return (enable != 0) ? Register(domain, cbid) : UnRegister(domain, cbid);
}

msptiResult CallbackManager::EnableDomain(uint32_t enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain)
{
    auto preCheckRet = PreCheck(subscriber);
    if (preCheckRet != MSPTI_SUCCESS)
    {
        return preCheckRet;
    }
    if (!IsValidCBDomain(domain))
    {
        MSPTI_LOGE("domain: %d was invalid.", domain);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    auto count = GetDomainCallbackCount(domain);
    if (count == 0)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    msptiResult ret = MSPTI_SUCCESS;
    for (uint32_t idx = 1; idx < count; ++idx)
    {
        auto cbid = static_cast<msptiCallbackId>(idx);
        auto regRet = (enable != 0) ? Register(domain, cbid) : UnRegister(domain, cbid);
        ret = (regRet != MSPTI_SUCCESS) ? regRet : ret;
    }
    return ret;
}

msptiResult CallbackManager::EnableAllDomains(uint32_t enable, msptiSubscriberHandle subscriber)
{
    auto preCheckRet = PreCheck(subscriber);
    if (preCheckRet != MSPTI_SUCCESS)
    {
        return preCheckRet;
    }
    msptiResult ret = MSPTI_SUCCESS;
    for (uint32_t domain = 1; domain < MSPTI_CB_DOMAIN_SIZE; ++domain)
    {
        auto count = GetDomainCallbackCount(static_cast<msptiCallbackDomain>(domain));
        if (count == 0)
        {
            continue;
        }
        auto cbDomain = static_cast<msptiCallbackDomain>(domain);
        for (uint32_t idx = 1; idx < count; ++idx)
        {
            auto cbid = static_cast<msptiCallbackId>(idx);
            auto regRet = (enable != 0) ? Register(cbDomain, cbid) : UnRegister(cbDomain, cbid);
            ret = (regRet != MSPTI_SUCCESS) ? regRet : ret;
        }
    }
    return ret;
}

void CallbackManager::ExecuteCallback(msptiCallbackDomain domain, msptiCallbackId cbid, msptiApiCallbackSite site,
                                      const char* funcName)
{
    if (!init_.load())
    {
        return;
    }
    if (!IsCallbackIdEnable(domain, cbid))
    {
        return;
    }

    auto local_ptr = std::atomic_load(&subscriber_ptr_);
    if (local_ptr && local_ptr->handle)
    {
        MSPTI_LOGD("CallbackManager execute Callbackfunc, funcName is %s", funcName);
        msptiCallbackData callbackData;
        callbackData.callbackSite = site;
        callbackData.functionName = funcName;
        local_ptr->handle(local_ptr->userdata, domain, cbid, &callbackData);
    }
}

bool CallbackManager::IsCallbackIdEnable(msptiCallbackDomain domain, msptiCallbackId cbid)
{
    if (!IsValidCBDomain(domain) || !IsValidCBId(cbid))
    {
        return false;
    }
    auto idx = static_cast<size_t>(domain);
    uint64_t bits = cbid_map_[idx].load(std::memory_order_relaxed);
    return (bits >> static_cast<int>(cbid)) & 1;
}

msptiResult CallbackManager::GetCallbackName(msptiCallbackDomain domain, uint32_t cbid, const char** name)
{
    if (name == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!IsValidCBDomain(domain) || !IsValidCBId(cbid))
    {
        MSPTI_LOGE("domain: %d, cbid: %d is invalid.", domain, cbid);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!GetCallbackNameByDomain(domain, static_cast<msptiCallbackId>(cbid), name))
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::GetCallbackState(uint32_t* enable, msptiSubscriberHandle subscriber,
                                              msptiCallbackDomain domain, msptiCallbackId cbid)
{
    if (enable == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!IsValidCBDomain(domain) || !IsValidCBId(cbid))
    {
        MSPTI_LOGE("domain: %d, cbid: %d is invalid.", domain, cbid);
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!init_.load())
    {
        MSPTI_LOGW("CallbackManager is not initialized.");
        return MSPTI_ERROR_NOT_INITIALIZED;
    }
    if (std::atomic_load(&subscriber_ptr_).get() != subscriber)
    {
        MSPTI_LOGE("subscriber is not subscribe.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *enable = IsCallbackIdEnable(domain, cbid) ? 1 : 0;
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::SupportedDomains(size_t* domainCount, msptiDomainTable* domainTable)
{
    if (domainCount == nullptr || domainTable == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    static auto supportedDomains = []() -> std::vector<msptiCallbackDomain>
    {
        std::vector<msptiCallbackDomain> domains;
        for (size_t domain = 1; domain < MSPTI_CB_DOMAIN_SIZE; ++domain)
        {
            if (GetDomainCallbackCount(static_cast<msptiCallbackDomain>(domain)) != 0)
            {
                domains.push_back(static_cast<msptiCallbackDomain>(domain));
            }
        }
        return domains;
    }();
    *domainCount = supportedDomains.size();
    *domainTable = supportedDomains.data();
    return MSPTI_SUCCESS;
}

msptiResult CallbackManager::GetEnabledCallbacks(msptiSubscriberHandle subscriber, msptiCallbackDomain domain,
                                                 msptiCallbackId* buffer, uint32_t* bufferSize,
                                                 uint32_t* enabledCallbacksCount)
{
    if (enabledCallbacksCount == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!IsValidCBDomain(domain))
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (!init_.load())
    {
        MSPTI_LOGW("CallbackManager is not initialized.");
        return MSPTI_ERROR_NOT_INITIALIZED;
    }
    if (std::atomic_load(&subscriber_ptr_).get() != subscriber)
    {
        MSPTI_LOGE("subscriber is not subscribe.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *enabledCallbacksCount = 0;
    auto count = GetDomainCallbackCount(domain);
    if (count == 0)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    for (uint32_t idx = 1; idx < count; ++idx)
    {
        if (IsCallbackIdEnable(domain, static_cast<msptiCallbackId>(idx)))
        {
            (*enabledCallbacksCount)++;
        }
    }
    if (buffer == nullptr)
    {
        return MSPTI_SUCCESS;
    }
    if (bufferSize == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    auto writeCount = std::min(*bufferSize, *enabledCallbacksCount);
    uint32_t written = 0;
    for (uint32_t idx = 1; idx < count && written < writeCount; ++idx)
    {
        auto cbid = static_cast<msptiCallbackId>(idx);
        if (IsCallbackIdEnable(domain, cbid))
        {
            buffer[written++] = cbid;
        }
    }
    return MSPTI_SUCCESS;
}
}  // namespace Callback
}  // namespace Mspti

msptiResult msptiSubscribe(msptiSubscriberHandle* subscriber, msptiCallbackFunc callback, void* userdata)
{
    msptiResult ret = Mspti::Callback::CallbackManager::GetInstance()->Init(subscriber, callback, userdata);
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("msptiSubscribe failed, ret: %d.", ret);
        return ret;
    }
    MSPTI_EVENT("msptiSubscribe success.");
    return ret;
}

msptiResult msptiUnsubscribe(msptiSubscriberHandle subscriber)
{
    if (Mspti::Activity::ActivityManager::GetInstance()->ResetAllDevice() != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Reset all device failed.");
    }
    msptiResult ret = Mspti::Callback::CallbackManager::GetInstance()->UnInit(subscriber);
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("msptiUnsubscribe failed, ret: %d.", ret);
        return ret;
    }
    MSPTI_EVENT("msptiUnsubscribe success.");
    return ret;
}

msptiResult msptiEnableCallback(uint32_t enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain,
                                msptiCallbackId cbid)
{
    return Mspti::Callback::CallbackManager::GetInstance()->EnableCallback(enable, subscriber, domain, cbid);
}

msptiResult msptiEnableDomain(uint32_t enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain)
{
    return Mspti::Callback::CallbackManager::GetInstance()->EnableDomain(enable, subscriber, domain);
}

msptiResult msptiEnableAllDomains(uint32_t enable, msptiSubscriberHandle subscriber)
{
    return Mspti::Callback::CallbackManager::GetInstance()->EnableAllDomains(enable, subscriber);
}

msptiResult msptiGetCallbackName(msptiCallbackDomain domain, uint32_t cbid, const char** name)
{
    return Mspti::Callback::CallbackManager::GetInstance()->GetCallbackName(domain, cbid, name);
}

msptiResult msptiGetCallbackState(uint32_t* enable, msptiSubscriberHandle subscriber, msptiCallbackDomain domain,
                                  msptiCallbackId cbid)
{
    return Mspti::Callback::CallbackManager::GetInstance()->GetCallbackState(enable, subscriber, domain, cbid);
}

msptiResult msptiSupportedDomains(size_t* domainCount, msptiDomainTable* domainTable)
{
    return Mspti::Callback::CallbackManager::GetInstance()->SupportedDomains(domainCount, domainTable);
}

msptiResult msptiGetEnabledCallbacks(msptiSubscriberHandle subscriber, msptiCallbackDomain domain,
                                     msptiCallbackId* buffer, uint32_t* bufferSize, uint32_t* enabledCallbacksCount)
{
    return Mspti::Callback::CallbackManager::GetInstance()->GetEnabledCallbacks(subscriber, domain, buffer, bufferSize,
                                                                                enabledCallbacksCount);
}

msptiResult msptiIsTracingSessionRunning(uint8_t* isRunning)
{
    if (isRunning == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *isRunning = Mspti::Callback::CallbackManager::GetInstance()->IsTracingSessionRunning() ? 1 : 0;
    return MSPTI_SUCCESS;
}
