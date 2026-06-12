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

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/inject/profapi_inject.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti.h"

namespace
{
class DevProfTaskUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(DevProfTaskUtest, CreateTasksShouldReturnRightProfTaskNumsWhenSetDifferentKind)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    constexpr uint32_t deviceId = 0;
    msptiActivityKind kind = MSPTI_ACTIVITY_KIND_MARKER;
    auto profTasks = Mspti::Ascend::DevProfTaskFactory::CreateTasks(deviceId, kind);
    constexpr size_t MARKER_PROF_TASK_NUM = 1;
    EXPECT_EQ(MARKER_PROF_TASK_NUM, profTasks.size());
    kind = MSPTI_ACTIVITY_KIND_KERNEL;
    constexpr size_t KERNEL_PROF_TASK_NUM = 1;
    profTasks = Mspti::Ascend::DevProfTaskFactory::CreateTasks(deviceId, kind);
    EXPECT_EQ(KERNEL_PROF_TASK_NUM, profTasks.size());
}

TEST_F(DevProfTaskUtest, CreateTasksShouldGetZeroProfTaskNumsWhenChipTypeIsInvalid)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::END_TYPE));
    constexpr uint32_t deviceId = 0;
    msptiActivityKind kind = MSPTI_ACTIVITY_KIND_MARKER;
    auto profTasks = Mspti::Ascend::DevProfTaskFactory::CreateTasks(deviceId, kind);
    constexpr size_t ZERO_PROF_TASK_NUM = 0;
    EXPECT_EQ(ZERO_PROF_TASK_NUM, profTasks.size());
}

TEST_F(DevProfTaskUtest, CreateTasksShouldGetZeroProfTaskNumsWhenKindIsInvalid)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));
    constexpr uint32_t deviceId = 0;
    msptiActivityKind kind = MSPTI_ACTIVITY_KIND_COUNT;
    auto profTasks = Mspti::Ascend::DevProfTaskFactory::CreateTasks(deviceId, kind);
    constexpr size_t ZERO_PROF_TASK_NUM = 0;
    EXPECT_EQ(ZERO_PROF_TASK_NUM, profTasks.size());
}

TEST_F(DevProfTaskUtest, DevProfTaskDefaultShouldRunSuccessfullyWhenRunNormally)
{
    std::shared_ptr<Mspti::Ascend::DevProfTaskDefault> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());
}

TEST_F(DevProfTaskUtest, DevProfTaskStarsShouldRunSuccessfullyWhenRunNormally)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskStars> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    // repeat start task for same device
    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    // repeat stop task for same device
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}

TEST_F(DevProfTaskUtest, DevProfTaskStarsShouldReturnSuccessWhenCheckChannelValidFail)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskStars> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(false));

    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    // clear ref_cnts
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());
}

TEST_F(DevProfTaskUtest, DevProfTaskStarsShouldReturnInnerErrorWhenDrvStartChannelFail)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskStars> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();

    // set drv start channel fail
    MOCKER_CPP(&ProfDrvStart).stubs().will(returnValue(-1));
    EXPECT_EQ(MSPTI_ERROR_INNER, task->StartTask());

    // clear ref_cnts
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}

TEST_F(DevProfTaskUtest, CanFlushShoulReturnFalseWhenNotStartDeviceChannelEarly)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskStars> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    EXPECT_FALSE(task->CanFlush());
}

TEST_F(DevProfTaskUtest, CanFlushShoulReturnTrueWhenStartDeviceChannelEarly)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskStars> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    EXPECT_TRUE(task->CanFlush());

    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}

TEST_F(DevProfTaskUtest, DevProfTaskTsFwShouldRunSuccessfullyWhenRunNormally)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskTsFw> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    // repeat start task for same device
    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    // repeat stop task for same device
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}

TEST_F(DevProfTaskUtest, DevProfTaskTsFwShouldReturnSuccessWhenCheckChannelValidFail)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskTsFw> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(false));

    EXPECT_EQ(MSPTI_SUCCESS, task->StartTask());

    // clear ref_cnts
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());
}

TEST_F(DevProfTaskUtest, DevProfTaskTsFwShouldReturnInnerErrorWhenDrvStartChannelFail)
{
    GlobalMockObject::verify();
    std::shared_ptr<Mspti::Ascend::DevProfTaskTsFw> task;
    Mspti::Common::MsptiMakeSharedPtr(task, 0);
    ASSERT_NE(task, nullptr);

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();

    // set drv start channel fail
    MOCKER_CPP(&ProfDrvStart).stubs().will(returnValue(-1));
    EXPECT_EQ(MSPTI_ERROR_INNER, task->StartTask());

    // clear ref_cnts
    EXPECT_EQ(MSPTI_SUCCESS, task->StopTask());

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}
}  // namespace
