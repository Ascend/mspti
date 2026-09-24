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
#include <cstdlib>

#include "csrc/activity/activity_manager.h"
#include "csrc/callback/callback_manager.h"
#include "gtest/gtest.h"
#include "mspti.h"

class CallbackRuntimeLaunchUtest : public testing::Test
{
   protected:
    virtual void SetUp() { setenv("LD_PRELOAD", "libmspti.so", 1); }
    virtual void TearDown() {}
};

static void UserCallback(void *pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                         const msptiCallbackData *pCallbackInfo)
{
    if (pCallbackInfo->callbackSite == MSPTI_API_ENTER)
    {
        printf("Enter: %s\n", pCallbackInfo->functionName);
    }
    else if (pCallbackInfo->callbackSite == MSPTI_API_EXIT)
    {
        printf("Exit: %s\n", pCallbackInfo->functionName);
    }
    if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_LAUNCH)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_LAUNCH\n");
    }
    else if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_AICPU_LAUNCH)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_AICPU_LAUNCH\n");
    }
    else if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_AIV_LAUNCH)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_AIV_LAUNCH\n");
    }
    else if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_FFTS_LAUNCH)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_FFTS_LAUNCH\n");
    }
    else if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_CPU_LAUNCH)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_CPU_LAUNCH\n");
    }
}

TEST_F(CallbackRuntimeLaunchUtest, CallbackExternalApiTestWithRuntimeDomain)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AICPU_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AIV_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FFTS_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CPU_LAUNCH));

    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_ENTER, "rtLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_EXIT, "rtLaunch");

    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AICPU_LAUNCH, MSPTI_API_ENTER, "rtAiCpuLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AICPU_LAUNCH, MSPTI_API_EXIT, "rtAiCpuLaunch");

    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AIV_LAUNCH, MSPTI_API_ENTER, "rtAivLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AIV_LAUNCH, MSPTI_API_EXIT, "rtAivLaunch");

    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FFTS_LAUNCH, MSPTI_API_ENTER, "rtFftsLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FFTS_LAUNCH, MSPTI_API_EXIT, "rtFftsLaunch");

    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CPU_LAUNCH, MSPTI_API_ENTER, "rtCpuLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CPU_LAUNCH, MSPTI_API_EXIT, "rtCpuLaunch");

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AICPU_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_AIV_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_FFTS_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CPU_LAUNCH));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
    msptiSubscriberHandle subscriberNul{nullptr};
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriberNul, nullptr, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriberNul));
}
