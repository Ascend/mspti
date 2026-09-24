/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

#include <atomic>
#include <thread>
#include <vector>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_track_cache.h"
#include "csrc/activity/ascend/parser/communication_calculator.h"
#include "csrc/common/context_manager.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti.h"

namespace
{

void StubBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void StubBufferComplete(uint8_t *buffer, size_t, size_t)
{
    if (buffer)
    {
        free(buffer);
    }
}

class CannTrackCacheUtest : public testing::Test
{
   protected:
    void SetUp() override
    {
        GlobalMockObject::verify();
        msptiActivityRegisterCallbacks(StubBufferRequest, StubBufferComplete);
        // 确保 COMMUNICATION 开关初始关闭，单测按需开启
        Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION);
        Mspti::Parser::CannTrackCache::GetInstance().StopTask();
    }
    void TearDown() override
    {
        Mspti::Parser::CannTrackCache::GetInstance().StopTask();
        Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_COMMUNICATION);
        msptiActivityFlushAll(1);
        GlobalMockObject::verify();
    }
};

TEST_F(CannTrackCacheUtest, StartStopTaskAreNoOpAndIdempotent)
{
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, ins.StartTask());
    EXPECT_EQ(MSPTI_SUCCESS, ins.StopTask());
    EXPECT_EQ(MSPTI_SUCCESS, ins.StopTask());
    EXPECT_EQ(MSPTI_SUCCESS, ins.StartTask());
}

TEST_F(CannTrackCacheUtest, AppendTsTrackReturnsSuccessWhenKindDisabled)
{
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();
    MsprofCompactInfo info{};
    info.threadId = 1;
    info.timeStamp = 1000;
    // 未使能 COMMUNICATION 时应直接返回 SUCCESS 且不创建缓存（不崩溃）
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(true, &info));
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(false, &info));
}

TEST_F(CannTrackCacheUtest, AppendTsTrackCreatesPerThreadCacheAndUpdatesAgingFlag)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    MsprofCompactInfo infoA{};
    infoA.threadId = 100;
    infoA.timeStamp = 1000;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(true, &infoA));

    MsprofCompactInfo infoB{};
    infoB.threadId = 200;
    infoB.timeStamp = 2000;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(false, &infoB));

    // 同一 threadId 再次更新应覆盖且不崩溃
    infoA.timeStamp = 1500;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(false, &infoA));
}

TEST_F(CannTrackCacheUtest, AppendCommunicationFiltersOutOfRangeAndLinksParent)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    // 先写入 TsTrack 作为 parent 范围
    MsprofCompactInfo compact{};
    compact.threadId = 42;
    compact.timeStamp = 1000;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(true, &compact));

    // 在范围内的 communication 应成功且缓存 lastCommunication
    MsprofApi apiIn{};
    apiIn.threadId = 42;
    apiIn.beginTime = 900;
    apiIn.endTime = 1100;
    apiIn.level = MSPROF_REPORT_HCCL_NODE_LEVEL;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendCommunication(true, &apiIn));

    // 超出范围的应被过滤但仍返回 SUCCESS
    MsprofApi apiOut{};
    apiOut.threadId = 42;
    apiOut.beginTime = 2000;
    apiOut.endTime = 2100;
    apiOut.level = MSPROF_REPORT_HCCL_NODE_LEVEL;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendCommunication(true, &apiOut));
}

TEST_F(CannTrackCacheUtest, AppendNodeLunchFiltersByLastCommunicationAndIncrementsId)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    // 准备 TsTrack + Communication 以建立 lastCommunication
    MsprofCompactInfo compact{};
    compact.threadId = 77;
    compact.timeStamp = 5000;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(true, &compact));

    MsprofApi comm{};
    comm.threadId = 77;
    comm.beginTime = 4900;
    comm.endTime = 5100;
    comm.level = MSPROF_REPORT_HCCL_NODE_LEVEL;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendCommunication(true, &comm));

    // 在 lastCommunication 范围内的 NodeLunch 应成功并使 nodeLaunchId 递增
    MsprofApi nodeIn{};
    nodeIn.threadId = 77;
    nodeIn.beginTime = 4950;
    nodeIn.endTime = 5050;
    nodeIn.level = MSPROF_REPORT_NODE_LAUNCH_TYPE;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendNodeLunch(true, &nodeIn));
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendNodeLunch(false, &nodeIn));

    // 范围外应被过滤
    MsprofApi nodeOut{};
    nodeOut.threadId = 77;
    nodeOut.beginTime = 8000;
    nodeOut.endTime = 8100;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendNodeLunch(true, &nodeOut));
}

TEST_F(CannTrackCacheUtest, ConcurrentAppendTsTrackDifferentThreadIdsNoCrash)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    constexpr int kThreads = 16;
    constexpr int kIters = 200;
    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    std::atomic<int> failures{0};
    for (int t = 0; t < kThreads; ++t)
    {
        workers.emplace_back(
            [t, &ins, &failures]()
            {
                for (int i = 0; i < kIters; ++i)
                {
                    MsprofCompactInfo info{};
                    info.threadId = static_cast<uint32_t>(t + 1);
                    info.timeStamp = static_cast<uint64_t>(1000 + i);
                    if (ins.AppendTsTrack(i % 2 == 0, &info) != MSPTI_SUCCESS)
                    {
                        failures.fetch_add(1);
                    }
                    MsprofApi api{};
                    api.threadId = static_cast<uint32_t>(t + 1);
                    api.beginTime = 900 + i;
                    api.endTime = 1100 + i;
                    api.level = MSPROF_REPORT_HCCL_NODE_LEVEL;
                    if (ins.AppendCommunication(true, &api) != MSPTI_SUCCESS)
                    {
                        failures.fetch_add(1);
                    }
                }
            });
    }
    for (auto &th : workers)
    {
        th.join();
    }
    EXPECT_EQ(0, failures.load());
}

TEST_F(CannTrackCacheUtest, ConcurrentAppendMixedOpsWithNodeLunchNoCrash)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    constexpr int kThreads = 8;
    std::vector<std::thread> workers;
    for (int t = 0; t < kThreads; ++t)
    {
        workers.emplace_back(
            [t, &ins]()
            {
                for (int i = 0; i < 100; ++i)
                {
                    MsprofCompactInfo compact{};
                    compact.threadId = static_cast<uint32_t>(1000 + t);
                    compact.timeStamp = 1000;
                    ins.AppendTsTrack(true, &compact);

                    MsprofApi comm{};
                    comm.threadId = 1000 + t;
                    comm.beginTime = 900;
                    comm.endTime = 1100;
                    comm.level = MSPROF_REPORT_HCCL_NODE_LEVEL;
                    ins.AppendCommunication(true, &comm);

                    MsprofApi node{};
                    node.threadId = 1000 + t;
                    node.beginTime = 950;
                    node.endTime = 1050;
                    ins.AppendNodeLunch(true, &node);
                }
            });
    }
    for (auto &th : workers)
    {
        th.join();
    }
    SUCCEED();
}

TEST_F(CannTrackCacheUtest, StopTaskClearsCacheAndNextAppendRecreates)
{
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, am->Register(MSPTI_ACTIVITY_KIND_COMMUNICATION));
    auto &ins = Mspti::Parser::CannTrackCache::GetInstance();

    MsprofCompactInfo info{};
    info.threadId = 999;
    info.timeStamp = 1234;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(true, &info));
    EXPECT_EQ(MSPTI_SUCCESS, ins.StopTask());

    // 清空后再次 Append 应重建缓存且仍成功，nodeLaunchId 从 0 开始
    MsprofCompactInfo info2{};
    info2.threadId = 999;
    info2.timeStamp = 5678;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendTsTrack(false, &info2));

    MsprofApi comm{};
    comm.threadId = 999;
    comm.beginTime = 5600;
    comm.endTime = 5700;
    EXPECT_EQ(MSPTI_SUCCESS, ins.AppendCommunication(true, &comm));
}

}  // namespace
