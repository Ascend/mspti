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

#include <atomic>
#include <chrono>
#include <thread>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/kernel_parser.h"
#include "csrc/common/context_manager.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
constexpr uint32_t TS_TASK_TYPE_KERNEL_AIVEC = 66;
constexpr uint32_t UNKNOWN_TASK_TYPE = 9999;
constexpr uint32_t TEST_DEVICE_ID = 0;
constexpr uint32_t TEST_STREAM_ID = 3;
constexpr uint32_t TEST_THREAD_ID = 6101;

class KernelParserPendingUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        Mspti::Parser::CannHashCache::RegTypeHashInfo(MSPROF_REPORT_RUNTIME_LEVEL, TS_TASK_TYPE_KERNEL_AIVEC,
                                                      "KERNEL_AIVEC");
        MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
        MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
        MOCKER_CPP(&Mspti::Ascend::DevTaskManager::FlushDevProfData).stubs().will(returnValue(MSPTI_SUCCESS));
        MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
            .stubs()
            .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));
        MOCKER_CPP(&Mspti::Activity::ActivityManager::Record).stubs().will(returnValue(MSPTI_SUCCESS));
        EXPECT_EQ(MSPTI_SUCCESS, Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_KERNEL));
    }

    virtual void TearDown()
    {
        // Pending is expected to be drained; UnRegister takes the fast path.
        EXPECT_EQ(MSPTI_SUCCESS,
                  Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_KERNEL));
        GlobalMockObject::verify();
    }

    MsprofCompactInfo MakeCompactInfo(uint32_t taskId, uint32_t taskType)
    {
        MsprofCompactInfo data;
        (void)memset_s(&data, sizeof(data), 0, sizeof(data));
        data.threadId = TEST_THREAD_ID;
        data.data.runtimeTrack.deviceId = TEST_DEVICE_ID;
        data.data.runtimeTrack.streamId = TEST_STREAM_ID;
        data.data.runtimeTrack.taskInfo = taskId;
        data.data.runtimeTrack.taskType = taskType;
        return data;
    }

    Mspti::HalLogData MakeSocLog(uint32_t taskId, uint16_t funcType)
    {
        Mspti::HalLogData socLog;
        (void)memset_s(&socLog, sizeof(socLog), 0, sizeof(socLog));
        socLog.type = Mspti::ACSQ_LOG;
        socLog.acsq.funcType = funcType;
        socLog.acsq.streamId = TEST_STREAM_ID;
        socLog.acsq.taskId = taskId;
        return socLog;
    }
};

// Aging host report +1; BEGIN moves it without changing pending; END consumes it back to baseline.
TEST_F(KernelParserPendingUtest, HostAgingReportIncreasesPendingUntilDeviceMatches)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t taskId = 7101;
    const int64_t baseline = instance.GetPendingKernelCount();

    auto compact = MakeCompactInfo(taskId, TS_TASK_TYPE_KERNEL_AIVEC);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compact));
    EXPECT_EQ(baseline + 1, instance.GetPendingKernelCount());

    auto beginLog = MakeSocLog(taskId, STARS_FUNC_TYPE_BEGIN);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, beginLog));
    EXPECT_EQ(baseline + 1, instance.GetPendingKernelCount());

    auto endLog = MakeSocLog(taskId, STARS_FUNC_TYPE_END);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, endLog));
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());
}

// Unaging (graph) kernels never count toward pending: after host report, BEGIN, and END.
TEST_F(KernelParserPendingUtest, UnagingReportDoesNotChangePending)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t taskId = 7102;
    const int64_t baseline = instance.GetPendingKernelCount();

    auto compact = MakeCompactInfo(taskId, TS_TASK_TYPE_KERNEL_AIVEC);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(0, &compact));
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());

    auto beginLog = MakeSocLog(taskId, STARS_FUNC_TYPE_BEGIN);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, beginLog));
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());

    auto endLog = MakeSocLog(taskId, STARS_FUNC_TYPE_END);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, endLog));
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());
}

// Non-kernel task types are filtered at entry and never counted.
TEST_F(KernelParserPendingUtest, NonKernelTaskTypeDoesNotChangePending)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t taskId = 7103;
    const int64_t baseline = instance.GetPendingKernelCount();

    auto compact = MakeCompactInfo(taskId, UNKNOWN_TASK_TYPE);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compact));
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());
}

// Swap+push must not open a zero window: concurrent pollers must never observe
// below the true outstanding count (no timing assertions; the poller only records).
TEST_F(KernelParserPendingUtest, PendingNeverDipsDuringHostTaskSwap)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t kBatchBase = 9000;
    constexpr int kBatchSize = 2000;
    const int64_t baseline = instance.GetPendingKernelCount();

    for (int i = 0; i < kBatchSize; ++i)
    {
        auto compact = MakeCompactInfo(kBatchBase + i, TS_TASK_TYPE_KERNEL_AIVEC);
        EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compact));
    }
    EXPECT_EQ(baseline + kBatchSize, instance.GetPendingKernelCount());

    // Spin a poller across the swap+push window; it records the minimum observed count.
    std::atomic<int64_t> minObserved{baseline + kBatchSize};
    std::atomic<bool> stopPolling{false};
    std::thread poller(
        [&]()
        {
            while (!stopPolling.load(std::memory_order_relaxed))
            {
                const int64_t sampled = instance.GetPendingKernelCount();
                int64_t cur = minObserved.load(std::memory_order_relaxed);
                while (sampled < cur && !minObserved.compare_exchange_weak(cur, sampled, std::memory_order_relaxed))
                {
                }
            }
        });

    // One unmatched soc log forces the whole batch through swap+push without consuming it.
    // NOTE: keep this id outside [kBatchBase, kBatchBase + kBatchSize) so it matches nothing.
    auto wedgeLog = MakeSocLog(15000, STARS_FUNC_TYPE_BEGIN);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, wedgeLog));
    stopPolling.store(true, std::memory_order_relaxed);
    poller.join();

    // Swap+push never touches the count, so every sample reads the full batch.
    EXPECT_EQ(baseline + kBatchSize, minObserved.load(std::memory_order_relaxed));
    EXPECT_EQ(baseline + kBatchSize, instance.GetPendingKernelCount());

    // Drain everything so TearDown stays on the fast path.
    for (int i = 0; i < kBatchSize; ++i)
    {
        auto beginLog = MakeSocLog(kBatchBase + i, STARS_FUNC_TYPE_BEGIN);
        EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, beginLog));
        auto endLog = MakeSocLog(kBatchBase + i, STARS_FUNC_TYPE_END);
        EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, endLog));
    }
    EXPECT_EQ(baseline, instance.GetPendingKernelCount());
}

// Staged device BEGINs must not survive Clear: a post-Clear END for a graph task
// (whose device map entry survives by design) must find no stale start time.
TEST_F(KernelParserPendingUtest, ClearDropsStagedDeviceBeginState)
{
    auto* mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t taskId = 7300;

    // Unaging task + BEGIN stages the start time; nothing is counted.
    auto compact = MakeCompactInfo(taskId, TS_TASK_TYPE_KERNEL_AIVEC);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(0, &compact));
    auto beginLog = MakeSocLog(taskId, STARS_FUNC_TYPE_BEGIN);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, beginLog));

    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_KERNEL));
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_KERNEL));

    // Drop the fixture stubs and require silence: the stale stage must not report.
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Activity::ActivityManager::Record).expects(never()).will(returnValue(MSPTI_SUCCESS));
    auto endLog = MakeSocLog(taskId, STARS_FUNC_TYPE_END);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(TEST_DEVICE_ID, endLog));
    GlobalMockObject::verify();
}

// Unmatched host kernels are dropped by Clear after retries give up.
TEST_F(KernelParserPendingUtest, UnRegisterClearsUnmatchedKernels)
{
    auto* mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint32_t taskId = 7104;
    const int64_t baseline = instance.GetPendingKernelCount();

    auto compact = MakeCompactInfo(taskId, TS_TASK_TYPE_KERNEL_AIVEC);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compact));
    EXPECT_EQ(baseline + 1, instance.GetPendingKernelCount());

    // Attach a device to enter the retry branch; feed no device data so retries give up.
    EXPECT_EQ(MSPTI_SUCCESS, mgr->SetDevice(TEST_DEVICE_ID));
    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_KERNEL));
    EXPECT_EQ(0, instance.GetPendingKernelCount());

    // Restore state for TearDown (idempotent re-UnRegister).
    EXPECT_EQ(MSPTI_SUCCESS, mgr->ResetDevice(TEST_DEVICE_ID));
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_KERNEL));
}

// Reports issued after the host gate closes must be dropped; no timing assertions.
TEST_F(KernelParserPendingUtest, HostReportDroppedAfterUnregisterStarts)
{
    auto* mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    // Seed one residual so the retry loop runs its full window (flush is mocked instant).
    auto seed = MakeCompactInfo(7200, TS_TASK_TYPE_KERNEL_AIVEC);
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &seed));
    const int64_t preCount = instance.GetPendingKernelCount();
    EXPECT_EQ(MSPTI_SUCCESS, mgr->SetDevice(TEST_DEVICE_ID));

    std::atomic<bool> sawGate{false};
    // No gtest assertions in worker thread; record state and assert after join.
    std::atomic<msptiResult> reportStatus{MSPTI_SUCCESS};
    // Gate observed closed before each report proves the drop branch was taken.
    std::atomic<bool> allSawClosedGate{true};
    int64_t duringCount = 0;
    bool gateStillOpen = false;
    std::thread reporter(
        [&]()
        {
            // Wait for UnRegister to close the host gate (5s escape).
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            while (mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_KERNEL))
            {
                if (std::chrono::steady_clock::now() > deadline)
                {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            sawGate.store(true);
            for (uint32_t i = 1; i <= 5; ++i)
            {
                if (mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_KERNEL))
                {
                    allSawClosedGate.store(false);
                }
                auto compact = MakeCompactInfo(7200 + i, TS_TASK_TYPE_KERNEL_AIVEC);
                msptiResult rc = instance.ReportRtTaskTrack(1, &compact);
                if (rc != MSPTI_SUCCESS)
                {
                    reportStatus.store(rc);
                }
            }
            duringCount = instance.GetPendingKernelCount();
            // Gate closed but kind still enabled means UnRegister hasn't finished (Clear not run).
            gateStillOpen = !mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_KERNEL) &&
                            mgr->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL);
        });

    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_KERNEL));
    reporter.join();

    EXPECT_TRUE(sawGate.load());
    EXPECT_EQ(MSPTI_SUCCESS, reportStatus.load());
    // All reports were issued while the gate was closed.
    EXPECT_TRUE(allSawClosedGate.load());
    if (gateStillOpen)
    {
        // Assert only if sampled before UnRegister finished; skip otherwise to avoid flakes.
        EXPECT_EQ(preCount, duringCount);
    }
    // Clear has removed the seed; restore state for TearDown.
    EXPECT_EQ(0, instance.GetPendingKernelCount());
    EXPECT_EQ(MSPTI_SUCCESS, mgr->ResetDevice(TEST_DEVICE_ID));
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_KERNEL));
}
}  // namespace
