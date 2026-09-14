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

#ifndef MSPTI_ACTIVITY_ACTIVITY_MANAGER_H
#define MSPTI_ACTIVITY_ACTIVITY_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_set>
#include <vector>

#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/config.h"
#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Activity
{

class ActivityBuffer
{
   public:
    ActivityBuffer() = default;
    void Init(msptiBuffersCallbackRequestFunc func);
    void UnInit(msptiBuffersCallbackCompleteFunc func);
    msptiResult Record(msptiActivity *activity, size_t size);
    bool BufValid();
    size_t BufSize();
    size_t ValidSize();

   private:
    uint8_t *buf_{nullptr};
    size_t buf_size_{0};
    size_t records_num_{0};
    size_t valid_size_{0};
};

// Singleton
class ActivityManager
{
   public:
    using ActivitySwitchType = Mspti::Ascend::DevTaskManager::ActivitySwitchType;
    static constexpr uint32_t FLUSH_RETRY_MAX_COUNT = 5;
    static constexpr uint32_t FLUSH_RETRY_WAIT_MS = 20;
    static ActivityManager *GetInstance();
    msptiResult RegisterCallbacks(msptiBuffersCallbackRequestFunc funcBufferRequested,
                                  msptiBuffersCallbackCompleteFunc funcBufferCompleted);
    msptiResult FlushPeriod(uint32_t time);
    msptiResult Record(msptiActivity *activity, size_t size);
    static msptiResult GetNextRecord(uint8_t *buffer, size_t validBufferSizeBytes, msptiActivity **record);
    msptiResult FlushAll();
    msptiResult SetDevice(uint32_t deviceId);
    msptiResult ResetDevice(uint32_t deviceId);
    msptiResult ResetAllDevice();
    msptiResult Register(msptiActivityKind kind);
    msptiResult UnRegister(msptiActivityKind kind);
    bool IsActivityKindEnable(msptiActivityKind kind);
    bool IsHostReportAllowed(msptiActivityKind kind);
    msptiResult GetEnabledKinds(msptiActivityKind *buffer, uint32_t *bufferSize, uint32_t *enabledKindsCount);
    size_t GetAndResetDroppedCount() { return cur_drop_num_.exchange(0, std::memory_order_relaxed); }

    const std::unordered_set<uint32_t> GetAllValidDevice();  // 仅返回当前device快照
    void StartActivityMgrThread();
    void StopActivityMgrThread();

   private:
    ActivityManager();
    ~ActivityManager();
    explicit ActivityManager(const ActivityManager &obj) = delete;
    ActivityManager &operator=(const ActivityManager &obj) = delete;
    explicit ActivityManager(ActivityManager &&obj) = delete;
    ActivityManager &operator=(ActivityManager &&obj) = delete;
    void Run();
    void ResetActivitySwitch();
    void JoinWorkThreads();
    msptiResult TryInitActivityBuffer();

   private:
    const static std::set<msptiActivityKind> supportActivityKinds_;
    // Replace map with bitest
    ActivitySwitchType activity_switch_;
    ActivitySwitchType append_only_activity_switch_;
    ActivitySwitchType unregistering_;
    std::unordered_set<uint32_t> devices_;
    std::mutex devices_mtx_;

    std::unique_ptr<std::thread> activity_mgr_thread_{nullptr};
    std::atomic<bool> thread_run_{false};
    std::condition_variable cv_;
    std::mutex cv_mtx_;
    std::mutex thread_mtx_;  // 保护 Start/Stop 线程的临界区
    bool buf_full_{false};
    bool flush_period_{false};
    uint32_t flush_period_time_{DEFAULT_PERIOD_FLUSH_TIME};
    std::vector<std::thread> work_thread_;

    std::deque<std::unique_ptr<ActivityBuffer>> co_activity_buffers_;
    std::unique_ptr<ActivityBuffer> cur_buf_;
    std::recursive_mutex buf_mtx_;

    std::atomic<size_t> cur_drop_num_{0};
    std::atomic<size_t> total_drop_num_{0};
    std::atomic<size_t> total_record_num_{0};

    msptiBuffersCallbackRequestFunc bufferRequested_handle_ = nullptr;
    msptiBuffersCallbackCompleteFunc bufferCompleted_handle_ = nullptr;
};

}  // namespace Activity
}  // namespace Mspti

#endif  // MSPTI_ACTIVITY_ACTIVITY_MANAGER_H
