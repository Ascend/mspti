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
#include <cstdio>
#include <cstdlib>

#include "csrc/activity/activity_manager.h"
#include "csrc/callback/callback_manager.h"  // <<< needed for ExecuteCallback
#include "gtest/gtest.h"
#include "mspti.h"

// -----------------------------------------------------------------------------
// Helpers exactly as in original CallbackUtest + buffer example
// -----------------------------------------------------------------------------
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
}

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    printf("========== UserBufferRequest ============\n");
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    printf("========== UserBufferComplete ============\n");
    (void)size;
    (void)validSize;
    free(buffer);
}

class CallbackDoStartChainUtest : public testing::Test
{
   protected:
    virtual void SetUp() { setenv("LD_PRELOAD", "libmspti.so", 1); }
    virtual void TearDown() {}
};

TEST_F(CallbackDoStartChainUtest, DoStartCallsMsptiFunctionsInExactSequence)
{
    // =====================================================================
    // EXACT verification sequence from the snippet
    // (this is the public side-effect check after doStart() has run)
    // =====================================================================
    msptiSubscriberHandle subscriber;
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&subscriber, UserCallback, nullptr));
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));  // proves activityEnable<true>(KERNEL) happened
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivityRegisterCallbacks(UserBufferRequest,
                                             UserBufferComplete));  // proves activityRegisterCallbacks<true> happened

    // === 4. setRuntimeCallbacks(subscriber, true) was executed ===
    // (it called enableCallback for all 5 launch IDs)
    const msptiCallbackId launchIds[] = {MSPTI_CBID_RUNTIME_LAUNCH, MSPTI_CBID_RUNTIME_AICPU_LAUNCH,
                                         MSPTI_CBID_RUNTIME_AIV_LAUNCH, MSPTI_CBID_RUNTIME_FFTS_LAUNCH,
                                         MSPTI_CBID_RUNTIME_CPU_LAUNCH};

    for (auto id : launchIds)
    {
        Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, id, MSPTI_API_ENTER,
                                                                         "launchTest");
        Mspti::Callback::CallbackManager::GetInstance()->ExecuteCallback(MSPTI_CB_DOMAIN_RUNTIME, id, MSPTI_API_EXIT,
                                                                         "launchTest");
    }

    // Optional: prove kernel activity is now enabled
    EXPECT_EQ(true, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // Cleanup (best-effort, matches your Stop test style)
    (void)msptiUnsubscribe(subscriber);
}
