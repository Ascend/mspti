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

#include "csrc/activity/ascend/channel/channel_pool.h"
#include "csrc/common/config.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
class ChannelPoolUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
    virtual void TearDown() {}
};

TEST_F(ChannelPoolUtest, AddReaderWillReturnSuccWhenCreateReaderSucc)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    constexpr uint32_t devId0 = 0;
    constexpr uint32_t devId1 = 1;

    EXPECT_EQ(MSPTI_SUCCESS, pool->AddReader(devId0, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(1, pool->readers_map_.size());

    EXPECT_EQ(MSPTI_SUCCESS, pool->AddReader(devId0, PROF_CHANNEL_STARS_SOC_LOG));
    EXPECT_EQ(2, pool->readers_map_.size());

    EXPECT_EQ(MSPTI_SUCCESS, pool->AddReader(devId1, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(3, pool->readers_map_.size());

    EXPECT_EQ(MSPTI_SUCCESS, pool->AddReader(devId1, PROF_CHANNEL_STARS_SOC_LOG));
    EXPECT_EQ(4, pool->readers_map_.size());

    // repeat add reader for same devId and channelId
    EXPECT_EQ(MSPTI_SUCCESS, pool->AddReader(devId0, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(4, pool->readers_map_.size());
}

TEST_F(ChannelPoolUtest, RemoveReaderWillReturnSuccWhenInputValidDevIdAndChannelId)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    constexpr uint32_t devId0 = 0;
    constexpr uint32_t devId1 = 1;

    EXPECT_EQ(MSPTI_SUCCESS, pool->RemoveReader(devId0, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(0, pool->readers_map_.size());

    ASSERT_EQ(MSPTI_SUCCESS, pool->AddReader(devId0, PROF_CHANNEL_TS_FW));
    ASSERT_EQ(1, pool->readers_map_.size());
    EXPECT_EQ(MSPTI_SUCCESS, pool->RemoveReader(devId0, PROF_CHANNEL_TS_FW));
    EXPECT_EQ(0, pool->readers_map_.size());
}

TEST_F(ChannelPoolUtest, ChannelPoolRunLoopWillBreakWhenProfChannelPollReturnError)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    MOCKER_CPP(&ProfChannelPoll).stubs().will(returnValue(CHANNEL_PROF_ERROR));

    EXPECT_EQ(MSPTI_SUCCESS, pool->Start());

    pool->Stop();
}

TEST_F(ChannelPoolUtest, ChannelPoolRunLoopWillBreakWhenProfChannelPollReturnInvalidValue)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    int invalidVal1 = -1;
    int invalidVal2 = 7;  // 7: CHANNEL_POOL_NUM(6) + 1
    MOCKER_CPP(&ProfChannelPoll).stubs().will(returnValue(invalidVal1)).then(returnValue(invalidVal2));

    EXPECT_EQ(MSPTI_SUCCESS, pool->Start());
    pool->Stop();

    EXPECT_EQ(MSPTI_SUCCESS, pool->Start());
    pool->Stop();
}

TEST_F(ChannelPoolUtest, ChannelPoolRunLoopWillContinueWhenProfChannelPollReturnStopAlready)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    MOCKER_CPP(&ProfChannelPoll).stubs().will(returnValue(CHANNEL_PROF_STOPPED_ALREADY));

    EXPECT_EQ(MSPTI_SUCCESS, pool->Start());

    pool->Stop();
}

TEST_F(ChannelPoolUtest, ChannelPoolRunLoopWillDispatchChannelWhenProfChannelPollReturnValidChannel)
{
    struct ProfPollInfo channels[6] = {0};
    channels[0] = {0, PROF_CHANNEL_TS_FW};

    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    MOCKER_CPP(&ProfChannelPoll).stubs().with(outBoundP(channels), any(), any()).will(returnValue(1));
    // mock ProfChannelRead to return 0, so ChannelReader loop will return succ
    MOCKER_CPP(&ProfChannelRead).stubs().will(returnValue(0));

    EXPECT_EQ(MSPTI_SUCCESS, pool->Start());
    ASSERT_EQ(MSPTI_SUCCESS, pool->AddReader(0, PROF_CHANNEL_TS_FW));
    ASSERT_EQ(1, pool->readers_map_.size());

    pool->Stop();
}

TEST_F(ChannelPoolUtest, FlushDrvBuffWillReturnInnerErrorWhenInputInvalidDevIdAndChannelId)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    EXPECT_EQ(MSPTI_ERROR_INNER, pool->FlushDrvBuff(0, PROF_CHANNEL_TS_FW));
}

TEST_F(ChannelPoolUtest, FlushDrvBuffWillReturnSuccWhenFlushDrvBuffReturnSucc)
{
    std::shared_ptr<Mspti::Ascend::Channel::ChannelPool> pool;
    Mspti::Common::MsptiMakeSharedPtr(pool, 1);
    ASSERT_NE(pool, nullptr);

    ASSERT_EQ(MSPTI_SUCCESS, pool->AddReader(0, PROF_CHANNEL_TS_FW));
    ASSERT_EQ(1, pool->readers_map_.size());

    MOCKER_CPP(&Mspti::Ascend::Channel::ChannelReader::FlushDrvBuff).stubs().will(returnValue(MSPTI_SUCCESS));

    EXPECT_EQ(MSPTI_SUCCESS, pool->FlushDrvBuff(0, PROF_CHANNEL_TS_FW));
}

}  // namespace
