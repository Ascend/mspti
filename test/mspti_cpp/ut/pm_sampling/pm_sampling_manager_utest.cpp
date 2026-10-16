/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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
#include "csrc/pm_sampling/dcmi/dcmi_collector.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
using namespace Mspti;
using namespace Mspti::PmSampling;

constexpr uint64_t kValidIntervalNs = 100000000ULL;  // 100ms
constexpr uint64_t kOneSecondNs = 1000000000ULL;

class PmSamplingManagerUtest : public testing::Test
{
   protected:
    void SetUp() override
    {
        // manager 是 leaky 单例，lastDcmiNs_ 跨用例残留；每用例将 nowNs 基准大幅前推规避
        nowNs_ += 100ULL * kOneSecondNs;
    }

    void TearDown() override
    {
        for (auto h : handles_)
        {
            (void)PmSamplingManager::GetInstance()->Disable(h);
        }
        handles_.clear();
        // 释放本用例的 mockcpp hook，避免泄漏到后续用例
        GlobalMockObject::verify();
    }

    msptiPmSamplingObjectPtr EnableOk(uint32_t deviceId = 0)
    {
        msptiPmSamplingObjectPtr h = nullptr;
        EXPECT_EQ(MSPTI_SUCCESS, PmSamplingManager::GetInstance()->Enable(deviceId, &h));
        handles_.push_back(h);
        return h;
    }

    std::shared_ptr<PmSamplingObject> Live(msptiPmSamplingObjectPtr h)
    {
        return PmSamplingManager::GetInstance()->GetLiveObject(h);
    }

    static msptiResult Config(PmSamplingObject &obj, const std::vector<uint32_t> &ids,
                              uint64_t intervalNs = kValidIntervalNs)
    {
        return obj.SetConfig(static_cast<uint32_t>(ids.size()), ids.data(), intervalNs,
                             MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, 0,
                             MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST);
    }

    // 配置并直调 Start 进入 Running（不走 StartCollectors，UT 环境无 dcmi 采集线程）
    void ToRunning(const std::vector<uint32_t> &ids)
    {
        auto obj = Live(EnableOk());
        ASSERT_EQ(MSPTI_SUCCESS, Config(*obj, ids));
        ASSERT_EQ(MSPTI_SUCCESS, obj->Start());
    }

    std::vector<msptiPmSamplingObjectPtr> handles_;
    uint64_t nowNs_ = 1000ULL * kOneSecondNs;
};

/*
 * 测试目的：校验 Enable 的三类拒绝：空出参指针、离线设备（stub 仅 device0 在线）、
 *           单设备单会话下的重复 Enable
 */
TEST_F(PmSamplingManagerUtest, EnableShouldRejectWhenPointerNullDeviceOfflineOrDuplicated)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, PmSamplingManager::GetInstance()->Enable(0, nullptr));
    msptiPmSamplingObjectPtr h = nullptr;
    EXPECT_EQ(MSPTI_ERROR_DEVICE_OFFLINE, PmSamplingManager::GetInstance()->Enable(1, &h));

    (void)EnableOk();
    EXPECT_EQ(MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED, PmSamplingManager::GetInstance()->Enable(0, &h));
}

/*
 * 测试目的：校验 dcmi 不可用时采集启动失败回滚：返回 DEVICE_OFFLINE，会话不留在 Running
 */
TEST_F(PmSamplingManagerUtest, StartCollectorsShouldRollbackWhenDcmiStartFails)
{
    MOCKER_CPP(&Mspti::PmSampling::DcmiCollector::StartCollector).stubs().will(returnValue(MSPTI_ERROR_DEVICE_OFFLINE));

    auto obj = Live(EnableOk());
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    ASSERT_EQ(MSPTI_SUCCESS, Config(*obj, ids));

    EXPECT_EQ(MSPTI_ERROR_DEVICE_OFFLINE, PmSamplingManager::GetInstance()->StartCollectors(*obj));
    // 回滚：不留 Running 会话（否则成为永远无数据的静默会话）
    EXPECT_EQ(PmSamplingObject::State::Stopped, obj->GetState());
    EXPECT_FALSE(obj->IsRunning());
}

/*
 * 测试目的：校验句柄安全契约：空句柄/野指针（仅按指针值查，不解引用）的 Disable 与
 *           GetLiveObject 行为，以及 Disable 后墓碑生效、重复 Disable 拒绝
 */
TEST_F(PmSamplingManagerUtest, DisableAndLiveQueryShouldFollowTombstoneSemanticsWhenHandleInvalid)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, PmSamplingManager::GetInstance()->Disable(nullptr));
    auto wild = reinterpret_cast<msptiPmSamplingObjectPtr>(0xdeadbeefULL);
    EXPECT_EQ(MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED, PmSamplingManager::GetInstance()->Disable(wild));
    EXPECT_EQ(nullptr, Live(nullptr));
    EXPECT_EQ(nullptr, Live(wild));

    auto h = EnableOk();
    auto obj = Live(h);
    ASSERT_NE(nullptr, obj);
    EXPECT_EQ(0U, obj->DeviceId());

    (void)PmSamplingManager::GetInstance()->Disable(h);
    EXPECT_EQ(nullptr, Live(h));
    EXPECT_EQ(MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED, PmSamplingManager::GetInstance()->Disable(h));
}

/*
 * 测试目的：校验无 Running 会话不放行；Running 后闸门放行时更新基准，
 *           未到最小周期不放行，到期后再次放行
 */
TEST_F(PmSamplingManagerUtest, ShouldCollectDcmiShouldThrottleByMinIntervalWhenRunning)
{
    EXPECT_FALSE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_));

    ToRunning({MSPTI_PM_SAMPLING_METRIC_ID_POWER});

    EXPECT_TRUE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_));
    EXPECT_FALSE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_ + 50 * 1000000ULL));
    EXPECT_TRUE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_ + 100 * 1000000ULL));
    EXPECT_FALSE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_ + 150 * 1000000ULL));
    EXPECT_TRUE(PmSamplingManager::GetInstance()->ShouldCollectDcmi(nowNs_ + 200 * 1000000ULL));
}

/*
 * 测试目的：校验 CollectNeeds 为 Running 会话指标并集：Configured 会话不计入，
 * Running 会话按 metricId 映射到 DCMI 接口组（util 类合并、HCCS 两个方向合并）
 */
TEST_F(PmSamplingManagerUtest, CollectNeedsShouldUnionRunningSessionMetrics)
{
    auto needs = PmSamplingManager::GetInstance()->CollectNeeds();
    EXPECT_FALSE(needs.power);
    EXPECT_FALSE(needs.temperature);
    EXPECT_FALSE(needs.utilization);
    EXPECT_FALSE(needs.frequency);
    EXPECT_FALSE(needs.hbm);
    EXPECT_FALSE(needs.hccsBw);
    EXPECT_FALSE(needs.pcieBw);
    EXPECT_FALSE(needs.roceBw);

    auto obj = Live(EnableOk());
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE,
                                       MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW};
    ASSERT_EQ(MSPTI_SUCCESS, Config(*obj, ids));
    // Configured 未 Start，不计入需求
    needs = PmSamplingManager::GetInstance()->CollectNeeds();
    EXPECT_FALSE(needs.power);

    // Start 后计入并集：util 类合并、HCCS 两方向合并为 hccsBw
    ASSERT_EQ(MSPTI_SUCCESS, obj->Start());
    needs = PmSamplingManager::GetInstance()->CollectNeeds();
    EXPECT_TRUE(needs.power);
    EXPECT_TRUE(needs.temperature);
    EXPECT_TRUE(needs.utilization);
    EXPECT_TRUE(needs.hccsBw);
    EXPECT_FALSE(needs.frequency);
    EXPECT_FALSE(needs.hbm);
    EXPECT_FALSE(needs.pcieBw);
    EXPECT_FALSE(needs.roceBw);
}

/*
 * 测试目的：校验 MinDcmiIntervalMs：无 Running 会话为 0，Running 会话按配置间隔换算 ms
 */
TEST_F(PmSamplingManagerUtest, MinDcmiIntervalMsShouldFollowRunningSessionInterval)
{
    EXPECT_EQ(0U, PmSamplingManager::GetInstance()->MinDcmiIntervalMs());

    ToRunning({MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    EXPECT_EQ(100U, PmSamplingManager::GetInstance()->MinDcmiIntervalMs());

    // 停止后重配为 1s，重新 Start 生效
    auto obj = Live(handles_.back());
    ASSERT_EQ(MSPTI_SUCCESS, obj->Stop());
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    ASSERT_EQ(MSPTI_SUCCESS, Config(*obj, ids, kOneSecondNs));
    ASSERT_EQ(MSPTI_SUCCESS, obj->Start());
    EXPECT_EQ(1000U, PmSamplingManager::GetInstance()->MinDcmiIntervalMs());
}

/*
 * 测试目的：校验 HasDcmiDemand 状态语义：Configured 不算需求，Running/Stopped 均算
 */
TEST_F(PmSamplingManagerUtest, HasDcmiDemandShouldFollowSessionState)
{
    EXPECT_FALSE(PmSamplingManager::GetInstance()->HasDcmiDemand());

    auto obj = Live(EnableOk());
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    ASSERT_EQ(MSPTI_SUCCESS, Config(*obj, ids));
    EXPECT_FALSE(PmSamplingManager::GetInstance()->HasDcmiDemand());

    ASSERT_EQ(MSPTI_SUCCESS, obj->Start());
    EXPECT_TRUE(PmSamplingManager::GetInstance()->HasDcmiDemand());

    ASSERT_EQ(MSPTI_SUCCESS, obj->Stop());
    EXPECT_TRUE(PmSamplingManager::GetInstance()->HasDcmiDemand());
}

/*
 * 测试目的：校验无 parser 注册时原始样本入口与投递入口安全忽略（不 crash、不入队）
 */
TEST_F(PmSamplingManagerUtest, SampleEntryShouldIgnoreWhenNoParserRegistered)
{
    PmDcmiSample sample;
    sample.deviceId = 0;
    sample.tsNs = 1000;
    PmSamplingManager::GetInstance()->OnDcmiSample(sample);

    PmSessionSample sessionSample;
    sessionSample.entries.push_back({MSPTI_PM_SAMPLING_METRIC_ID_POWER, 0, 1.0});
    PmSamplingManager::GetInstance()->DeliverLocked(5, std::move(sessionSample));

    // 会话 Running 但无 parser（未走 StartCollectors），样本不应入队
    ToRunning({MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    auto obj = Live(handles_.back());
    PmSamplingManager::GetInstance()->OnDcmiSample(sample);
    EXPECT_EQ(0U, obj->QueueCount());
}
}  // namespace
