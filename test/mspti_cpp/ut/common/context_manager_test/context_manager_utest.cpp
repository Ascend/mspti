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
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/driver_inject.h"

namespace {
using namespace Mspti::Common;
class ContextManagerUtest : public testing::Test {
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

TEST_F(ContextManagerUtest, GetRealTimeFromSysCnt)
{
    uint64_t sysCnt = 1000000;
    uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDevTimeInfo(deviceId));

    uint64_t result = contextManager->GetRealTimeFromSysCnt(deviceId, sysCnt);
    EXPECT_GT(result, 0ULL);
}

TEST_F(ContextManagerUtest, GetRealTimeFromSysCntVector)
{
    std::vector<uint64_t> sysCnts = {1000000, 2000000, 3000000};
    uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDevTimeInfo(deviceId));

    std::vector<uint64_t> results = contextManager->GetRealTimeFromSysCnt(deviceId, sysCnts);
    EXPECT_EQ(results.size(), sysCnts.size());
    for (const auto& result : results) {
        EXPECT_GT(result, 0ULL);
    }
}

TEST_F(ContextManagerUtest, GetRealTimeFromSysCntHost)
{
    uint64_t sysCnt = 1000000;
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());

    uint64_t result = contextManager->GetRealTimeFromSysCnt(sysCnt);
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

    uint64_t resultMonotonic = ContextManager::CalculateRealTimeWithMonotonicTime(sysCnt, devTimeInfo);
    uint64_t resultSysCnt = ContextManager::CalculateRealTimeWithSysCnt(sysCnt, devTimeInfo);
    uint64_t result = ContextManager::CalculateRealTime(sysCnt, devTimeInfo);

    EXPECT_GT(resultMonotonic, devTimeInfo.startRealTime);
    EXPECT_GT(resultSysCnt, devTimeInfo.startRealTime);
    EXPECT_EQ(result, resultSysCnt);
}

TEST_F(ContextManagerUtest, GetCurDevTimeInfo)
{
    uint32_t deviceId = 0;
    DevTimeInfo devTimeInfo;
    EXPECT_NO_THROW(contextManager->InitDevTimeInfo(deviceId));

    auto result = contextManager->GetCurDevTimeInfo(deviceId, devTimeInfo);
    EXPECT_TRUE(result);
    EXPECT_GT(devTimeInfo.freq, 0ULL);
}

TEST_F(ContextManagerUtest, ShouldInitDeviceFreqWithDefaultValueWhenDrvFailed)
{
    MOCKER_CPP(HalGetDeviceInfo).stubs().will(returnValue(DRV_ERROR_NOT_SUPPORT));
    constexpr uint32_t deviceId = 0;
    EXPECT_NO_THROW(contextManager->InitDevTimeInfo(deviceId));
    DevTimeInfo devTimeInfo;
    EXPECT_TRUE(contextManager->GetCurDevTimeInfo(deviceId, devTimeInfo));
    constexpr uint64_t expectedFreq = 50ULL;
    EXPECT_EQ(expectedFreq, devTimeInfo.freq);
    constexpr uint64_t expectedStartSysCnt = 0ULL;
    EXPECT_EQ(expectedStartSysCnt, devTimeInfo.startSysCnt);
    EXPECT_GT(devTimeInfo.startRealTime, 0ULL);
    EXPECT_EQ(devTimeInfo.startMonotonicRawNs, 0ULL);
}

TEST_F(ContextManagerUtest, HostFreqIsEnable)
{
    EXPECT_NO_THROW(contextManager->InitHostTimeInfo());
    bool isEnabled = contextManager->HostFreqIsEnable();
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
}  // namespace
