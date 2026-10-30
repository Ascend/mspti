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

#include <chrono>
#include <memory>
#include <thread>

#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/channel/channel_pool.h"
#include "csrc/activity/ascend/channel/channel_reader.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/inject/inject_base.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
class ChannelReaderUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
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

// Test UnInit() function
TEST_F(ChannelReaderUtest, UinitShouldReturnSuccess)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    reader.Init();
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
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
    reader.UnInit();
}

// Test Execute() function with TS_FW channel
TEST_F(ChannelReaderUtest, ExecuteShouldProcessTsFwChannelData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.UnInit();
}

// Test Execute() function with STARS_SOC_LOG channel
TEST_F(ChannelReaderUtest, ExecuteShouldProcessStarsSocLogChannelData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_STARS_SOC_LOG;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    reader.UnInit();
}

// Test Init() fails fast for invalid channel ID instead of stalling later;
// Execute() without a successful Init is a no-op.
TEST_F(ChannelReaderUtest, InitShouldReturnErrorWhenChannelIdIsInvalid)
{
    constexpr unsigned int maxChannelId = 160;
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = static_cast<AI_DRV_CHANNEL>(maxChannelId);
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_ERROR_INNER, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test FlushDrvBuff() function
TEST_F(ChannelReaderUtest, FlushDrvBuffShouldReturnSuccess)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.FlushDrvBuff());
    reader.UnInit();
}

TEST_F(ChannelReaderUtest, FlushDrvBuffShouldReturnSuccessWhenDriverDoesNotSupportFlush)
{
    MOCKER_CPP(&HalProfDataFlush).stubs().will(returnValue(static_cast<int32_t>(DRV_ERROR_NOT_SUPPORT)));
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_TS_FW);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    msptiResult flushResult = MSPTI_ERROR_INNER;
    std::thread flushThread([&reader, &flushResult]() { flushResult = reader.FlushDrvBuff(); });
    flushThread.join();
    EXPECT_EQ(MSPTI_SUCCESS, flushResult);
    reader.UnInit();
    GlobalMockObject::verify();
}

TEST_F(ChannelReaderUtest, FlushDrvBuffShouldReturnInnerErrorWhenDriverFlushFails)
{
    constexpr int32_t flushFailed = 1;
    MOCKER_CPP(&HalProfDataFlush).stubs().will(returnValue(flushFailed));
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_TS_FW);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    msptiResult flushResult = MSPTI_SUCCESS;
    std::thread flushThread([&reader, &flushResult]() { flushResult = reader.FlushDrvBuff(); });
    flushThread.join();
    EXPECT_EQ(MSPTI_ERROR_INNER, flushResult);
    reader.UnInit();
    GlobalMockObject::verify();
}

// Test FlushDrvBuff() with data to flush
TEST_F(ChannelReaderUtest, FlushDrvBuffShouldHandleFlushWithData)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    // Start a thread to trigger flush completion
    std::thread flushThread(
        [&reader]()
        {
            // Give FlushDrvBuff time to start waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            reader.SetChannelStopped();
        });

    EXPECT_EQ(MSPTI_SUCCESS, reader.FlushDrvBuff());

    if (flushThread.joinable())
    {
        flushThread.join();
    }

    reader.UnInit();
}

// Test complete lifecycle: Init -> Execute -> UnInit
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

    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test multiple Init/UnInit cycles (guards buffer reallocation and curPos_ reset)
TEST_F(ChannelReaderUtest, MultipleLifecycleCyclesShouldWork)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test Init/Execute/UnInit followed by re-Init/Execute/UnInit
// (guards curPos_ reset and buffer reallocation across lifecycle boundaries)
TEST_F(ChannelReaderUtest, ExecuteAfterReInitShouldSucceed)
{
    uint32_t deviceId = 0;
    AI_DRV_CHANNEL channelId = PROF_CHANNEL_TS_FW;
    Mspti::Ascend::Channel::ChannelReader reader(deviceId, channelId);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    reader.SetChannelStopped();
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test that Execute can process multiple channels in sequence
// (guards curPos_ member variable independent behavior per-instance)
TEST_F(ChannelReaderUtest, MultipleReadersExecuteShouldWorkIndependently)
{
    uint32_t deviceId = 0;
    Mspti::Ascend::Channel::ChannelReader reader1(deviceId, PROF_CHANNEL_TS_FW);
    Mspti::Ascend::Channel::ChannelReader reader2(deviceId, PROF_CHANNEL_STARS_SOC_LOG);

    EXPECT_EQ(MSPTI_SUCCESS, reader1.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader2.Init());

    reader1.SetChannelStopped();
    reader2.SetChannelStopped();

    EXPECT_EQ(MSPTI_SUCCESS, reader1.Execute());
    EXPECT_EQ(MSPTI_SUCCESS, reader2.Execute());

    EXPECT_EQ(MSPTI_SUCCESS, reader1.UnInit());
    EXPECT_EQ(MSPTI_SUCCESS, reader2.UnInit());
}

// Test Execute() without Init(): read loop is skipped, returns immediately
TEST_F(ChannelReaderUtest, ExecuteShouldReturnQuicklyWhenNotInited)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
}

// Test UnInit() is idempotent: second call joins nothing and does not crash
TEST_F(ChannelReaderUtest, DoubleUnInitShouldBeSafe)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test destructor joins the consumer thread even if UnInit was never called
TEST_F(ChannelReaderUtest, DestructorShouldJoinConsumeThreadWithoutUnInit)
{
    auto reader = std::make_unique<Mspti::Ascend::Channel::ChannelReader>(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader->Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader->Execute());
    // intentionally skip UnInit: destructor must drain and join, no std::terminate
    reader.reset();
}

// Test full-buffer handoff to the consumer thread with mocked driver data.
// The mock returns exactly one full buffer then 0, so Execute hands off once
// and UnInit drains the consumer queue; asserts the async path end to end.
TEST_F(ChannelReaderUtest, ExecuteShouldHandOffFullBufferToConsumerThread)
{
    constexpr int fullBufferSize = 2 * 1024 * 1024;
    MOCKER_CPP(&ProfChannelRead).stubs().will(returnValue(fullBufferSize)).then(returnValue(0));
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
    GlobalMockObject::verify();
}

// Test that handoff is rejected once the consumer thread has stopped: data must
// stay in the reader buffer so the final drain can process it synchronously.
TEST_F(ChannelReaderUtest, HandoffShouldBeRejectedAfterConsumerStopped)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());

    reader.buffer_.assign(512, 'y');
    reader.curPos_ = 512;

    EXPECT_FALSE(reader.TryHandoffBufferedData(256));
    EXPECT_EQ(512u, reader.curPos_);
    EXPECT_EQ(std::string(512, 'y'), reader.buffer_.substr(0, 512));
    EXPECT_EQ(0u, reader.consumerQueue_->size());
}

// Test that the consume queue capacity is a fixed entry count and does not depend on
// the buffer size.
TEST_F(ChannelReaderUtest, QueueCapacityShouldBeFixed)
{
    constexpr size_t expectedCapacity = 128;
    constexpr uint32_t bufferSize = 2 * 1024 * 1024;
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG, bufferSize);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(expectedCapacity, reader.consumeQueueCapacity_);
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());

    constexpr uint32_t hugeBufferSize = 64 * 1024 * 1024;
    Mspti::Ascend::Channel::ChannelReader hugeReader(0, PROF_CHANNEL_STARS_SOC_LOG, hugeBufferSize);
    EXPECT_EQ(MSPTI_SUCCESS, hugeReader.Init());
    EXPECT_EQ(expectedCapacity, hugeReader.consumeQueueCapacity_);
    EXPECT_EQ(MSPTI_SUCCESS, hugeReader.UnInit());
}

// Test the handoff queue preserves FIFO order (consumer parses in read order).
TEST_F(ChannelReaderUtest, HandoffQueuePreservesFifoOrder)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());  // stop consumer thread for race-free driving

    for (char tag : {'a', 'b', 'c'})
    {
        Mspti::Ascend::Channel::ChannelReader::BufferData chunk;
        chunk.data.assign(4, tag);
        chunk.validSize = 4;
        EXPECT_TRUE(reader.consumerQueue_->TryPush(std::move(chunk)));
    }
    for (char tag : {'a', 'b', 'c'})
    {
        Mspti::Ascend::Channel::ChannelReader::BufferData out;
        EXPECT_TRUE(reader.consumerQueue_->TryPop(out));
        EXPECT_EQ(4u, out.validSize);
        EXPECT_EQ(std::string(4, tag), out.data);
    }
}

// Test DrainToConsumer() flushes residual bytes that are below the batch threshold:
// the residual must be handed off to the consumer thread and fully consumed before return.
TEST_F(ChannelReaderUtest, DrainToConsumerShouldFlushResidualBelowThreshold)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    // stage one complete struct (far below the 1.6MB handoff threshold) in the reader buffer
    const size_t structSize = reader.dataStructSize_;
    ASSERT_GT(structSize, 0u);
    reader.buffer_.assign(structSize, '\0');
    reader.curPos_ = structSize;

    // drain must hand the residual off and wait until the consumer finished parsing it
    EXPECT_EQ(MSPTI_SUCCESS, reader.DrainToConsumer());
    EXPECT_EQ(0u, reader.curPos_);
    EXPECT_EQ(0, reader.pendingConsumeCount_.load());

    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test DrainToConsumer() is a no-op when the reader was never inited (no consumer queue)
TEST_F(ChannelReaderUtest, DrainToConsumerShouldReturnWhenNotInited)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_TS_FW);
    EXPECT_EQ(MSPTI_SUCCESS, reader.DrainToConsumer());
}

// Test DrainToConsumer() returns immediately for discard channels (no consumer queue)
TEST_F(ChannelReaderUtest, DrainToConsumerShouldReturnForDiscardChannel)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_AICPU);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.DrainToConsumer());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test the handoff rejects an invalid chunk size without touching the buffered bytes
TEST_F(ChannelReaderUtest, HandoffShouldRejectInvalidConsumedSize)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    reader.buffer_.assign(64, '\0');
    reader.curPos_ = 64;

    // nothing to hand off, and more than was read: both must be refused as-is
    EXPECT_FALSE(reader.TryHandoffBufferedData(0));
    EXPECT_FALSE(reader.TryHandoffBufferedData(65));
    EXPECT_EQ(64u, reader.curPos_);
    EXPECT_EQ(std::string(64, '\0'), reader.buffer_);

    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test a successful handoff keeps the reader buffer writable: the string size must be
// restored to bufferSize_, otherwise the next ProfChannelRead writes past size() and the
// residual accounting operates on a truncated buffer.
TEST_F(ChannelReaderUtest, SuccessfulHandoffShouldRestoreBufferSize)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());

    const size_t bufferSize = static_cast<size_t>(reader.bufferSize_);
    const size_t structSize = reader.dataStructSize_;
    ASSERT_GT(structSize, 0u);
    constexpr size_t tailSize = 5;
    const size_t consumedSize = 4 * structSize;
    reader.buffer_.assign(consumedSize + tailSize, '\0');
    reader.curPos_ = consumedSize + tailSize;

    EXPECT_TRUE(reader.TryHandoffBufferedData(consumedSize));
    // only the unaligned tail stays pending, and the buffer is fully sized again
    EXPECT_EQ(tailSize, reader.curPos_);
    EXPECT_EQ(bufferSize, reader.buffer_.size());

    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
}

// Test backpressure: a full consume queue makes the handoff fail without losing a byte,
// rolls the pending count back and counts the rejection, so the next scheduling tick
// retries with the whole buffer intact.
TEST_F(ChannelReaderUtest, FullQueueShouldKeepBufferForRetry)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    // stop the consumer thread first so nothing drains the queue behind our back
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
    reader.consumerRunning_.store(true);

    Mspti::Ascend::Channel::ChannelReader::BufferData chunk;
    chunk.data.assign(64, '\0');
    chunk.validSize = 64;
    size_t pushed = 0;
    while (reader.consumerQueue_->TryPush(chunk))
    {
        ++pushed;
    }
    ASSERT_EQ(reader.consumeQueueCapacity_, pushed);

    reader.buffer_.assign(1024, '\0');
    reader.curPos_ = 1024;

    EXPECT_FALSE(reader.TryHandoffBufferedData(256));
    EXPECT_EQ(1024u, reader.curPos_);
    EXPECT_EQ(1024u, reader.buffer_.size());
    EXPECT_EQ(0, reader.pendingConsumeCount_.load());
    EXPECT_EQ(1u, reader.queueFullCount_.load());

    // neutralise the pre-stopped reader so the destructor does not parse the filler
    reader.consumerRunning_.store(false);
    reader.curPos_ = 0;
    Mspti::Ascend::Channel::ChannelReader::BufferData drop;
    while (reader.consumerQueue_->TryPop(drop))
    {
    }
}

// Test the shutdown drain after the consumer thread is gone: complete structs are parsed
// synchronously and an unaligned tail is dropped, so UnInit never discards the whole batch.
TEST_F(ChannelReaderUtest, DrainBufferedDataShouldParseSynchronouslyAfterConsumerStopped)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());  // consumer thread is stopped from here on

    const size_t structSize = reader.dataStructSize_;
    ASSERT_GT(structSize, 0u);
    constexpr size_t tailSize = 3;
    reader.buffer_.assign(2 * structSize + tailSize, '\0');
    reader.curPos_ = 2 * structSize + tailSize;

    reader.DrainBufferedDataToConsumer();
    EXPECT_EQ(0u, reader.curPos_);
}

// Test StopConsumerThread drains whatever is still queued after the join, so no chunk is
// left unparsed when the queue is destroyed.
TEST_F(ChannelReaderUtest, StopConsumerThreadShouldDrainQueueResidue)
{
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);
    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());

    const size_t structSize = reader.dataStructSize_;
    ASSERT_GT(structSize, 0u);
    Mspti::Ascend::Channel::ChannelReader::BufferData chunk;
    chunk.data.assign(structSize, '\0');
    chunk.validSize = structSize;
    ASSERT_TRUE(reader.consumerQueue_->TryPush(chunk));
    reader.pendingConsumeCount_.store(1);

    reader.StopConsumerThread();
    EXPECT_EQ(0u, reader.consumerQueue_->size());
    EXPECT_EQ(0, reader.pendingConsumeCount_.load());
}

// Test AICPU data is read and dropped: the discard channel never accumulates bytes
TEST_F(ChannelReaderUtest, ExecuteShouldDiscardAicpuData)
{
    constexpr int dataSize = 1024;
    MOCKER_CPP(&ProfChannelRead).stubs().will(returnValue(dataSize)).then(returnValue(0));
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_AICPU);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(0u, reader.curPos_);
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());
    GlobalMockObject::verify();
}

// Test SOC_LOG reads below the handoff threshold stay buffered for the next tick,
// so a slow trickle of records is not handed off one chunk at a time.
TEST_F(ChannelReaderUtest, ExecuteShouldKeepDataBelowHandoffThreshold)
{
    constexpr int dataSize = 1024;
    MOCKER_CPP(&ProfChannelRead).stubs().will(returnValue(dataSize)).then(returnValue(0));
    Mspti::Ascend::Channel::ChannelReader reader(0, PROF_CHANNEL_STARS_SOC_LOG);

    EXPECT_EQ(MSPTI_SUCCESS, reader.Init());
    EXPECT_EQ(MSPTI_SUCCESS, reader.Execute());
    EXPECT_EQ(static_cast<size_t>(dataSize), reader.curPos_);
    EXPECT_LT(reader.curPos_, reader.handoffThreshold_);
    EXPECT_EQ(0u, reader.consumerQueue_->size());
    EXPECT_EQ(MSPTI_SUCCESS, reader.UnInit());  // UnInit drains the residual
    GlobalMockObject::verify();
}
}  // namespace
