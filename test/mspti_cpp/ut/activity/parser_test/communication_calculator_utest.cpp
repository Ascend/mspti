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

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/stars_common.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/communication_calculator.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/acl_inject.h"
#include "csrc/common/inject/profapi_inject.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
class CommunicationCalculatorUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
    virtual void TearDown() {}
};

// Observes the pending count while the last communication record is being reported.
// Member-function stubs take this as the first parameter (mockcpp hook convention).
std::atomic<int64_t> g_pendingDuringCommReport{-1};
msptiResult CapturePendingDuringCommReport(Mspti::Activity::ActivityManager * /*self*/, msptiActivity * /*activity*/,
                                           size_t /*size*/)
{
    g_pendingDuringCommReport.store(
        Mspti::Parser::CommunicationCalculator::GetInstance().GetPendingCommunicationCount(),
        std::memory_order_relaxed);
    return MSPTI_SUCCESS;
}

TEST_F(CommunicationCalculatorUtest, ShouldReturnSuccessWhenChildEmpty)
{
    std::unique_ptr<Mspti::Parser::ApiEvent> api2TaskInfo = std::make_unique<Mspti::Parser::ApiEvent>();
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    EXPECT_EQ(instance.AppendApi2TaskInfo(*api2TaskInfo), MSPTI_SUCCESS);
}

TEST_F(CommunicationCalculatorUtest, ShouldReturnSuccess)
{
    uint32_t threadId = 2;
    uint64_t beginTime = 100;
    uint64_t endTime = 100;
    uint64_t subBeginTime = 110;
    uint64_t subEndTime = 180;
    uint16_t level = MSPROF_REPORT_HCCL_NODE_LEVEL;
    std::unique_ptr<Mspti::Parser::ApiEvent> api2TaskInfo = std::make_unique<Mspti::Parser::ApiEvent>();
    api2TaskInfo->api.beginTime = beginTime;
    api2TaskInfo->api.endTime = endTime;
    api2TaskInfo->api.threadId = threadId;
    std::unique_ptr<Mspti::Parser::ApiEvent> subApi2TaskInfo = std::make_unique<Mspti::Parser::ApiEvent>();
    subApi2TaskInfo->api.beginTime = subBeginTime;
    subApi2TaskInfo->api.endTime = subEndTime;
    api2TaskInfo->children.push_back(*subApi2TaskInfo);
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    EXPECT_EQ(instance.AppendApi2TaskInfo(*api2TaskInfo), MSPTI_SUCCESS);
}

TEST_F(CommunicationCalculatorUtest, ShouldReturnSuccessAppendCompactInfo)
{
    bool agingFlag = 1;
    uint8_t dataType = 1;
    uint64_t dataCount = 1;
    MsprofCompactInfo data;
    (void)memset_s(&data, sizeof(data), 0, sizeof(data));
    data.data.hcclopInfo.dataType = dataType;
    data.data.hcclopInfo.groupName = Mspti::Parser::CannHashCache::GenHashId("hcom_1");
    data.data.hcclopInfo.algType = Mspti::Parser::CannHashCache::GenHashId("mesh");
    data.data.hcclopInfo.algType = dataCount;
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance.AppendCompactInfo(agingFlag, &data));
    EXPECT_EQ(MSPTI_SUCCESS,
              Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    EXPECT_EQ(MSPTI_SUCCESS, instance.AppendCompactInfo(agingFlag, &data));
}

TEST_F(CommunicationCalculatorUtest, PendingCountClearedOnUnregister)
{
    auto *mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    // Enable first (Register with no device touches no driver).
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    const int64_t baseline = instance.GetPendingCommunicationCount();

    // Aging task registration +1 pending.
    Mspti::Parser::ApiEvent apiEvent{};
    apiEvent.agingFlag = true;
    apiEvent.compactInfo.data.runtimeTrack.deviceId = 0;
    apiEvent.compactInfo.data.runtimeTrack.streamId = 3;
    apiEvent.compactInfo.data.runtimeTrack.taskInfo = 8101;
    apiEvent.api.beginTime = 100;
    apiEvent.api.endTime = 200;
    instance.AppendCommunicationTask(apiEvent);
    EXPECT_EQ(baseline + 1, instance.GetPendingCommunicationCount());

    // Unaging (graph) entries never count toward pending.
    Mspti::Parser::ApiEvent graphEvent{};
    graphEvent.agingFlag = false;
    graphEvent.compactInfo.data.runtimeTrack.deviceId = 0;
    graphEvent.compactInfo.data.runtimeTrack.streamId = 3;
    graphEvent.compactInfo.data.runtimeTrack.taskInfo = 8102;
    instance.AppendCommunicationTask(graphEvent);
    EXPECT_EQ(baseline + 1, instance.GetPendingCommunicationCount());

    // UnRegister ends with Clear: residuals removed, pending back to zero.
    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    EXPECT_EQ(0, instance.GetPendingCommunicationCount());
}

TEST_F(CommunicationCalculatorUtest, OverwriteSameDstKeyReturnsOldPendingCount)
{
    auto *mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    const int64_t baseline = instance.GetPendingCommunicationCount();

    auto makeAgingEvent = [](uint64_t parentEventId)
    {
        Mspti::Parser::ApiEvent apiEvent{};
        apiEvent.parentEventId = parentEventId;
        apiEvent.agingFlag = true;
        apiEvent.compactInfo.data.runtimeTrack.deviceId = 0;
        apiEvent.compactInfo.data.runtimeTrack.streamId = 3;
        apiEvent.compactInfo.data.runtimeTrack.taskInfo = 8300;
        apiEvent.api.beginTime = 100;
        apiEvent.api.endTime = 200;
        return apiEvent;
    };

    // First aging task on this dstKey: +1 pending.
    auto first = makeAgingEvent(101);
    instance.AppendCommunicationTask(first);
    EXPECT_EQ(baseline + 1, instance.GetPendingCommunicationCount());

    // Same dstKey, different op: overwrite must return the old entry's count,
    // otherwise the orphaned +1 never drains and a later consume underflows.
    auto second = makeAgingEvent(102);
    instance.AppendCommunicationTask(second);
    EXPECT_EQ(baseline + 1, instance.GetPendingCommunicationCount());

    // Overwriting with a non-aging (graph) task returns the count as well.
    Mspti::Parser::ApiEvent graphEvent{};
    graphEvent.parentEventId = 103;
    graphEvent.agingFlag = false;
    graphEvent.compactInfo.data.runtimeTrack.deviceId = 0;
    graphEvent.compactInfo.data.runtimeTrack.streamId = 3;
    graphEvent.compactInfo.data.runtimeTrack.taskInfo = 8300;
    instance.AppendCommunicationTask(graphEvent);
    EXPECT_EQ(baseline, instance.GetPendingCommunicationCount());

    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    EXPECT_EQ(0, instance.GetPendingCommunicationCount());
}

TEST_F(CommunicationCalculatorUtest, PendingHeldUntilLastReportCompletes)
{
    auto *mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    auto &devTasks = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    const int64_t baseline = instance.GetPendingCommunicationCount();

    // The last op must be reported exactly once; capture pending while it runs.
    g_pendingDuringCommReport.store(-1, std::memory_order_relaxed);
    MOCKER_CPP(&Mspti::Activity::ActivityManager::Record)
        .expects(exactly(1))
        .will(invoke(CapturePendingDuringCommReport));

    constexpr uint16_t kDevice = 0;
    constexpr uint16_t kTrackStream = 7;
    constexpr uint32_t kTaskInfo = 9400;
    constexpr uint64_t kEventId = 771;
    constexpr uint64_t kThreadId = 5;
    // Device soc logs carry converted ids; derive them with the same conversion.
    const uint16_t socStream = Mspti::Convert::StarsCommon::GetStreamId(kTrackStream, static_cast<uint16_t>(kTaskInfo));
    const uint32_t socTask = Mspti::Convert::StarsCommon::GetHostTaskId(kTrackStream, kTaskInfo, kDevice);

    // Host task side: +1 pending.
    Mspti::Parser::ApiEvent taskEvent{};
    taskEvent.parentEventId = kEventId;
    taskEvent.agingFlag = true;
    taskEvent.compactInfo.data.runtimeTrack.deviceId = kDevice;
    taskEvent.compactInfo.data.runtimeTrack.streamId = kTrackStream;
    taskEvent.compactInfo.data.runtimeTrack.taskInfo = kTaskInfo;
    taskEvent.api.beginTime = 100;
    taskEvent.api.endTime = 200;
    instance.AppendCommunicationTask(taskEvent);
    EXPECT_EQ(baseline + 1, instance.GetPendingCommunicationCount());

    // Host op side: marks the entry last so the device hit reports it.
    Mspti::Parser::ApiEvent childEvent{};
    childEvent.agingFlag = true;
    childEvent.compactInfo.data.runtimeTrack.deviceId = kDevice;
    childEvent.compactInfo.data.runtimeTrack.streamId = kTrackStream;
    childEvent.compactInfo.data.runtimeTrack.taskInfo = kTaskInfo;
    childEvent.api.beginTime = 100;
    childEvent.api.endTime = 200;
    Mspti::Parser::ApiEvent opEvent{};
    opEvent.eventId = kEventId;
    opEvent.agingFlag = true;
    opEvent.api.beginTime = 100;
    opEvent.api.endTime = 200;
    opEvent.api.threadId = kThreadId;
    opEvent.children.push_back(childEvent);
    EXPECT_EQ(MSPTI_SUCCESS, instance.AppendApi2TaskInfo(opEvent));

    // Host addition info the report reads.
    MsprofCompactInfo compact{};
    (void)memset_s(&compact, sizeof(compact), 0, sizeof(compact));
    compact.threadId = kThreadId;
    compact.timeStamp = 150;
    compact.data.hcclopInfo.dataType = 1;
    compact.data.hcclopInfo.groupName = Mspti::Parser::CannHashCache::GenHashId("hcom_1");
    compact.data.hcclopInfo.algType = Mspti::Parser::CannHashCache::GenHashId("mesh");
    compact.data.hcclopInfo.count = 1;
    EXPECT_EQ(MSPTI_SUCCESS, instance.AppendCompactInfo(true, &compact));

    // Device side: BEGIN then END drives CommunicationCalculator::Record.
    Mspti::HalLogData beginLog{};
    (void)memset_s(&beginLog, sizeof(beginLog), 0, sizeof(beginLog));
    beginLog.type = Mspti::ACSQ_LOG;
    beginLog.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    beginLog.acsq.streamId = socStream;
    beginLog.acsq.taskId = socTask;
    beginLog.acsq.timestamp = 1000;
    Mspti::HalLogData endLog = beginLog;
    endLog.acsq.funcType = STARS_FUNC_TYPE_END;
    endLog.acsq.timestamp = 2000;
    EXPECT_EQ(MSPTI_SUCCESS, devTasks.ReportStarsSocLog(kDevice, beginLog));
    EXPECT_EQ(MSPTI_SUCCESS, devTasks.ReportStarsSocLog(kDevice, endLog));

    // The count must still be held while the report runs (released only after).
    EXPECT_EQ(baseline + 1, g_pendingDuringCommReport.load(std::memory_order_relaxed));
    // ... and released once consumption completes.
    EXPECT_EQ(baseline, instance.GetPendingCommunicationCount());

    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    EXPECT_EQ(0, instance.GetPendingCommunicationCount());
    GlobalMockObject::verify();
}

// Reports (task + addition info) issued after the gate closes must be dropped.
TEST_F(CommunicationCalculatorUtest, HostReportDroppedAfterUnregisterStarts)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::FlushDevProfData).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    auto *mgr = Mspti::Activity::ActivityManager::GetInstance();
    auto &instance = Mspti::Parser::CommunicationCalculator::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, mgr->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));

    // Seed one residual so the retry loop runs its full window (flush is mocked instant).
    Mspti::Parser::ApiEvent seed{};
    seed.agingFlag = true;
    seed.compactInfo.data.runtimeTrack.deviceId = 0;
    seed.compactInfo.data.runtimeTrack.streamId = 3;
    seed.compactInfo.data.runtimeTrack.taskInfo = 8200;
    seed.api.beginTime = 100;
    seed.api.endTime = 200;
    instance.AppendCommunicationTask(seed);
    const int64_t preCount = instance.GetPendingCommunicationCount();
    EXPECT_EQ(MSPTI_SUCCESS, mgr->SetDevice(0));

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
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
            while (mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_COMMUNICATION))
            {
                if (std::chrono::steady_clock::now() > deadline)
                {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            sawGate.store(true);
            for (uint32_t i = 1; i <= 3; ++i)
            {
                if (mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_COMMUNICATION))
                {
                    allSawClosedGate.store(false);
                }
                Mspti::Parser::ApiEvent apiEvent{};
                apiEvent.agingFlag = true;
                apiEvent.compactInfo.data.runtimeTrack.deviceId = 0;
                apiEvent.compactInfo.data.runtimeTrack.streamId = 3;
                apiEvent.compactInfo.data.runtimeTrack.taskInfo = 8200 + i;
                apiEvent.api.beginTime = 100;
                apiEvent.api.endTime = 200;
                instance.AppendCommunicationTask(apiEvent);
                MsprofCompactInfo hccl{};
                (void)memset_s(&hccl, sizeof(hccl), 0, sizeof(hccl));
                msptiResult rc = instance.AppendCompactInfo(true, &hccl);
                if (rc != MSPTI_SUCCESS)
                {
                    reportStatus.store(rc);
                }
            }
            duringCount = instance.GetPendingCommunicationCount();
            // Gate closed but kind still enabled means UnRegister hasn't finished.
            gateStillOpen = !mgr->IsHostReportAllowed(MSPTI_ACTIVITY_KIND_COMMUNICATION) &&
                            mgr->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_COMMUNICATION);
        });

    EXPECT_EQ(MSPTI_SUCCESS, mgr->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION));
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
    // Clear has removed the seed residual; detach device to restore state.
    EXPECT_EQ(0, instance.GetPendingCommunicationCount());
    EXPECT_EQ(MSPTI_SUCCESS, mgr->ResetDevice(0));
    // Reset mocks to avoid leaking into later test files.
    GlobalMockObject::verify();
}
}  // namespace
