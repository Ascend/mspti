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

#ifndef MSPTI_COMMON_CONTEXT_MANAGER_H
#define MSPTI_COMMON_CONTEXT_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "csrc/common/concurrent_map.h"
#include "csrc/include/mspti_activity.h"
#include "csrc/include/mspti_result.h"

namespace Mspti
{
namespace Common
{
struct DevTimeInfo
{
    uint64_t freq{0};
    uint64_t startRealTime{0};
    uint64_t startSysCnt{0};
    uint64_t startMonotonicRawNs{0};
};

enum class PlatformType
{
    CHIP_910B = 5,
    CHIP_310B = 7,
    CHIP_V6 = 15,
    END_TYPE
};

struct ContextInfo
{
    PlatformType platformType;
    std::once_flag flag;
};

class ContextManager final
{
   public:
    static ContextManager* GetInstance();

    // Time calibration initialization
    void InitDeviceTimeInfo(uint32_t deviceId);
    void InitHostTimeInfo();
    bool IsHostFreqEnabled();
    bool GetHostTimeInfo(DevTimeInfo& devTimeInfo);
    bool GetDeviceTimeInfo(uint32_t deviceId, DevTimeInfo& devTimeInfo);

    // Counter -> real time conversion (pure, static)
    static uint64_t ConvertCounterToRealTime(uint64_t sysCnt, const DevTimeInfo& devTimeInfo);
    static uint64_t ConvertSysCntToRealTime(uint64_t sysCnt, const DevTimeInfo& devTimeInfo);
    static uint64_t ConvertMonotonicCounterToRealTime(uint64_t monotonicNs, const DevTimeInfo& devTimeInfo);
    // Host counter -> real time conversion
    uint64_t GetHostRealTime(uint64_t sysCnt);
    // Device counter -> real time conversion
    uint64_t GetDeviceRealTime(uint32_t deviceId, uint64_t sysCnt);
    std::vector<uint64_t> GetDeviceRealTime(uint32_t deviceId, const std::vector<uint64_t>& sysCnts);

    // Host real-time accessors
    uint64_t GetHostTimeStampNs();
    uint64_t GetHostSysCnt();
    msptiResult SetTimestampCallback(msptiTimestampCallbackFunc funcTimestamp);

    PlatformType GetChipType(uint32_t deviceId);
    uint64_t GetCorrelationId(uint32_t threadId = 0);
    uint64_t UpdateAndReportCorrelationId();
    uint64_t UpdateAndReportCorrelationId(uint32_t tid);

    msptiResult StartSyncTime();
    msptiResult StopSyncTime();

    static uint64_t EncodeDstKey(uint16_t deviceId, uint16_t streamId, uint32_t taskId);
    static std::tuple<uint16_t, uint16_t, uint32_t> DecodeDstKey(uint64_t key);

   private:
    ContextManager() = default;
    ~ContextManager();
    explicit ContextManager(const ContextManager& obj) = delete;
    ContextManager& operator=(const ContextManager& obj) = delete;
    explicit ContextManager(ContextManager&& obj) = delete;
    ContextManager& operator=(ContextManager&& obj) = delete;

    void Run();

    uint64_t GetCurrentHostRealTimeNs();

   private:
    std::unordered_map<uint32_t, std::unique_ptr<DevTimeInfo>> devTimeInfo_;
    std::mutex devTimeMtx_;

    std::once_flag hostTimeInfoInitFlag_;
    std::mutex hostTimeMtx_;
    std::unique_ptr<DevTimeInfo> hostTimeInfo_;

    std::atomic<bool> isQuit_{false};
    std::thread t_;

    std::atomic<uint64_t> correlationId_{1};

    std::mutex cv_mutex_;
    std::condition_variable cv_;

    ConcurrentMap<uint32_t, uint64_t> threadCorrelationIdInfo_;
    std::unordered_map<uint32_t, ContextInfo> deviceInfoCache_;

    std::atomic<msptiTimestampCallbackFunc> timestampCallback_{nullptr};
};
}  // namespace Common
}  // namespace Mspti

#endif  // MSPTI_COMMON_CONTEXT_MANAGER_H
