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

#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/inject/profapi_inject.h"
#include "mspti.h"

namespace {
class DevTaskManager : public testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(DevTaskManager, DevProfTaskShouldRunSuccessfullyWhenUseDevTaskManagerNormal)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels)
        .stubs()
        .will(returnValue(MSPTI_SUCCESS));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance -> StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    ret = instance -> StopDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetErrorWhenDeviceOffline)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline)
        .stubs()
        .will(returnValue(false));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance -> StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);

    ret = instance -> StopDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetErrorWhenGetChannelsError)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels)
        .stubs()
        .will(returnValue(MSPTI_ERROR_INNER));
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    auto ret = instance -> StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetErrorWhenStartOrStopCannProfTaskFailed)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline)
        .stubs()
        .will(returnValue(true));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels)
        .stubs()
        .will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand)
        .stubs()
        .will(returnValue(static_cast<int32_t>(MSPTI_ERROR_INNER)));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance -> StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);

    ret = instance -> StopDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetSuccessWhenCannProfNotSupport)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand)
        .stubs()
        .will(returnValue(static_cast<int32_t>(MSPTI_SUCCESS)));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_MARKER] = true;
    auto ret = instance -> StartCannProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    ret = instance -> StopCannProfTask(deviceId);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}
}