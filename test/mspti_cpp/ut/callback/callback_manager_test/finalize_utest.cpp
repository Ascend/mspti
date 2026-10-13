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
#include <array>
#include <atomic>
#include <thread>

#include "csrc/callback/callback_manager.h"
#include "gtest/gtest.h"
#include "mspti.h"

using Mspti::Callback::CallbackManager;

namespace
{
constexpr msptiCallbackId FINALIZE_TEST_CBID = MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX;
std::atomic<int32_t> gFinalizeEnterCount{0};
std::atomic<int32_t> gFinalizeExitCount{0};

void CountingCallback(void *pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                      const msptiCallbackData *pCallbackInfo)
{
    (void)pUserData;
    (void)domain;
    (void)callbackId;
    if (pCallbackInfo->callbackSite == MSPTI_API_ENTER)
    {
        gFinalizeEnterCount++;
    }
    else if (pCallbackInfo->callbackSite == MSPTI_API_EXIT)
    {
        gFinalizeExitCount++;
    }
}

// Finalize from the EXIT site. The outer EXIT dispatch
// is dropped: the subscriber is already cleared.
void FinalizingCallback(void *pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                        const msptiCallbackData *pCallbackInfo)
{
    (void)pUserData;
    (void)domain;
    (void)callbackId;
    if (pCallbackInfo->callbackSite == MSPTI_API_ENTER)
    {
        gFinalizeEnterCount++;
        return;
    }
    gFinalizeExitCount++;
    (void)msptiFinalize();
}

void FireOneCallback()
{
    CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID, MSPTI_API_ENTER,
                                                    "TestFunc");
    CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID, MSPTI_API_EXIT,
                                                    "TestFunc");
}

}  // namespace

class FinalizeUtest : public testing::Test
{
   protected:
    void SetUp() override
    {
        gFinalizeEnterCount.store(0);
        gFinalizeExitCount.store(0);
        // Idempotent cleanup: a prior test may have left a session behind.
        if (CallbackManager::GetInstance()->IsTracingSessionRunning())
        {
            (void)msptiFinalize();
        }
    }

    void TearDown() override
    {
        if (CallbackManager::GetInstance()->IsTracingSessionRunning())
        {
            (void)msptiFinalize();
        }
    }
};

TEST_F(FinalizeUtest, FinalizeWhenIdleSucceedsAndIsIdempotent)
{
    uint8_t isRunning = 1;
    EXPECT_EQ(MSPTI_SUCCESS, msptiFinalize());
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);
    // Second call is a no-op success.
    EXPECT_EQ(MSPTI_SUCCESS, msptiFinalize());
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);
}

TEST_F(FinalizeUtest, FinalizeTearsDownSubscriberAndAllowsNewSession)
{
    uint8_t isRunning = 1;
    msptiSubscriberHandle subscriber = nullptr;
    ASSERT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, CountingCallback, nullptr));
    ASSERT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID));

    FireOneCallback();
    EXPECT_EQ(1, gFinalizeEnterCount.load());
    EXPECT_EQ(1, gFinalizeExitCount.load());

    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(1, isRunning);

    EXPECT_EQ(MSPTI_SUCCESS, msptiFinalize());
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);

    FireOneCallback();
    EXPECT_EQ(1, gFinalizeEnterCount.load());
    EXPECT_EQ(1, gFinalizeExitCount.load());

    // A new session starts cleanly; callback enable state belongs to the
    // session, so it must be re-enabled after re-subscribing.
    msptiSubscriberHandle reSubscriber = nullptr;
    ASSERT_EQ(MSPTI_SUCCESS, msptiSubscribe(&reSubscriber, CountingCallback, nullptr));
    ASSERT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, reSubscriber, MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID));
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(1, isRunning);
    FireOneCallback();
    EXPECT_EQ(2, gFinalizeEnterCount.load());
    EXPECT_EQ(2, gFinalizeExitCount.load());
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(reSubscriber));
}

TEST_F(FinalizeUtest, CallbackScopePassesThroughAfterFinalize)
{
    msptiSubscriberHandle subscriber = nullptr;
    ASSERT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, CountingCallback, nullptr));
    ASSERT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID));

    {
        Mspti::Callback::CallbackScope scope(MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID, "TestFunc");
    }
    EXPECT_EQ(1, gFinalizeEnterCount.load());
    EXPECT_EQ(1, gFinalizeExitCount.load());

    ASSERT_EQ(MSPTI_SUCCESS, msptiFinalize());

    {
        // Post-finalize dispatches are dropped (pure pass-through).
        Mspti::Callback::CallbackScope scope(MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID, "TestFunc");
    }
    EXPECT_EQ(1, gFinalizeEnterCount.load());
    EXPECT_EQ(1, gFinalizeExitCount.load());
}

TEST_F(FinalizeUtest, FinalizeFromExitCallbackSiteSucceeds)
{
    msptiSubscriberHandle subscriber = nullptr;
    ASSERT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, FinalizingCallback, nullptr));
    ASSERT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID));

    {
        // Finalize runs inside EXIT; the trailing dispatch is dropped.
        Mspti::Callback::CallbackScope scope(MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID, "TestFunc");
    }
    EXPECT_EQ(1, gFinalizeEnterCount.load());
    EXPECT_EQ(1, gFinalizeExitCount.load());

    uint8_t isRunning = 1;
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);
}

TEST_F(FinalizeUtest, ConcurrentFinalizeCallsAllSucceed)
{
    constexpr int THREAD_NUM = 4;
    std::atomic<int> successCount{0};
    std::array<std::thread, THREAD_NUM> threads;
    for (auto &thread : threads)
    {
        thread = std::thread(
            [&successCount]()
            {
                if (msptiFinalize() == MSPTI_SUCCESS)
                {
                    successCount++;
                }
            });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    EXPECT_EQ(THREAD_NUM, successCount.load());
    uint8_t isRunning = 1;
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);
}

// Concurrent Finalize against a live session: all calls must succeed and
// fully tear the session down.
TEST_F(FinalizeUtest, ConcurrentFinalizeWithActiveSessionAllSucceed)
{
    constexpr int THREAD_NUM = 4;
    std::atomic<int> successCount{0};
    msptiSubscriberHandle subscriber = nullptr;
    ASSERT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, CountingCallback, nullptr));
    ASSERT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, FINALIZE_TEST_CBID));

    std::array<std::thread, THREAD_NUM> threads;
    for (auto &thread : threads)
    {
        thread = std::thread(
            [&successCount]()
            {
                if (msptiFinalize() == MSPTI_SUCCESS)
                {
                    successCount++;
                }
            });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }
    EXPECT_EQ(THREAD_NUM, successCount.load());
    uint8_t isRunning = 1;
    EXPECT_EQ(MSPTI_SUCCESS, msptiIsTracingSessionRunning(&isRunning));
    EXPECT_EQ(0, isRunning);

    // The session is fully gone: further dispatches are pure pass-through.
    FireOneCallback();
    EXPECT_EQ(0, gFinalizeEnterCount.load());
    EXPECT_EQ(0, gFinalizeExitCount.load());
}
