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

#include <memory>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"

#include "mockcpp/mockcpp.hpp"
#include "csrc/common/inject/inject_base.h"
#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/activity/ascend/channel/channel_reader.h"
#include "csrc/activity/ascend/channel/channel_pool.h"

namespace {
class ChannelReaderUtest : public testing::Test {
protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
    }
    virtual void TearDown() {}
};

// Test constructor and basic initialization
TEST_F(ChannelReaderUtest, ConstructorShouldInitializeCorrectly)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    // Verify initial scheduling status is false
    EXPECT_EQ(false, reader.GetSchedulingStatus());
}

// Test Init() function
TEST_F(ChannelReaderUtest, InitShouldReturnSuccessAndGenerateHashId)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    // HashId should be non-zero after initialization
    EXPECT_NE(0u, reader.HashId());
}

// Test HashId() function
TEST_F(ChannelReaderUtest, HashIdShouldReturnUniqueValueForDifferentInstances)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;

    Mspti::Ascend::Channel::ChannelReader reader1(deviceId, channelId);
    Mspti::Ascend::Channel::ChannelReader reader2(deviceId, channelId);

    reader1.Init();
    reader2.Init();

    // Different instances should have different hashIds (with high probability)
    EXPECT_NE(reader1.HashId(), reader2.HashId());
}

// Test Uinit() function
TEST_F(ChannelReaderUtest, UinitShouldReturnSuccess)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    reader.Init();
    EXPECT_EQ(MSPTI_SUCCESS, reader.Uinit());
}

// Test SetSchedulingStatus() and GetSchedulingStatus() functions
TEST_F(ChannelReaderUtest, SchedulingStatusShouldBeSetAndGetCorrectly)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    // Initial status should be false
    EXPECT_EQ(false, reader.GetSchedulingStatus());

    // Set to true
    reader.SetSchedulingStatus(true);
    EXPECT_EQ(true, reader.GetSchedulingStatus());

    // Set back to false
    reader.SetSchedulingStatus(false);
    EXPECT_EQ(false, reader.GetSchedulingStatus());
}

// Test SetChannelStopped() function
TEST_F(ChannelReaderUtest, SetChannelStoppedShouldStopExecuteLoop)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    reader.Init();
    reader.SetChannelStopped();

    // Execute should return quickly since channel is stopped
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.Uinit();
}

// Test Execute() function with TS_FW channel
TEST_F(ChannelReaderUtest, ExecuteShouldProcessTsFwChannelData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.Uinit();
}

// Test Execute() function with STARS_SOC_LOG channel
TEST_F(ChannelReaderUtest, ExecuteShouldProcessStarsSocLogChannelData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_STARS_SOC_LOG;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.Uinit();
}

// Test Execute() function with invalid channel ID
TEST_F(ChannelReaderUtest, ExecuteShouldBreakWhenChannelIdIsInvalid)
{
    constexpr unsigned int maxChannelId = 160;
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = static_cast<AI_DRV_CHANNEL>(maxChannelId);
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.Uinit();
}

// Test FlushDrvBuff() function
TEST_F(ChannelReaderUtest, FlushDrvBuffShouldReturnSuccess)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.FlushDrvBuff());
    reader.Uinit();
}

// Test FlushDrvBuff() with data to flush
TEST_F(ChannelReaderUtest, FlushDrvBuffShouldHandleFlushWithData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    // Start a thread to trigger flush completion
    std::thread flushThread([&reader]() {
        // Give FlushDrvBuff time to start waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        reader.SetChannelStopped();
    });

    EXPECT_EQ(MSPTI_SUCCESS, reader.FlushDrvBuff());

    if (flushThread.joinable()) {
        flushThread.join();
    }

    reader.Uinit();
}

// Test complete lifecycle: Init -> Execute -> Uinit
TEST_F(ChannelReaderUtest, LifecycleShouldWorkCorrectly)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    // Test complete lifecycle
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_NE(0u, reader.HashId());
    EXPECT_EQ(false, reader.GetSchedulingStatus());

    reader.SetSchedulingStatus(true);
    EXPECT_EQ(true, reader.GetSchedulingStatus());

    EXPECT_EQ(MSPTI_SUCCESS, reader.FlushDrvBuff());

    reader.SetChannelStopped();
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());

    EXPECT_EQ(MSPTI_SUCCESS, reader.Uinit());
}

// Test ChannelPool AddReader and RemoveReader
TEST_F(ChannelReaderUtest, ChannelPoolAddAndRemoveReaderShouldSucceed)
{
    std::unique_ptr<Mspti::Ascend::Channel::ChannelPool> drvChannelPoll_ =
        std::make_unique<Mspti::Ascend::Channel::ChannelPool>(1);
    EXPECT_EQ(MSPTI_SUCCESS, drvChannelPoll_->AddReader(1, PROF_CHANNEL_UNKNOWN));
    EXPECT_EQ(MSPTI_SUCCESS, drvChannelPoll_->RemoveReader(1, PROF_CHANNEL_UNKNOWN));
}
}  // namespace
