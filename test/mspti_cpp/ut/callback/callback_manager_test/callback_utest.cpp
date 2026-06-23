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
#include <cstdlib>

#include "csrc/callback/callback_manager.h"
#include "gtest/gtest.h"
#include "mspti.h"

class CallbackUtest : public testing::Test
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
    if (domain == MSPTI_CB_DOMAIN_RUNTIME && callbackId == MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX)
    {
        printf("Domain MSPTI_CB_DOMAIN_RUNTIME, CallbackID MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX\n");
    }
}

TEST_F(CallbackUtest, CallbackExternalApiTestWithRuntimeDomain)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX, MSPTI_API_ENTER, "rtCtxCreateEx");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX, MSPTI_API_EXIT, "rtCtxCreateEx");
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX, MSPTI_API_ENTER, "rtCtxCreateEx");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_CONTEXT_CREATED_EX, MSPTI_API_EXIT, "rtCtxCreateEx");
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
    msptiSubscriberHandle subscriberNul{nullptr};
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriberNul, nullptr, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriberNul));
}

TEST_F(CallbackUtest, SubscribeWithNullSubscriberHandleReturnsInvalidParam)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiSubscribe(nullptr, UserCallback, nullptr));
}

TEST_F(CallbackUtest, UnsubscribeWithoutInitIsSuccess)
{
    msptiSubscriberHandle dummy = reinterpret_cast<msptiSubscriberHandle>(0x1);
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(dummy));
}

TEST_F(CallbackUtest, UnsubscribeWithMismatchedSubscriberReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    msptiSubscriberHandle wrong = reinterpret_cast<msptiSubscriberHandle>(0x1);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiUnsubscribe(wrong));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, EnableCallbackWithInvalidDomainReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_INVALID, MSPTI_CBID_RUNTIME_LAUNCH));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_SIZE, MSPTI_CBID_RUNTIME_LAUNCH));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, EnableCallbackWithInvalidCbidReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, 1024));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, EnableCallbackWithMismatchedSubscriberReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    msptiSubscriberHandle wrong = reinterpret_cast<msptiSubscriberHandle>(0x1);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiEnableCallback(1, wrong, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, EnableCallbackWithoutInitIsSuccess)
{
    msptiSubscriberHandle subscriber = reinterpret_cast<msptiSubscriberHandle>(0x1);
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME));
}

TEST_F(CallbackUtest, EnableDomainWithInvalidDomainReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_INVALID));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_SIZE));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, EnableDomainWithMismatchedSubscriberReturnsInvalidParam)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    msptiSubscriberHandle wrong = reinterpret_cast<msptiSubscriberHandle>(0x1);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiEnableDomain(1, wrong, MSPTI_CB_DOMAIN_RUNTIME));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, ExecuteCallbackBeforeInitDoesNotInvokeCallback)
{
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_ENTER, "rtLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_EXIT, "rtLaunch");
}

TEST_F(CallbackUtest, ExecuteCallbackWithInvalidDomainOrCbidIsNoop)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_INVALID, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_ENTER, "noop");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, 1024, MSPTI_API_ENTER,
                                                                     "noop");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_SIZE, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_ENTER, "noop");
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, ExecuteCallbackWithDisabledCbidDoesNotInvokeCallback)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_ENTER, "rtLaunch");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH,
                                                                     MSPTI_API_EXIT, "rtLaunch");
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, CallbackScopeInvokesEnterAndExit)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));
    {
        Mspti::Callback::CallbackScope scope(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH, "rtLaunch");
    }
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_LAUNCH));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, CallbackExternalApiTestWithHcclDomain)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE,
                                                                     MSPTI_API_ENTER, "HcclAllReduce");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE,
                                                                     MSPTI_API_EXIT, "HcclAllReduce");
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE));

    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_HCCL));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_BROADCAST,
                                                                     MSPTI_API_ENTER, "HcclBroadcast");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_BROADCAST,
                                                                     MSPTI_API_EXIT, "HcclBroadcast");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_SENDRECV,
                                                                     MSPTI_API_ENTER, "HcclSendRecv");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_SENDRECV,
                                                                     MSPTI_API_EXIT, "HcclSendRecv");
    EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(0, subscriber, MSPTI_CB_DOMAIN_HCCL));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, MultipleRuntimeCbidsCanBeEnabledAndDisabledIndependently)
{
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_CREATED));
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_DESTROY));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_CREATED, MSPTI_API_ENTER, "rtStreamCreate");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_DESTROY, MSPTI_API_ENTER, "rtStreamDestroy");
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_CREATED));
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_CREATED, MSPTI_API_EXIT, "rtStreamCreate");
    Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
        MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_DESTROY, MSPTI_API_EXIT, "rtStreamDestroy");
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiEnableCallback(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_STREAM_DESTROY));
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
}

TEST_F(CallbackUtest, SubscribeUnsubscribeSubscribeRepeatedLifecycle)
{
    msptiSubscriberHandle subscriber = nullptr;
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
        ASSERT_NE(subscriber, nullptr);
        EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(1, subscriber, MSPTI_CB_DOMAIN_RUNTIME));
        Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
            MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MALLOC, MSPTI_API_ENTER, "rtMalloc");
        Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(
            MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MALLOC, MSPTI_API_EXIT, "rtMalloc");
        EXPECT_EQ(MSPTI_SUCCESS, msptiEnableDomain(0, subscriber, MSPTI_CB_DOMAIN_RUNTIME));
        EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(subscriber));
        subscriber = nullptr;
    }
}
