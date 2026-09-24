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

#include <cstdio>
#include <cstdlib>

#include "csrc/activity/activity_manager.h"
#include "gtest/gtest.h"
#include "mspti.h"

class CallbackStopChainUtest : public testing::Test
{
   protected:
    void SetUp() override
    {
        setenv("LD_PRELOAD", "libmspti.so", 1);
        subscriber_ = nullptr;
    }

    void TearDown() override
    {
        // Best-effort cleanup for cases where the test fails in the middle.
        if (subscriber_ != nullptr)
        {
            (void)msptiUnsubscribe(subscriber_);
            subscriber_ = nullptr;
        }
        (void)msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL);
    }

    static void UserCallback(void *pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                             const msptiCallbackData *pCallbackInfo)
    {
        (void)pUserData;
        (void)domain;
        (void)callbackId;
        (void)pCallbackInfo;
    }

    static void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
    {
        constexpr size_t kBufferSize = 1024;
        *buffer = static_cast<uint8_t *>(malloc(kBufferSize));
        *size = kBufferSize;
        *maxNumRecords = 0;
    }

    static void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
    {
        (void)size;
        (void)validSize;
        free(buffer);
    }

    msptiSubscriberHandle subscriber_;
};

TEST_F(CallbackStopChainUtest, StopSequenceShouldDisableKernelAndAllowResubscribe)
{
    // This test is intentionally lightweight.
    // It exercises the public stop-chain used by Huawei doStop():
    // 1) subscribe and enable runtime callback delivery,
    // 2) enable kernel activity collection,
    // 3) disable kernel activity collection,
    // 4) unsubscribe from callback delivery,
    // 5) verify that the callback layer can be initialized again.
    //
    // The goal is to cover the current public contract without introducing
    // additional test seams or deep internal mocks.

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete));

    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    // Verify that duplicate subscription is rejected before stop-chain cleanup.
    // This makes the later re-subscribe check more meaningful.
    msptiSubscriberHandle anotherSubscriber = nullptr;
    EXPECT_EQ(MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED,
              msptiSubscribe(&anotherSubscriber, UserCallback, nullptr));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber_, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(true, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // This is the first half of the Huawei stop sequence.
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // Repeated disable is intentionally tolerated here.
    // The current MSPTI implementation resets the activity flag and stops analysis work,
    // so a second disable should not break the rest of the stop-chain.
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // This is the second half of the Huawei stop sequence.
    // In the current MSPTI design, unsubscribe is also responsible for deeper cleanup.
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;

    // Re-subscribe is used here as a lightweight proof that callback-side teardown completed.
    // If cleanup is incomplete, this step is a likely place to fail with
    // MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED or equivalent state leakage.
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    // Verify that callback configuration can be enabled again after stop-chain cleanup.
    // This is stronger than checking subscribe alone because it shows that the callback
    // layer is not only re-opened, but also usable.
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber_, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;
}

TEST_F(CallbackStopChainUtest, ActivityDisableAloneShouldNotBeTreatedAsFullStop)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete));

    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber_, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // activityDisable alone is not expected to perform full callback-side cleanup.
    msptiSubscriberHandle anotherSubscriber = nullptr;
    EXPECT_EQ(MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED,
              msptiSubscribe(&anotherSubscriber, UserCallback, nullptr));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;

    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;
}

TEST_F(CallbackStopChainUtest, StopSequenceShouldSupportRestartAndSecondStop)
{
    // This test extends the minimal stop-chain scenario into a full restart cycle.
    // The goal is to verify that the public MSPTI lifecycle remains usable after
    // a complete stop sequence:
    //   start -> stop -> restart -> stop
    //
    // This is still a public-contract test.
    // It does not validate internal device reset calls directly, but it increases
    // confidence that unsubscribe-driven cleanup is sufficient for repeated use.

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete));

    // First start cycle.
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber_, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(true, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // First stop cycle.
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;

    // Restart after complete stop.
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber_, UserCallback, nullptr));
    ASSERT_NE(subscriber_, nullptr);

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber_, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(true, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // Second stop cycle.
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber_));
    subscriber_ = nullptr;
}
