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
#include <memory>

#include "csrc/pm_sampling/pm_sampling_manager.h"
#include "csrc/pm_sampling/pm_sampling_parser.h"
#include "gtest/gtest.h"

namespace
{
using namespace Mspti;
using namespace Mspti::PmSampling;

constexpr uint64_t kValidIntervalNs = 100000000ULL;  // 100ms

class PmSamplingParserUtest : public testing::Test
{
   protected:
    void TearDown() override
    {
        if (handle_ != nullptr)
        {
            (void)PmSamplingManager::GetInstance()->Disable(handle_);
            handle_ = nullptr;
        }
    }

    // 建会话并直调 Start 进入 Running（不注册 manager parser），自建 parser 供定向喂样
    PmSamplingObject *SetupSession(const std::vector<uint32_t> &ids)
    {
        msptiPmSamplingObjectPtr h = nullptr;
        EXPECT_EQ(MSPTI_SUCCESS, PmSamplingManager::GetInstance()->Enable(0, &h));
        handle_ = h;
        auto obj = PmSamplingManager::GetInstance()->GetLiveObject(h);
        EXPECT_EQ(MSPTI_SUCCESS, obj->SetConfig(static_cast<uint32_t>(ids.size()), ids.data(), kValidIntervalNs,
                                                MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, 0,
                                                MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST));
        EXPECT_EQ(MSPTI_SUCCESS, obj->Start());
        parser_ = std::unique_ptr<PmSamplingParser>(new PmSamplingParser(ids));
        return obj.get();
    }

    static PmDcmiSample MakeSample(uint64_t tsNs, int32_t power)
    {
        PmDcmiSample sample;
        sample.deviceId = 0;
        sample.tsNs = tsNs;
        sample.power = power;
        return sample;
    }

    msptiPmSamplingObjectPtr handle_ = nullptr;
    std::unique_ptr<PmSamplingParser> parser_;
};

/*
 * 测试目的：校验首 tick 仅作窗口基准不产出，第二个 tick 产出 [首tick, 当前tick] 窗口，
 *           entries 顺序 = 配置顺序（与喂样字段无关），temperature sentinel 归 0，
 *           power 由 0.1W 归一化为 W
 */
TEST_F(PmSamplingParserUtest, SecondTickShouldEmitWindowInConfiguredOrderWithNormalizedPower)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE, MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    parser_->OnDcmiSample(MakeSample(1000, 0));
    EXPECT_EQ(0U, obj->QueueCount());  // 首 tick 仅作基准

    parser_->OnDcmiSample(MakeSample(2000, 1052));

    ASSERT_EQ(1U, obj->QueueCount());
    PmSessionSample sample;
    ASSERT_TRUE(obj->GetQueueSample(0, sample));
    EXPECT_EQ(1000U, sample.startNs);
    EXPECT_EQ(2000U, sample.endNs);
    ASSERT_EQ(2U, sample.entries.size());
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE), sample.entries[0].metricId);
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_POWER), sample.entries[1].metricId);
    EXPECT_DOUBLE_EQ(0.0, sample.entries[0].value);    // temperature sentinel(-1) -> 0
    EXPECT_DOUBLE_EQ(105.2, sample.entries[1].value);  // 0.1W -> W
}

/*
 * 测试目的：校验 sentinel（power 实值 0 透传、temperature/util 类默认 sentinel）按 0 组装
 */
TEST_F(PmSamplingParserUtest, SentinelShouldNormalizeToZero)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE,
                             MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL});
    parser_->OnDcmiSample(MakeSample(1000, 0));
    // MakeSample 默认值：power 显式传 0，temperature/-1、util/0xFFFFFFFF 走默认 sentinel
    parser_->OnDcmiSample(MakeSample(2000, 0));

    ASSERT_EQ(1U, obj->QueueCount());
    PmSessionSample sample;
    ASSERT_TRUE(obj->GetQueueSample(0, sample));
    ASSERT_EQ(3U, sample.entries.size());
    EXPECT_DOUBLE_EQ(0.0, sample.entries[0].value);  // power=0 非 sentinel，原值透传
    EXPECT_DOUBLE_EQ(0.0, sample.entries[1].value);  // temperature sentinel -> 0
    EXPECT_DOUBLE_EQ(0.0, sample.entries[2].value);  // aicoreUtil sentinel -> 0
}

/*
 * 测试目的：校验 HCCS 逐链路透传（instanceId/值不改）、PCIE 单实例取首个点、
 *           本窗口无数据的带宽类指标补一条 0
 */
TEST_F(PmSamplingParserUtest, BandwidthShouldPassThroughPerLinkAndFillZeroWhenAbsent)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW,
                             MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW, MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW});
    parser_->OnDcmiSample(MakeSample(1000, 0));

    PmDcmiSample cur = MakeSample(2000, 0);
    cur.bandwidths.push_back({PM_BW_HCCS_TX, 3, 1.5e9});
    cur.bandwidths.push_back({PM_BW_HCCS_TX, 7, 2.5e9});
    cur.bandwidths.push_back({PM_BW_PCIE_TX, 0, 1.0e8});
    parser_->OnDcmiSample(cur);

    ASSERT_EQ(1U, obj->QueueCount());
    PmSessionSample out;
    ASSERT_TRUE(obj->GetQueueSample(0, out));
    ASSERT_EQ(5U, out.entries.size());
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW), out.entries[0].metricId);
    EXPECT_EQ(3U, out.entries[0].instanceId);
    EXPECT_DOUBLE_EQ(1.5e9, out.entries[0].value);
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW), out.entries[1].metricId);
    EXPECT_EQ(7U, out.entries[1].instanceId);
    EXPECT_DOUBLE_EQ(2.5e9, out.entries[1].value);
    // HCCS RX 本窗口无数据，补一条 0
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW), out.entries[2].metricId);
    EXPECT_EQ(0U, out.entries[2].instanceId);
    EXPECT_DOUBLE_EQ(0.0, out.entries[2].value);
    // PCIE 单实例，取首个匹配点
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW), out.entries[3].metricId);
    EXPECT_DOUBLE_EQ(1.0e8, out.entries[3].value);
    // ROCE RX 本窗口无数据，补一条 0
    EXPECT_EQ(static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW), out.entries[4].metricId);
    EXPECT_DOUBLE_EQ(0.0, out.entries[4].value);
}

/*
 * 测试目的：校验时间倒退时重置窗口基准不产出，后续 tick 以新基准组窗
 */
TEST_F(PmSamplingParserUtest, TimestampFallbackShouldResetBaselineWithoutOutput)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    parser_->OnDcmiSample(MakeSample(2000, 0));
    // 倒退：仅重置基准
    parser_->OnDcmiSample(MakeSample(1000, 0));
    EXPECT_EQ(0U, obj->QueueCount());
    // 新窗口从重置后的基准 1000 起算
    parser_->OnDcmiSample(MakeSample(3000, 0));
    ASSERT_EQ(1U, obj->QueueCount());
    PmSessionSample sample;
    ASSERT_TRUE(obj->GetQueueSample(0, sample));
    EXPECT_EQ(1000U, sample.startNs);
    EXPECT_EQ(3000U, sample.endNs);
}

/*
 * 测试目的：校验 Flush 清空窗口基准，之后的首个 tick 重新只作基准不产出
 */
TEST_F(PmSamplingParserUtest, FlushShouldDiscardBaseline)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    parser_->OnDcmiSample(MakeSample(1000, 0));
    parser_->Flush();
    parser_->OnDcmiSample(MakeSample(2000, 0));
    EXPECT_EQ(0U, obj->QueueCount());
}

TEST_F(PmSamplingParserUtest, AllScalarMetricsPreserveValuesAndNormalizeMissingFields)
{
    const std::vector<uint32_t> ids = {
        MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL,      MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL, MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL,   MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL,
        MSPTI_PM_SAMPLING_METRIC_ID_POWER,         MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE,
        MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ};
    auto obj = SetupSession(ids);
    PmDcmiSample current = MakeSample(1000, 1052);
    current.npuUtil = 11;
    current.aicoreUtil = 22;
    current.aivectorUtil = 33;
    current.aicubeUtil = 44;
    current.hbmBwUtil = 55;
    current.hbmMemUtil = 66;
    current.temperature = -5;  // Negative temperature is valid unless it is the -1 sentinel.
    current.aicoreFreq = 800;
    parser_->OnDcmiSample(current);
    current.tsNs = 2000;
    parser_->OnDcmiSample(current);
    PmSessionSample result;
    ASSERT_TRUE(obj->GetQueueSample(0, result));
    const double expected[]{11, 22, 33, 44, 55, 66, 105.2, -5, 800};
    ASSERT_EQ(result.entries.size(), ids.size());
    for (size_t i = 0; i < ids.size(); ++i)
    {
        EXPECT_EQ(result.entries[i].metricId, ids[i]);
        EXPECT_EQ(result.entries[i].instanceId, 0U);
        EXPECT_DOUBLE_EQ(result.entries[i].value, expected[i]);
    }
    current = MakeSample(3000, -1);
    parser_->OnDcmiSample(current);
    ASSERT_TRUE(obj->GetQueueSample(1, result));
    ASSERT_EQ(result.entries.size(), ids.size());
    for (const auto &entry : result.entries) EXPECT_DOUBLE_EQ(entry.value, 0.0);
}

TEST_F(PmSamplingParserUtest, PcieAndRoceDirectionsSelectFirstMatchingPoint)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW, MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW,
                             MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW});
    parser_->OnDcmiSample(MakeSample(1000, 0));
    auto sample = MakeSample(2000, 0);
    sample.bandwidths = {{PM_BW_HCCS_RX, 3, 999},
                         {PM_BW_PCIE_RX, 0, 11},
                         {PM_BW_ROCE_TX, 0, 22},
                         {PM_BW_ROCE_RX, 0, 33},
                         {PM_BW_ROCE_TX, 0, 999}};
    parser_->OnDcmiSample(sample);
    PmSessionSample result;
    ASSERT_TRUE(obj->GetQueueSample(0, result));
    ASSERT_EQ(result.entries.size(), 3U);
    EXPECT_DOUBLE_EQ(result.entries[0].value, 11);
    EXPECT_DOUBLE_EQ(result.entries[1].value, 22);
    EXPECT_DOUBLE_EQ(result.entries[2].value, 33);
}

TEST_F(PmSamplingParserUtest, EmptyOrUnknownMetricPlanDoesNotDeliverSamples)
{
    auto obj = SetupSession({MSPTI_PM_SAMPLING_METRIC_ID_POWER});
    for (const auto &ids : std::vector<std::vector<uint32_t>>{{}, {MSPTI_PM_SAMPLING_METRIC_ID_COUNT}})
    {
        PmSamplingParser parser(ids);
        parser.OnDcmiSample(MakeSample(1000, 10));
        parser.OnDcmiSample(MakeSample(2000, 20));
        EXPECT_EQ(obj->QueueCount(), 0U);
    }
}
}  // namespace
