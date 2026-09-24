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

#include "csrc/activity/ascend/channel/channel_pool_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/inject/profapi_inject.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti.h"

namespace
{
class DevTaskManager : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(DevTaskManager, DevProfTaskShouldRunSuccessfullyWhenUseDevTaskManagerNormal)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels).stubs().will(returnValue(MSPTI_SUCCESS));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance->StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    ret = instance->StopDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetErrorWhenDeviceOffline)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline).stubs().will(returnValue(false));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance->StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);

    ret = instance->StopDevProfTask(deviceId, kinds);
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
    auto ret = instance->StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetErrorWhenStartOrStopCannProfTaskFailed)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand).stubs().will(returnValue(static_cast<int32_t>(MSPTI_ERROR_INNER)));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;
    auto ret = instance->StartDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);

    ret = instance->StopDevProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(DevTaskManager, DevProfTaskShouldRetSuccessWhenCannProfNotSupport)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand).stubs().will(returnValue(static_cast<int32_t>(MSPTI_SUCCESS)));
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    uint32_t deviceId = 0;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    kinds[MSPTI_ACTIVITY_KIND_MARKER] = true;
    auto ret = instance->StartCANNProfTask(deviceId, kinds);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    ret = instance->StopCANNProfTask(deviceId);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}

TEST_F(DevTaskManager, StartShouldPairWithStopEvenWhenDrvStartFails)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand).stubs().will(returnValue(static_cast<int32_t>(MSPTI_SUCCESS)));
    MOCKER_CPP(&ProfDrvStart).stubs().will(returnValue(-1));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    constexpr uint32_t kDev = 5;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    for (auto& kindSwitch : kinds)
    {
        kindSwitch.store(false);
    }
    kinds[MSPTI_ACTIVITY_KIND_KERNEL] = true;  // KERNEL → 1 × DevProfTaskStars on 910B

    // driver start fails but Start() swallows the error by design: the task is still
    // tracked in task_map_, so the paired Stop() balances refcnt (no leak, retry works).
    EXPECT_EQ(MSPTI_SUCCESS, instance->StartDevProfTask(kDev, kinds));
    Mspti::Ascend::DevProfTaskStars probe(kDev);
    EXPECT_TRUE(probe.CanFlush());
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(
                                 kDev, PROF_CHANNEL_STARS_SOC_LOG));

    EXPECT_EQ(MSPTI_SUCCESS, instance->StopDevProfTask(kDev, kinds));
    EXPECT_FALSE(probe.CanFlush());
    EXPECT_EQ(MSPTI_ERROR_INNER, Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(
                                     kDev, PROF_CHANNEL_STARS_SOC_LOG));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}

TEST_F(DevTaskManager, StartShouldPairWithStopForAllSiblingTasksEvenWhenDrvStartFails)
{
    GlobalMockObject::verify();
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::CheckDeviceOnline).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType)
        .stubs()
        .will(returnValue(Mspti::Common::PlatformType::CHIP_910B));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::GetAllChannels).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPoolManager::CheckChannelValid).stubs().will(returnValue(true));
    MOCKER_CPP(&Mspti::Inject::profSetProfCommand).stubs().will(returnValue(static_cast<int32_t>(MSPTI_SUCCESS)));
    MOCKER_CPP(&ProfDrvStart).stubs().will(returnValue(-1));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    auto instance = Mspti::Ascend::DevTaskManager::GetInstance();
    constexpr uint32_t kDev = 6;
    Mspti::Ascend::DevTaskManager::ActivitySwitchType kinds;
    for (auto& kindSwitch : kinds)
    {
        kindSwitch.store(false);
    }
    kinds[MSPTI_ACTIVITY_KIND_COMMUNICATION] = true;  // COMMUNICATION → TsFw + Stars on 910B

    // both tasks are tracked despite the driver failure, so Stop() balances both refcnts
    EXPECT_EQ(MSPTI_SUCCESS, instance->StartDevProfTask(kDev, kinds));
    Mspti::Ascend::DevProfTaskTsFw tsfwProbe(kDev);
    Mspti::Ascend::DevProfTaskStars starsProbe(kDev);
    EXPECT_TRUE(tsfwProbe.CanFlush());
    EXPECT_TRUE(starsProbe.CanFlush());

    EXPECT_EQ(MSPTI_SUCCESS, instance->StopDevProfTask(kDev, kinds));
    EXPECT_FALSE(tsfwProbe.CanFlush());
    EXPECT_FALSE(starsProbe.CanFlush());
    EXPECT_EQ(MSPTI_ERROR_INNER,
              Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(kDev, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(MSPTI_ERROR_INNER, Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(
                                     kDev, PROF_CHANNEL_STARS_SOC_LOG));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();
}
}  // namespace
