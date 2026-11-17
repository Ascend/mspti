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
#include <cstdint>
#include <thread>
#include <vector>

#include "csrc/common/plog_manager.h"
#include "gtest/gtest.h"

namespace
{
using Mspti::Common::PlogManager;

class PlogManagerUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

// *_ONCE：首次放行且并发下恰好一个调用者放行
TEST_F(PlogManagerUtest, ShouldLogOnceReturnsTrueOnlyForFirstCall)
{
    std::atomic<bool> flag{false};
    EXPECT_TRUE(PlogManager::ShouldLogOnce(flag));
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_FALSE(PlogManager::ShouldLogOnce(flag));
    }

    constexpr int threadNum = 8;
    constexpr int loopNum = 1000;
    std::atomic<bool> sharedFlag{false};
    std::atomic<int> hitCount{0};
    std::vector<std::thread> threads;
    threads.reserve(threadNum);
    for (int t = 0; t < threadNum; ++t)
    {
        threads.emplace_back(
            [&sharedFlag, &hitCount, loopNum]()
            {
                for (int i = 0; i < loopNum; ++i)
                {
                    if (PlogManager::ShouldLogOnce(sharedFlag))
                    {
                        hitCount.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    EXPECT_EQ(1, hitCount.load(std::memory_order_relaxed));
}

// *_EVERY_N：n==0 恒放行，n==3 首次放行后每 3 次一次
TEST_F(PlogManagerUtest, ShouldLogEveryNFollowsFirstThenInterval)
{
    std::atomic<uint64_t> zeroCounter{0};
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_TRUE(PlogManager::ShouldLogEveryN(zeroCounter, 0));
    }
    EXPECT_EQ(0U, zeroCounter.load(std::memory_order_relaxed));

    std::atomic<uint64_t> counter{0};
    const bool expected[] = {true, false, false, true, false, false, true, false, false, true};
    for (bool shouldLog : expected)
    {
        EXPECT_EQ(shouldLog, PlogManager::ShouldLogEveryN(counter, 3));
    }
    EXPECT_EQ(10U, counter.load(std::memory_order_relaxed));
}

// *_TOTAL：首次与跨窗时返回累计值，其余返回 0
TEST_F(PlogManagerUtest, LogEveryNTotalLogsFirstEventThenOnBoundary)
{
    std::atomic<uint64_t> counter{0};
    std::vector<uint64_t> logged;
    for (int i = 0; i < 20; ++i)
    {
        const uint64_t total = PlogManager::LogEveryNTotal(counter, 5);
        if (total != 0)
        {
            logged.push_back(total);
        }
    }
    EXPECT_EQ((std::vector<uint64_t>{1, 5, 10, 15, 20}), logged);
    EXPECT_EQ(20U, counter.load(std::memory_order_relaxed));
}

// *_TOTAL_INC：increment>1 时按窗口边界采样
TEST_F(PlogManagerUtest, LogEveryNTotalWithIncrementCrossesBoundaries)
{
    std::atomic<uint64_t> counter{0};
    std::vector<uint64_t> logged;
    for (int i = 0; i < 10; ++i)
    {
        const uint64_t total = PlogManager::LogEveryNTotal(counter, 10, 3);
        if (total != 0)
        {
            logged.push_back(total);
        }
    }
    EXPECT_EQ((std::vector<uint64_t>{3, 12, 21, 30}), logged);
    EXPECT_EQ(30U, counter.load(std::memory_order_relaxed));
}
}  // namespace
