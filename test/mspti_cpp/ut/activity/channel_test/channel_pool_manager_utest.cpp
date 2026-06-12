/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
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
#include "csrc/common/inject/driver_inject.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti_result.h"

namespace
{
class ChannelPoolManagerUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
    virtual void TearDown() {}
};

TEST_F(ChannelPoolManagerUtest, InitWillNotReturnSuccWhenDrvGetDevNumReturnError)
{
    MOCKER_CPP(&DrvGetDevNum).stubs().will(returnValue(DRV_ERROR_NO_DEVICE));

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_ERROR_DEVICE_OFFLINE, ret);
}

TEST_F(ChannelPoolManagerUtest, InitWillNotReturnSuccWhenGetZeroDevNum)
{
    uint32_t devNum = 0;
    MOCKER_CPP(&DrvGetDevNum).stubs().with(outBoundP(&devNum)).will(returnValue(DRV_ERROR_NONE));

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_ERROR_DEVICE_OFFLINE, ret);
}

TEST_F(ChannelPoolManagerUtest, InitWillReturnSuccWhenGetDevNumLargerThanMaxDevNum)
{
    uint32_t devNum = 65;
    MOCKER_CPP(&DrvGetDevNum).stubs().with(outBoundP(&devNum)).will(returnValue(DRV_ERROR_NONE));

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}

TEST_F(ChannelPoolManagerUtest, InitWillReturnSuccWhenGetDevNumIsValid)
{
    uint32_t devNum = 1;
    MOCKER_CPP(&DrvGetDevNum).stubs().with(outBoundP(&devNum)).will(returnValue(DRV_ERROR_NONE));

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    // repeat init
    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}

TEST_F(ChannelPoolManagerUtest, GetAllChannelsWillReturnInnerErrorWhenGetInvalidChannels)
{
    ChannelListT channelList;
    channelList.chipType = 0;
    channelList.channelNum = 0;
    MOCKER_CPP(&ProfDrvGetChannels).stubs().with(any(), outBoundP(&channelList)).will(returnValue(0));
    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(0);
    EXPECT_EQ(MSPTI_ERROR_INNER, ret);
}

TEST_F(ChannelPoolManagerUtest, GetAllChannelsWillReturnSuccWhenGetValidChannels)
{
    ChannelListT channelList;
    channelList.chipType = 0;
    channelList.channelNum = 1;
    MOCKER_CPP(&ProfDrvGetChannels).stubs().with(any(), outBoundP(&channelList)).will(returnValue(0));
    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(0);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->channels_.clear();
}

TEST_F(ChannelPoolManagerUtest, CheckChannelValidWillReturnFalseWhenInputInvalidChannel)
{
    ChannelListT channelList;
    channelList.chipType = 0;
    channelList.channelNum = 1;
    channelList.channel[0].channelId = PROF_CHANNEL_TS_FW;
    MOCKER_CPP(&ProfDrvGetChannels).stubs().with(any(), outBoundP(&channelList)).will(returnValue(0));
    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(0);
    ASSERT_EQ(MSPTI_SUCCESS, ret);

    EXPECT_FALSE(Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(1, 44));
    EXPECT_FALSE(Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(0, 50));
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->channels_.clear();
}

TEST_F(ChannelPoolManagerUtest, CheckChannelValidWillReturnTrueWhenInputValidChannel)
{
    ChannelListT channelList;
    channelList.chipType = 0;
    channelList.channelNum = 1;
    channelList.channel[0].channelId = PROF_CHANNEL_TS_FW;
    MOCKER_CPP(&ProfDrvGetChannels).stubs().with(any(), outBoundP(&channelList)).will(returnValue(0));
    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->GetAllChannels(0);
    ASSERT_EQ(MSPTI_SUCCESS, ret);

    EXPECT_TRUE(Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->CheckChannelValid(0, 44));

    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->channels_.clear();
}

TEST_F(ChannelPoolManagerUtest, AddReaderAndRemoveReaderWillReturnSuccWhenInputDevIdAndChannelId)
{
    // clean drvChannelPoll_
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->AddReader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    uint32_t devNum = 1;
    MOCKER_CPP(&DrvGetDevNum).stubs().with(outBoundP(&devNum)).will(returnValue(DRV_ERROR_NONE));

    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    ASSERT_EQ(MSPTI_SUCCESS, ret);

    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->AddReader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
    EXPECT_EQ(1, Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->drvChannelPoll_->readers_map_.size());
    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->RemoveReader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
    EXPECT_EQ(0, Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->drvChannelPoll_->readers_map_.size());
}

TEST_F(ChannelPoolManagerUtest, FlushDrvBuffWillReturnSuccWhenChannelPoolFlushSucc)
{
    // clean drvChannelPoll_
    Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->UnInit();

    msptiResult ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);

    uint32_t devNum = 1;
    MOCKER_CPP(&DrvGetDevNum).stubs().with(outBoundP(&devNum)).will(returnValue(DRV_ERROR_NONE));
    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelPool::FlushDrvBuff).stubs().will(returnValue(MSPTI_SUCCESS));
    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->Init();
    ASSERT_EQ(MSPTI_SUCCESS, ret);

    ret = Mspti::Ascend::Channel::ChannelPoolManager::GetInstance()->FlushDrvBuff(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, ret);
}
}  // namespace
