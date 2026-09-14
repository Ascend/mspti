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

#include "csrc/activity/activity_manager.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <functional>
#include <thread>

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/activity/ascend/parser/communication_calculator.h"
#include "csrc/activity/ascend/parser/kernel_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/activity/ascend/reporter/external_correlation_reporter.h"
#include "csrc/activity/ascend/reporter/overhead_reporter.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "securec.h"

namespace Mspti
{
namespace Activity
{
constexpr uint32_t ActivityManager::FLUSH_RETRY_MAX_COUNT;
constexpr uint32_t ActivityManager::FLUSH_RETRY_WAIT_MS;
namespace
{
msptiResult IsNeedLdPreload(msptiActivityKind kind)
{
    // Some activity kinds depend on LD_PRELOAD hooking (libmspti.so). If those
    // kinds are enabled but LD_PRELOAD does not contain libmspti.so, return
    // MSPTI_ERROR_WITHOUT_LD_PRELOAD to notify the caller.
    static const std::unordered_set<msptiActivityKind> needLdPreloadKinds = []()
    {
        std::unordered_set<msptiActivityKind> kinds = {MSPTI_ACTIVITY_KIND_HCCL};
        if (!Common::IsRuntimeSupportMemoryReport())
        {
            kinds.insert(MSPTI_ACTIVITY_KIND_MEMORY);
            kinds.insert(MSPTI_ACTIVITY_KIND_MEMSET);
            kinds.insert(MSPTI_ACTIVITY_KIND_MEMCPY);
        }
        return kinds;
    }();

    if (needLdPreloadKinds.find(kind) != needLdPreloadKinds.end())
    {
        static const std::string ld = Mspti::Common::Utils::GetEnv("LD_PRELOAD");
        if (ld.find("libmspti.so") == std::string::npos)
        {
            MSPTI_LOGE("Enable activity kind %d requires libmspti.so in LD_PRELOAD.", static_cast<int>(kind));
            return MSPTI_ERROR_WITHOUT_LD_PRELOAD;
        }
    }
    return MSPTI_SUCCESS;
}

inline bool IsNeededDevTask(msptiActivityKind kind)
{
    static const std::unordered_set<msptiActivityKind> needDevTaskKinds = {
        MSPTI_ACTIVITY_KIND_MARKER, MSPTI_ACTIVITY_KIND_KERNEL, MSPTI_ACTIVITY_KIND_HCCL,
        MSPTI_ACTIVITY_KIND_COMMUNICATION};
    return needDevTaskKinds.find(kind) != needDevTaskKinds.end();
}

// For KERNEL / COMMUNICATION: poll parser pending after the first flush and retry until drained,
// at most FLUSH_RETRY_MAX_COUNT times. Requires the activity switch to stay on for device drain.
void RetryFlushUntilDrained(msptiActivityKind kind, const std::unordered_set<uint32_t> &localDevices)
{
    if (kind != MSPTI_ACTIVITY_KIND_KERNEL && kind != MSPTI_ACTIVITY_KIND_COMMUNICATION)
    {
        return;
    }
    if (localDevices.empty())
    {
        return;
    }
    auto getPendingCount = [kind]() -> int64_t
    {
        return kind == MSPTI_ACTIVITY_KIND_KERNEL
                   ? Parser::KernelParser::GetInstance().GetPendingKernelCount()
                   : Parser::CommunicationCalculator::GetInstance().GetPendingCommunicationCount();
    };
    const char *parserName = (kind == MSPTI_ACTIVITY_KIND_KERNEL) ? "KernelParser" : "CommunicationCalculator";
    const char *pendingDesc = (kind == MSPTI_ACTIVITY_KIND_KERNEL) ? "pending kernels" : "pending communications";
    MSPTI_LOGI("RetryFlushUntilDrained, kind %d, count %" PRId64, kind, getPendingCount());
    for (uint32_t retry = 0; retry < ActivityManager::FLUSH_RETRY_MAX_COUNT; retry++)
    {
        auto pendingCount = getPendingCount();
        if (pendingCount == 0)
        {
            break;
        }
        MSPTI_LOGW("%s has %" PRId64 " %s before unregister, retry flush %u/%u", parserName, pendingCount, pendingDesc,
                   retry + 1, ActivityManager::FLUSH_RETRY_MAX_COUNT);
        for (auto device : localDevices)
        {
            Ascend::DevTaskManager::GetInstance()->FlushDevProfData(device, kind);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(ActivityManager::FLUSH_RETRY_WAIT_MS));
    }
    auto remainCount = getPendingCount();
    if (remainCount > 0)
    {
        MSPTI_LOGW("%s still has %" PRId64 " %s after %u retries, give up.", parserName, remainCount, pendingDesc,
                   ActivityManager::FLUSH_RETRY_MAX_COUNT);
    }
}
}  // namespace

inline bool GetActivityStructSize(msptiActivityKind kind, size_t *size)
{
    if (size == nullptr)
    {
        return false;
    }
    if (kind <= MSPTI_ACTIVITY_KIND_INVALID || MSPTI_ACTIVITY_KIND_COUNT <= kind)
    {
        return false;
    }
    static constexpr std::array<size_t, MSPTI_ACTIVITY_KIND_COUNT> activityKindDataSize = {
        0,                                         // MSPTI_ACTIVITY_KIND_INVALID
        sizeof(msptiActivityMarker),               // MSPTI_ACTIVITY_KIND_MARKER
        sizeof(msptiActivityKernel),               // MSPTI_ACTIVITY_KIND_KERNEL
        sizeof(msptiActivityApi),                  // MSPTI_ACTIVITY_KIND_API
        sizeof(msptiActivityHccl),                 // MSPTI_ACTIVITY_KIND_HCCL
        sizeof(msptiActivityMemory),               // MSPTI_ACTIVITY_KIND_MEMORY
        sizeof(msptiActivityMemset),               // MSPTI_ACTIVITY_KIND_MEMSET
        sizeof(msptiActivityMemcpy),               // MSPTI_ACTIVITY_KIND_MEMCPY
        sizeof(msptiActivityExternalCorrelation),  // MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION
        sizeof(msptiActivityCommunication),        // MSPTI_ACTIVITY_KIND_COMMUNICATION
        sizeof(msptiActivityApi),                  // MSPTI_ACTIVITY_KIND_ACL_API
        sizeof(msptiActivityApi),                  // MSPTI_ACTIVITY_KIND_NODE_API
        sizeof(msptiActivityApi),                  // MSPTI_ACTIVITY_KIND_RUNTIME_API
        sizeof(msptiActivityOverhead),             // MSPTI_ACTIVITY_KIND_OVERHEAD
    };
    *size = activityKindDataSize[kind];
    return true;
}

inline size_t GetActivityAttributeSize(msptiActivityAttribute attr)
{
    switch (attr)
    {
        case MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE:
            return sizeof(uint32_t);
        case MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK:
            return sizeof(msptiTimestampCallbackFunc);
        default:
            break;
    }
    return 0;
}

void ActivityBuffer::Init(msptiBuffersCallbackRequestFunc func)
{
    if (func == nullptr)
    {
        MSPTI_LOGE("The request callback is nullptr.");
        return;
    }
    Reporter::OverheadRecord overheadRecord(MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST,
                                            MSPTI_ACTIVITY_OBJECT_THREAD);
    func(&buf_, &buf_size_, &records_num_);
    constexpr uint64_t MIN_ACTIVITY_BUFFER_SIZE = 2 * 1024 * 1024;
    if (buf_size_ < MIN_ACTIVITY_BUFFER_SIZE)
    {
        MSPTI_LOGW("Please malloc the Activity Buffer more than 2MB. Current is %lu Bytes.", buf_size_);
    }
    MSPTI_LOGI("ActivityBuffer init, bufSize: %zu, recordsNum: %zu", buf_size_, records_num_);
}

void ActivityBuffer::UnInit(msptiBuffersCallbackCompleteFunc func)
{
    if (func == nullptr)
    {
        MSPTI_LOGE("The complete callback is nullptr.");
        return;
    }
    Reporter::OverheadRecord overheadRecord(MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH,
                                            MSPTI_ACTIVITY_OBJECT_THREAD);
    MSPTI_LOGI("CallbackCompleteFunc start, validSize: %zu, bufSize: %zu, recordsNum: %zu", valid_size_, buf_size_,
               records_num_);
    func(buf_, buf_size_, valid_size_);
}

msptiResult ActivityBuffer::Record(msptiActivity *activity, size_t size)
{
    if (activity == nullptr)
    {
        MSPTI_LOGE("The activity is nullptr, failed to record.");
        return MSPTI_ERROR_INNER;
    }
    if (buf_ == nullptr)
    {
        MSPTI_LOGE("The ActivityBuffer is nullptr, failed to record activity.");
        return MSPTI_ERROR_INNER;
    }
    if (size > buf_size_ - valid_size_)
    {
        MSPTI_LOGW("Record is dropped due to insufficient space of Activity Buffer.");
        return MSPTI_ERROR_INNER;
    }
    if (memcpy_s(buf_ + valid_size_, buf_size_ - valid_size_, activity, size) != EOK)
    {
        return MSPTI_ERROR_INNER;
    }
    valid_size_ += size;
    records_num_++;
    return MSPTI_SUCCESS;
}

bool ActivityBuffer::BufValid() { return buf_ != nullptr; }

size_t ActivityBuffer::BufSize() { return buf_size_; }

size_t ActivityBuffer::ValidSize() { return valid_size_; }

const std::set<msptiActivityKind> ActivityManager::supportActivityKinds_ = {
    MSPTI_ACTIVITY_KIND_MARKER,        MSPTI_ACTIVITY_KIND_KERNEL,
    MSPTI_ACTIVITY_KIND_API,           MSPTI_ACTIVITY_KIND_HCCL,
    MSPTI_ACTIVITY_KIND_MEMORY,        MSPTI_ACTIVITY_KIND_MEMSET,
    MSPTI_ACTIVITY_KIND_MEMCPY,        MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION,
    MSPTI_ACTIVITY_KIND_COMMUNICATION, MSPTI_ACTIVITY_KIND_ACL_API,
    MSPTI_ACTIVITY_KIND_NODE_API,      MSPTI_ACTIVITY_KIND_RUNTIME_API,
    MSPTI_ACTIVITY_KIND_OVERHEAD,
};

ActivityManager *ActivityManager::GetInstance()
{
    static ActivityManager instance;
    return &instance;
}

ActivityManager::~ActivityManager()
{
    StopActivityMgrThread();
    devices_.clear();
    MSPTI_EVENT("Total activity record: %lu. Total activity drop: %lu", total_record_num_.load(),
                total_drop_num_.load());
}

void ActivityManager::ResetActivitySwitch()
{
    for (auto &kindSwitch : activity_switch_)
    {
        kindSwitch.store(false);
    }
    for (auto &kindSwitch : append_only_activity_switch_)
    {
        kindSwitch.store(false);
    }
    for (auto &kindSwitch : unregistering_)
    {
        kindSwitch.store(false);
    }
}

void ActivityManager::StartActivityMgrThread()
{
    std::lock_guard<std::mutex> lk(thread_mtx_);
    if (thread_run_.load())
    {
        return;
    }
    if (activity_mgr_thread_ && activity_mgr_thread_->joinable())
    {
        MSPTI_LOGW("ActivityManager thread is already running.");
        thread_run_.store(true);
        return;
    }
    thread_run_.store(true);
    try
    {
        activity_mgr_thread_ = std::make_unique<std::thread>(&ActivityManager::Run, this);
        MSPTI_LOGI("ActivityManager thread started.");
    }
    catch (const std::exception &e)
    {
        MSPTI_LOGE("Failed to start ActivityManager thread: %s", e.what());
        thread_run_.store(false);
        activity_mgr_thread_.reset();
    }
}

void ActivityManager::StopActivityMgrThread()
{
    std::unique_ptr<std::thread> threadToJoin;
    {
        std::lock_guard<std::mutex> lk(thread_mtx_);
        if (!thread_run_.load() && !(activity_mgr_thread_ && activity_mgr_thread_->joinable()))
        {
            MSPTI_LOGW("ActivityManager thread is not running.");
            return;
        }
        thread_run_.store(false);
        {
            std::unique_lock<std::mutex> lck(cv_mtx_);
            try
            {
                cv_.notify_one();
            }
            catch (...)
            {
                // Exception occurred during destruction of ActivityManager
            }
        }
        if (activity_mgr_thread_ && activity_mgr_thread_->joinable())
        {
            threadToJoin = std::move(activity_mgr_thread_);
        }
        else
        {
            activity_mgr_thread_.reset();
        }
    }
    if (threadToJoin && threadToJoin->joinable())
    {
        try
        {
            threadToJoin->join();
        }
        catch (...)
        {
            // Exception occurred during destruction of ActivityManager
        }
    }
    JoinWorkThreads();
    ResetActivitySwitch();
    MSPTI_LOGI("ActivityManager thread stopped.");
}

msptiResult ActivityManager::TryInitActivityBuffer()
{
    Mspti::Common::MsptiMakeUniquePtr(cur_buf_);
    if (!cur_buf_)
    {
        MSPTI_LOGE("Failed to create cur buf object.");
        return MSPTI_ERROR_INNER;
    }
    cur_buf_->Init(bufferRequested_handle_);
    if (!cur_buf_->BufValid())
    {
        MSPTI_LOGE("Failed to init activity buffer.");
        cur_buf_.reset();
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

void ActivityManager::JoinWorkThreads()
{
    for (auto &thread : work_thread_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    work_thread_.clear();
}

msptiResult ActivityManager::RegisterCallbacks(msptiBuffersCallbackRequestFunc funcBufferRequested,
                                               msptiBuffersCallbackCompleteFunc funcBufferCompleted)
{
    if (funcBufferRequested == nullptr || funcBufferCompleted == nullptr)
    {
        MSPTI_LOGE("Call msptiActivityRegisterCallbacks failed while request or complete callback is nullptr.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    bufferRequested_handle_ = funcBufferRequested;
    bufferCompleted_handle_ = funcBufferCompleted;
    StartActivityMgrThread();
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::Register(msptiActivityKind kind)
{
    if (supportActivityKinds_.find(kind) == supportActivityKinds_.end())
    {
        MSPTI_LOGE("The ActivityKind: %d was not support.", static_cast<int>(kind));
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (IsNeedLdPreload(kind) != MSPTI_SUCCESS)
    {
        return MSPTI_ERROR_WITHOUT_LD_PRELOAD;
    }
    activity_switch_[kind] = true;
    append_only_activity_switch_[kind] = true;
    // 重新打开 host 数据上报开关（防御：上次 UnRegister 若未正常复位也在这里恢复）
    unregistering_[kind] = false;

    auto localDevices = GetAllValidDevice();
    ActivitySwitchType curOpenSwitch{};
    curOpenSwitch[kind] = true;
    for (auto device : localDevices)
    {
        Ascend::DevTaskManager::GetInstance()->StartDevProfTask(device, curOpenSwitch);
    }
    Parser::ParserManager::GetInstance()->StartAnalysisTask(kind);
    MSPTI_LOGI("Register Activity kind: %d", static_cast<int>(kind));
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::UnRegister(msptiActivityKind kind)
{
    if (supportActivityKinds_.find(kind) == supportActivityKinds_.end())
    {
        MSPTI_LOGE("The ActivityKind: %d was not support.", static_cast<int>(kind));
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    // 先关闭 host 数据上报开关：此后 parser 只消费存量排空 pending，不再接受新 host 数据
    unregistering_[kind] = true;
    if (IsNeededDevTask(kind))
    {
        auto localDevices = GetAllValidDevice();
        for (auto device : localDevices)
        {
            Ascend::DevTaskManager::GetInstance()->FlushDevProfData(device, kind);
        }
        if (!localDevices.empty() &&
            Common::ContextManager::GetInstance()->GetChipType(*localDevices.begin()) == Common::PlatformType::CHIP_V6)
        {
            constexpr uint32_t sleep_ms = 20;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
        }
        RetryFlushUntilDrained(kind, localDevices);
    }
    Parser::ParserManager::GetInstance()->StopAnalysisTask(kind);
    activity_switch_[kind] = false;
    if (kind == MSPTI_ACTIVITY_KIND_KERNEL)
    {
        Parser::KernelParser::GetInstance().Clear();
    }
    else if (kind == MSPTI_ACTIVITY_KIND_COMMUNICATION)
    {
        Parser::CommunicationCalculator::GetInstance().Clear();
    }
    unregistering_[kind] = false;
    MSPTI_LOGI("UnRegister Activity kind: %d", static_cast<int>(kind));
    return MSPTI_SUCCESS;
}

bool ActivityManager::IsActivityKindEnable(msptiActivityKind kind) { return activity_switch_[kind]; }

bool ActivityManager::IsHostReportAllowed(msptiActivityKind kind)
{
    return activity_switch_[kind] && !unregistering_[kind];
}

msptiResult ActivityManager::GetEnabledKinds(msptiActivityKind *buffer, uint32_t *bufferSize,
                                             uint32_t *enabledKindsCount)
{
    if (enabledKindsCount == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *enabledKindsCount = 0;
    for (auto &kindSwitch : activity_switch_)
    {
        if (kindSwitch.load(std::memory_order_relaxed))
        {
            (*enabledKindsCount)++;
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
    auto writeCount = std::min(*bufferSize, *enabledKindsCount);
    for (uint32_t i = 0, written = 0; i < activity_switch_.size() && written < writeCount; i++)
    {
        if (activity_switch_[i].load(std::memory_order_relaxed))
        {
            buffer[written++] = static_cast<msptiActivityKind>(i);
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::GetNextRecord(uint8_t *buffer, size_t validBufferSizeBytes, msptiActivity **record)
{
    if (buffer == nullptr)
    {
        MSPTI_LOGE("The address of Activity Buffer is nullptr.");
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    static thread_local size_t pos = 0;
    if (pos >= validBufferSizeBytes)
    {
        pos = 0;
        return MSPTI_ERROR_MAX_LIMIT_REACHED;
    }

    msptiActivityKind *pKind = Common::ReinterpretConvert<msptiActivityKind *>(buffer + pos);
    size_t size{0};
    if (UNLIKELY(!GetActivityStructSize(*pKind, &size) || size == 0))
    {
        MSPTI_LOGE("GetNextRecord failed, invalid kind: %d", *pKind);
        return MSPTI_ERROR_INVALID_KIND;
    }
    *record = Common::ReinterpretConvert<msptiActivity *>(buffer + pos);
    pos += size;
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::FlushAll()
{
    std::deque<std::unique_ptr<ActivityBuffer>> flushBuffers;
    {
        std::unique_lock<std::mutex> lck(cv_mtx_);
        flushBuffers = std::move(co_activity_buffers_);
    }
    for (const auto &buffer : flushBuffers)
    {
        if (buffer)
        {
            buffer->UnInit(bufferCompleted_handle_);
        }
    }
    {
        std::unique_lock<std::mutex> lck(cv_mtx_);
        JoinWorkThreads();
    }
    {
        std::lock_guard<std::recursive_mutex> lk(buf_mtx_);
        if (cur_buf_)
        {
            auto consumeBuf = std::move(cur_buf_);
            consumeBuf->UnInit(this->bufferCompleted_handle_);
        }
    }
    MSPTI_LOGI("Flush all activity buffer.");
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::FlushPeriod(uint32_t time)
{
    std::unique_lock<std::mutex> lck(cv_mtx_);
    if (time == 0)
    {
        flush_period_time_ = DEFAULT_PERIOD_FLUSH_TIME;
        flush_period_ = false;
    }
    else
    {
        flush_period_time_ = time;
        flush_period_ = true;
        cv_.notify_one();
    }
    return MSPTI_SUCCESS;
}

msptiResult ActivityManager::Record(msptiActivity *activity, size_t size)
{
    if (activity == nullptr)
    {
        return MSPTI_ERROR_INNER;
    }
    if (!IsActivityKindEnable(activity->kind))
    {
        return MSPTI_SUCCESS;
    }
    static const float ACTIVITY_BUFFER_THRESHOLD = 0.8;
    std::lock_guard<std::recursive_mutex> lk(buf_mtx_);
    if (!cur_buf_)
    {
        if (TryInitActivityBuffer() != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("Failed to record activity, kind %d.", static_cast<int>(activity->kind));
            return MSPTI_ERROR_INNER;
        }
    }
    else if (cur_buf_->ValidSize() >= ACTIVITY_BUFFER_THRESHOLD * cur_buf_->BufSize())
    {
        {
            std::unique_lock<std::mutex> lck(cv_mtx_);
            buf_full_ = true;
            co_activity_buffers_.emplace_back(std::move(cur_buf_));
            cv_.notify_one();
        }
        if (TryInitActivityBuffer() != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("Failed to record activity, kind %d.", static_cast<int>(activity->kind));
            return MSPTI_ERROR_INNER;
        }
    }
    if (cur_buf_->Record(activity, size) != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("Failed to record activity, kind %d.", static_cast<int>(activity->kind));
        cur_drop_num_++;
        total_drop_num_++;
        return MSPTI_ERROR_INNER;
    }
    total_record_num_++;
    return MSPTI_SUCCESS;
}

void ActivityManager::Run()
{
    pthread_setname_np(pthread_self(), "ActivityManager");
    MSPTI_LOGI("ActivityManager thread start running.");
    while (true)
    {
        {
            std::unique_lock<std::mutex> lk(cv_mtx_);
            bool serveForWaitFor = true;
            cv_.wait_for(lk, std::chrono::milliseconds(flush_period_time_),
                         [&]()
                         {
                             serveForWaitFor = !serveForWaitFor;
                             return (serveForWaitFor && flush_period_) || buf_full_ || !thread_run_.load();
                         });
            if (!thread_run_.load())
            {
                break;
            }
            {
                for (auto &activity_buffer : co_activity_buffers_)
                {
                    work_thread_.emplace_back(std::thread([this](std::unique_ptr<ActivityBuffer> activity_buffer)
                                                          { activity_buffer->UnInit(this->bufferCompleted_handle_); },
                                                          std::move(activity_buffer)));
                }
                co_activity_buffers_.clear();
                buf_full_ = false;
            }
        }
    }
    JoinWorkThreads();
    MSPTI_LOGI("ActivityManager thread stop running.");
}

msptiResult ActivityManager::SetDevice(uint32_t deviceId)
{
    MSPTI_LOGI("Set device: %u", deviceId);
    {
        std::lock_guard<std::mutex> lk(devices_mtx_);
        if (devices_.find(deviceId) != devices_.end())
        {
            MSPTI_LOGW("Device: %u is already set.", deviceId);
            return MSPTI_SUCCESS;
        }
        devices_.insert(deviceId);
    }
    if (std::find(activity_switch_.begin(), activity_switch_.end(), true) == activity_switch_.end())
    {
        return MSPTI_SUCCESS;
    }
    return Mspti::Ascend::DevTaskManager::GetInstance()->StartDevProfTask(deviceId, activity_switch_);
}

msptiResult ActivityManager::ResetDevice(uint32_t deviceId)
{
    MSPTI_LOGI("Reset device: %u", deviceId);
    {
        std::lock_guard<std::mutex> lk(devices_mtx_);
        auto iter = devices_.find(deviceId);
        if (iter == devices_.end())
        {
            MSPTI_LOGW("Device: %u is not set, nothing to reset.", deviceId);
            return MSPTI_SUCCESS;
        }
        devices_.erase(iter);
    }
    return Mspti::Ascend::DevTaskManager::GetInstance()->StopDevProfTask(deviceId, append_only_activity_switch_);
}

msptiResult ActivityManager::ResetAllDevice()
{
    auto ret = MSPTI_SUCCESS;
    {
        std::lock_guard<std::mutex> lk(devices_mtx_);
        for (const auto &device : devices_)
        {
            MSPTI_LOGI("Reset device: %u", device);
            auto temp =
                Mspti::Ascend::DevTaskManager::GetInstance()->StopDevProfTask(device, append_only_activity_switch_);
            if (temp != MSPTI_SUCCESS)
            {
                ret = temp;
            }
        }
    }
    return ret;
}

ActivityManager::ActivityManager() { ResetActivitySwitch(); }

const std::unordered_set<uint32_t> ActivityManager::GetAllValidDevice()
{
    std::lock_guard<std::mutex> lk(devices_mtx_);
    return devices_;
}
}  // namespace Activity
}  // namespace Mspti

msptiResult msptiActivityRegisterCallbacks(msptiBuffersCallbackRequestFunc funcBufferRequested,
                                           msptiBuffersCallbackCompleteFunc funcBufferCompleted)
{
    return Mspti::Activity::ActivityManager::GetInstance()->RegisterCallbacks(funcBufferRequested, funcBufferCompleted);
}

msptiResult msptiActivityEnable(msptiActivityKind kind)
{
    return Mspti::Activity::ActivityManager::GetInstance()->Register(kind);
}

msptiResult msptiActivityDisable(msptiActivityKind kind)
{
    return Mspti::Activity::ActivityManager::GetInstance()->UnRegister(kind);
}

bool msptiActivityIsEnabled(msptiActivityKind kind)
{
    return Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(kind);
}

msptiResult msptiActivityGetNextRecord(uint8_t *buffer, size_t validBufferSizeBytes, msptiActivity **record)
{
    return Mspti::Activity::ActivityManager::GetInstance()->GetNextRecord(buffer, validBufferSizeBytes, record);
}

msptiResult msptiActivityFlushAll(uint32_t flag)
{
    UNUSED(flag);
    return Mspti::Activity::ActivityManager::GetInstance()->FlushAll();
}

msptiResult msptiActivityFlushPeriod(uint32_t time)
{
    return Mspti::Activity::ActivityManager::GetInstance()->FlushPeriod(time);
}

msptiResult msptiActivityPushExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t id)
{
    return Mspti::Reporter::ExternalCorrelationReporter::GetInstance()->PushExternalCorrelationId(kind, id);
}

msptiResult msptiActivityPopExternalCorrelationId(msptiExternalCorrelationKind kind, uint64_t *lastId)
{
    return Mspti::Reporter::ExternalCorrelationReporter::GetInstance()->PopExternalCorrelationId(kind, lastId);
}

msptiResult msptiGetVersion(uint32_t *version)
{
    if (version == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    constexpr uint32_t INVALID_VERSION = 0;
    static auto msptiVersion = []() -> uint32_t
    {
        auto versionStr = Mspti::Common::GetCANNModuleVersion("mspti");
        MSPTI_LOGI("mspti version str: %s", versionStr.c_str());
        return Mspti::Common::ParseMsptiVersion(versionStr);
    }();
    if (UNLIKELY(msptiVersion == INVALID_VERSION))
    {
        return MSPTI_ERROR_INNER;
    }
    *version = msptiVersion;
    return MSPTI_SUCCESS;
}

msptiResult msptiActivityGetStructSize(msptiActivityKind activityKind, uint32_t version, size_t *activityStructSize)
{
    UNUSED(version);
    if (activityStructSize == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    size_t size{0};
    if (!Mspti::Activity::GetActivityStructSize(activityKind, &size) || size == 0)
    {
        return MSPTI_ERROR_INVALID_KIND;
    }
    *activityStructSize = size;
    return MSPTI_SUCCESS;
}

msptiResult msptiActivityGetEnabledKinds(msptiSubscriberHandle subscriber, msptiActivityKind *buffer,
                                         uint32_t *bufferSize, uint32_t *enabledKindsCount)
{
    UNUSED(subscriber);
    return Mspti::Activity::ActivityManager::GetInstance()->GetEnabledKinds(buffer, bufferSize, enabledKindsCount);
}

msptiResult msptiActivityGetNumDroppedRecords(void *context, uint32_t streamId, size_t *dropped)
{
    UNUSED(context);
    UNUSED(streamId);
    if (dropped == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *dropped = Mspti::Activity::ActivityManager::GetInstance()->GetAndResetDroppedCount();
    return MSPTI_SUCCESS;
}

msptiResult msptiGetTimestamp(uint64_t *timestamp)
{
    if (timestamp == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    *timestamp = Mspti::Common::ContextManager::GetInstance()->GetHostTimeStampNs();
    return MSPTI_SUCCESS;
}

msptiResult msptiActivityRegisterTimestampCallback(msptiTimestampCallbackFunc funcTimestamp)
{
    return Mspti::Common::ContextManager::GetInstance()->SetTimestampCallback(funcTimestamp);
}

msptiResult msptiActivitySetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
{
    if (valueSize == nullptr || value == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    size_t size = Mspti::Activity::GetActivityAttributeSize(attr);
    if (size == 0)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (*valueSize < size)
    {
        return MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT;
    }
    switch (attr)
    {
        case MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE:
        {
            uint32_t channelBufferSize{0};
            if (memcpy_s(&channelBufferSize, size, value, size) != EOK)
            {
                return MSPTI_ERROR_INNER;
            }
            return Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->SetChannelBufferSize(channelBufferSize);
        }
        case MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK:
        {
            msptiTimestampCallbackFunc funcTimestamp{nullptr};
            if (memcpy_s(&funcTimestamp, size, value, size) != EOK)
            {
                return MSPTI_ERROR_INNER;
            }
            return msptiActivityRegisterTimestampCallback(funcTimestamp);
        }
        default:
            return MSPTI_ERROR_INVALID_PARAMETER;
    }
}

msptiResult msptiActivityGetAttribute(msptiActivityAttribute attr, size_t *valueSize, void *value)
{
    if (valueSize == nullptr || value == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    size_t size = Mspti::Activity::GetActivityAttributeSize(attr);
    if (size == 0)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    if (*valueSize < size)
    {
        return MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT;
    }
    msptiResult ret{MSPTI_SUCCESS};
    switch (attr)
    {
        case MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE:
        {
            auto channelBufferSize = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetChannelBufferSize();
            if (memcpy_s(value, size, &channelBufferSize, sizeof(channelBufferSize)) != EOK)
            {
                ret = MSPTI_ERROR_INNER;
            }
            break;
        }
        case MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK:
        {
            auto funcTimestamp = Mspti::Common::ContextManager::GetInstance()->GetTimestampCallback();
            if (memcpy_s(value, size, &funcTimestamp, sizeof(funcTimestamp)) != EOK)
            {
                ret = MSPTI_ERROR_INNER;
            }
            break;
        }
        default:
            ret = MSPTI_ERROR_INVALID_PARAMETER;
            break;
    }
    if (ret != MSPTI_SUCCESS)
    {
        return ret;
    }
    *valueSize = size;
    return MSPTI_SUCCESS;
}
