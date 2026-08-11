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

#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
using namespace Mspti::Common;
class ContextManagerUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        contextManager = ContextManager::GetInstance();
    }

    virtual void TearDown() {}

    ContextManager* contextManager{nullptr};
};

TEST_F(ContextManagerUtest, GetInstance)
{
    ASSERT_NE(contextManager, nullptr);
    ContextManager* anotherInstance = ContextManager::GetInstance();
    EXPECT_EQ(contextManager, anotherInstance);
}

TEST_F(ContextManagerUtest, GetDeviceRealTime)
{
    uint64_t sysCnt = 1000000;
    uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDeviceTimeInfo(deviceId));

    uint64_t result = contextManager->GetDeviceRealTime(deviceId, sysCnt);
    EXPECT_GT(result, 0ULL);
}

TEST_F(ContextManagerUtest, GetDeviceRealTimeVector)
{
    std::vector<uint64_t> sysCnts = {1000000, 2000000, 3000000};
    uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDeviceTimeInfo(deviceId));

    std::vector<uint64_t> results = contextManager->GetDeviceRealTime(deviceId, sysCnts);
    EXPECT_EQ(results.size(), sysCnts.size());
    for (const auto& result : results)
    {
        EXPECT_GT(result, 0ULL);
    }
}

TEST_F(ContextManagerUtest, GetHostRealTime)
{
    uint64_t sysCnt = 1000000;
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());

    uint64_t result = contextManager->GetHostRealTime(sysCnt);
    EXPECT_GT(result, 0ULL);
}

TEST_F(ContextManagerUtest, GetHostTimeStampNs)
{
    uint64_t timestamp1 = contextManager->GetHostTimeStampNs();
    uint64_t timestamp2 = contextManager->GetHostTimeStampNs();

    EXPECT_GE(timestamp2, timestamp1);
}

TEST_F(ContextManagerUtest, GetHostSysCnt)
{
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());
    uint64_t sysCnt1 = contextManager->GetHostSysCnt();
    uint64_t sysCnt2 = contextManager->GetHostSysCnt();

    EXPECT_GE(sysCnt2, sysCnt1);
    EXPECT_GT(sysCnt1, 0ULL);
}

TEST_F(ContextManagerUtest, GetChipType)
{
    uint32_t deviceId = 0;
    PlatformType chipType = contextManager->GetChipType(deviceId);

    EXPECT_TRUE(chipType == PlatformType::CHIP_910B || chipType == PlatformType::CHIP_310B ||
                chipType == PlatformType::CHIP_V6 || chipType == PlatformType::END_TYPE);
}

TEST_F(ContextManagerUtest, GetCorrelationId)
{
    uint32_t threadId = 1;
    uint64_t correlationId1 = contextManager->GetCorrelationId(threadId);
    uint64_t correlationId2 = contextManager->GetCorrelationId(threadId);

    EXPECT_EQ(correlationId1, correlationId2);
}

TEST_F(ContextManagerUtest, UpdateAndReportCorrelationId)
{
    uint64_t correlationId1 = contextManager->UpdateAndReportCorrelationId();
    uint64_t correlationId2 = contextManager->UpdateAndReportCorrelationId();

    EXPECT_GT(correlationId2, correlationId1);
}

TEST_F(ContextManagerUtest, UpdateAndReportCorrelationIdWithTid)
{
    uint32_t tid = 100;
    uint64_t correlationId1 = contextManager->UpdateAndReportCorrelationId(tid);
    uint64_t correlationId2 = contextManager->UpdateAndReportCorrelationId(tid);

    EXPECT_GT(correlationId2, correlationId1);
}

TEST_F(ContextManagerUtest, TimeCalculationFunctions)
{
    DevTimeInfo devTimeInfo;
    devTimeInfo.freq = 1000000000;  // 1GHz
    devTimeInfo.startRealTime = 1000000000000ULL;
    devTimeInfo.startSysCnt = 500000000000ULL;
    devTimeInfo.startMonotonicRawNs = 500000000000ULL;

    uint64_t sysCnt = 500000001000ULL;  // 1us

    uint64_t resultMonotonic = ContextManager::ConvertMonotonicCounterToRealTime(sysCnt, devTimeInfo);
    uint64_t resultSysCnt = ContextManager::ConvertSysCntToRealTime(sysCnt, devTimeInfo);
    uint64_t result = ContextManager::ConvertCounterToRealTime(sysCnt, devTimeInfo);

    EXPECT_GT(resultMonotonic, devTimeInfo.startRealTime);
    EXPECT_GT(resultSysCnt, devTimeInfo.startRealTime);
    EXPECT_EQ(result, resultSysCnt);
}

TEST_F(ContextManagerUtest, GetDeviceTimeInfo)
{
    uint32_t deviceId = 0;
    DevTimeInfo devTimeInfo;
    EXPECT_NO_THROW(contextManager->InitDeviceTimeInfo(deviceId));

    auto result = contextManager->GetDeviceTimeInfo(deviceId, devTimeInfo);
    EXPECT_TRUE(result);
    EXPECT_GT(devTimeInfo.freq, 0ULL);
}

TEST_F(ContextManagerUtest, ShouldInitDeviceFreqWithDefaultValueWhenDrvFailed)
{
    MOCKER_CPP(HalGetDeviceInfo).stubs().will(returnValue(DRV_ERROR_NOT_SUPPORT));
    constexpr uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDeviceTimeInfo(deviceId));
    DevTimeInfo devTimeInfo;
    EXPECT_TRUE(contextManager->GetDeviceTimeInfo(deviceId, devTimeInfo));
    constexpr uint64_t expectedFreq = 50ULL;
    EXPECT_EQ(expectedFreq, devTimeInfo.freq);
    constexpr uint64_t expectedStartSysCnt = 0ULL;
    EXPECT_EQ(expectedStartSysCnt, devTimeInfo.startSysCnt);
    EXPECT_GT(devTimeInfo.startRealTime, 0ULL);
    EXPECT_EQ(devTimeInfo.startMonotonicRawNs, 0ULL);
}

TEST_F(ContextManagerUtest, IsHostFreqEnabled)
{
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());
    bool isEnabled = contextManager->IsHostFreqEnabled();
    EXPECT_TRUE(isEnabled);
}

TEST_F(ContextManagerUtest, GetHostTimeInfo)
{
    DevTimeInfo devTimeInfo;
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());

    bool result = contextManager->GetHostTimeInfo(devTimeInfo);
    EXPECT_TRUE(result);
    EXPECT_GT(devTimeInfo.freq, 0);
}

TEST_F(ContextManagerUtest, StartStopSyncTime)
{
    msptiResult startResult = contextManager->StartSyncTime();
    EXPECT_EQ(startResult, MSPTI_SUCCESS);
    msptiResult stopResult = contextManager->StopSyncTime();
    EXPECT_EQ(stopResult, MSPTI_SUCCESS);
}

TEST_F(ContextManagerUtest, EncodeDstKeyWithAllZerosProducesZeroKey)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    uint64_t key = ContextManager::EncodeDstKey(0, 0, 0);
    EXPECT_EQ(key, 0ULL);
}

TEST_F(ContextManagerUtest, EncodeDstKeyWithMaxKeyProducesAllMax)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    uint64_t key = ContextManager::EncodeDstKey(UINT16_MAX, UINT16_MAX, UINT32_MAX);
    EXPECT_EQ(key, UINT64_MAX);
}

TEST_F(ContextManagerUtest, EncodeDstKeyWithNonZeroStreamPreservesValueOnNonV6Chip)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    const uint16_t deviceId = 0xABCD;
    const uint16_t streamId = 0x1234;
    const uint32_t taskId = 0x56789ABC;
    uint64_t key = ContextManager::EncodeDstKey(deviceId, streamId, taskId);
    uint64_t expected = (static_cast<uint64_t>(deviceId) << 48) | (static_cast<uint64_t>(streamId) << 32) |
                        static_cast<uint64_t>(taskId);
    EXPECT_EQ(key, expected);
}

TEST_F(ContextManagerUtest, EncodeDstKeyClearsStreamIdOnChipV6)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_V6));

    const uint16_t deviceId = 0xABCD;
    const uint16_t streamId = 0x1234;
    const uint32_t taskId = 0x56789ABC;
    uint64_t key = ContextManager::EncodeDstKey(deviceId, streamId, taskId);

    uint64_t expected = (static_cast<uint64_t>(deviceId) << 48) | static_cast<uint64_t>(taskId);
    EXPECT_EQ(key, expected);
}

TEST_F(ContextManagerUtest, EncodeDstKeyWithMaxValuesPreservesStreamOnNonV6Chip)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    uint64_t key = ContextManager::EncodeDstKey(UINT16_MAX, UINT16_MAX, UINT32_MAX);
    EXPECT_EQ(key, UINT64_MAX);
}

TEST_F(ContextManagerUtest, EncodeDstKeyOnChipV6WithMaxValuesProducesZeroStreamBits)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_V6));

    uint64_t key = ContextManager::EncodeDstKey(UINT16_MAX, UINT16_MAX, UINT32_MAX);
    uint64_t expected = (static_cast<uint64_t>(UINT16_MAX) << 48) | UINT32_MAX;
    EXPECT_EQ(key, expected);
}

TEST_F(ContextManagerUtest, EncodeDstKeyOnChipV6DifferentStreamIdsProduceSameKey)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_V6));

    const uint16_t deviceId = 1;
    const uint32_t taskId = 42;
    uint64_t keyA = ContextManager::EncodeDstKey(deviceId, 0, taskId);
    uint64_t keyB = ContextManager::EncodeDstKey(deviceId, 0xFF, taskId);
    uint64_t keyC = ContextManager::EncodeDstKey(deviceId, UINT16_MAX, taskId);

    EXPECT_EQ(keyA, keyB);
    EXPECT_EQ(keyB, keyC);
}

TEST_F(ContextManagerUtest, DecodeDstKeyWithZeroKeyReturnsAllZeros)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    auto decodedRes = ContextManager::DecodeDstKey(0ULL);
    EXPECT_EQ(std::get<0>(decodedRes), 0);
    EXPECT_EQ(std::get<1>(decodedRes), 0);
    EXPECT_EQ(std::get<2>(decodedRes), 0U);
}

TEST_F(ContextManagerUtest, DecodeDstKeyWithMaxKeyReturnsAllMax)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    auto decodedRes = ContextManager::DecodeDstKey(UINT64_MAX);
    EXPECT_EQ(std::get<0>(decodedRes), UINT16_MAX);
    EXPECT_EQ(std::get<1>(decodedRes), UINT16_MAX);
    EXPECT_EQ(std::get<2>(decodedRes), UINT32_MAX);
}

TEST_F(ContextManagerUtest, EncodeThenDecodeDstKeyRoundTripPreservesFieldsOnNonV6Chip)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_910B));

    struct TestCase
    {
        uint16_t deviceId;
        uint16_t streamId;
        uint32_t taskId;
    };
    const TestCase cases[] = {
        {1, 2, 3},
        {0xABCD, 0xFFFF, 0x56789ABC},
        {UINT16_MAX, UINT16_MAX, UINT32_MAX},
        {0x1234, 0x5678, 0x9ABCDEF0},
    };
    for (const auto& c : cases)
    {
        uint64_t key = ContextManager::EncodeDstKey(c.deviceId, c.streamId, c.taskId);
        auto decodedRes = ContextManager::DecodeDstKey(key);
        EXPECT_EQ(std::get<0>(decodedRes), c.deviceId);
        EXPECT_EQ(std::get<1>(decodedRes), c.streamId);
        EXPECT_EQ(std::get<2>(decodedRes), c.taskId);
    }
}

TEST_F(ContextManagerUtest, EncodeThenDecodeDstKeyRoundTripClearsStreamIdOnChipV6)
{
    MOCKER_CPP(&ContextManager::GetChipType).stubs().will(returnValue(PlatformType::CHIP_V6));

    struct TestCase
    {
        uint16_t deviceId;
        uint16_t streamId;
        uint32_t taskId;
    };
    const TestCase cases[] = {
        {1, 2, 3},
        {0xABCD, 0xFFFF, 0x56789ABC},
        {UINT16_MAX, UINT16_MAX, UINT32_MAX},
        {0x1234, 0x5678, 0x9ABCDEF0},
    };
    for (const auto& c : cases)
    {
        uint64_t key = ContextManager::EncodeDstKey(c.deviceId, c.streamId, c.taskId);
        auto decodedRes = ContextManager::DecodeDstKey(key);
        EXPECT_EQ(std::get<0>(decodedRes), c.deviceId);
        EXPECT_EQ(std::get<1>(decodedRes), 0);
        EXPECT_EQ(std::get<2>(decodedRes), c.taskId);
    }
}

static uint64_t g_timestampCallbackCallCount = 0;
static uint64_t TimestampCallbackImpl()
{
    g_timestampCallbackCallCount++;
    return Mspti::Common::Utils::GetClockRealTimeNs();
}

TEST_F(ContextManagerUtest, SetTimestampCallbackWithNullReturnsInvalidParam)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, contextManager->SetTimestampCallback(nullptr));
}

TEST_F(ContextManagerUtest, SetTimestampCallbackAndGetCurrentHostRealTimeInvokesCallback)
{
    g_timestampCallbackCallCount = 0;
    EXPECT_EQ(MSPTI_SUCCESS, contextManager->SetTimestampCallback(TimestampCallbackImpl));

    uint64_t t1 = contextManager->GetCurrentHostRealTimeNs();
    uint64_t t2 = contextManager->GetCurrentHostRealTimeNs();
    EXPECT_GE(t2, t1);
    EXPECT_GE(g_timestampCallbackCallCount, 1);
}
}  // namespace
