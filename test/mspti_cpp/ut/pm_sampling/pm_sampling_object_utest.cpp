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
#include "csrc/pm_sampling/pm_sampling_object.h"
#include "gtest/gtest.h"

namespace
{
using namespace Mspti;
using namespace Mspti::PmSampling;

// 单 power 样本字节 = 24 固定 + 1*16 entry = 40，用于精确构造队列容量
constexpr size_t kPowerSampleBytes = 40;
constexpr uint64_t kValidIntervalNs = 100000000ULL;  // 100ms
// 契约间隔边界：[1ms, 1s] 闭区间合法
constexpr uint64_t kMinIntervalNs = 1000000ULL;
constexpr uint64_t kMaxIntervalNs = 1000000000ULL;

class PmSamplingObjectUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}

    static msptiResult Config(
        PmSamplingObject &obj, const std::vector<uint32_t> &ids, uint64_t intervalNs, size_t bufferSize = 0,
        msptiPmSamplingBufferAppendMode mode = MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST)
    {
        return obj.SetConfig(static_cast<uint32_t>(ids.size()), ids.data(), intervalNs,
                             MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, bufferSize, mode);
    }

    static PmSessionSample MakeSample(uint64_t startNs, uint64_t endNs)
    {
        PmSessionSample sample;
        sample.startNs = startNs;
        sample.endNs = endNs;
        sample.entries.push_back({MSPTI_PM_SAMPLING_METRIC_ID_POWER, 0, 1.0});
        return sample;
    }
};

/*
 * 测试目的：校验合法配置成功且回读一致；顺带校验非 Running 状态入队被丢弃
 */
TEST_F(PmSamplingObjectUtest, SetConfigShouldStoreConfigWhenInputValid)
{
    PmSamplingObject obj(0);
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE};
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, ids, kValidIntervalNs));
    EXPECT_EQ(ids, obj.MetricIds());
    EXPECT_EQ(kValidIntervalNs, obj.IntervalNs());
    EXPECT_EQ(PmSamplingObject::State::Configured, obj.GetState());

    obj.EnqueueSample(MakeSample(1, 2));
    EXPECT_EQ(0U, obj.QueueCount());
}

/*
 * 测试目的：校验 SetConfig 各维度非法入参的拒绝：numMetrics/空指针/间隔越界/模式非法/
 *           bufferSize 越界/metricId 越界（专用错误码）
 */
TEST_F(PmSamplingObjectUtest, SetConfigShouldRejectWhenParamsInvalid)
{
    PmSamplingObject obj(0);
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};

    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              obj.SetConfig(0, ids.data(), kValidIntervalNs, MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, 0,
                            MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST));
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              obj.SetConfig(1, nullptr, kValidIntervalNs, MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, 0,
                            MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST));
    // 间隔边界 [1ms, 1s] 开区间外拒绝
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER, Config(obj, ids, kMinIntervalNs - 1));
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER, Config(obj, ids, kMaxIntervalNs + 1));
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              obj.SetConfig(1, ids.data(), kValidIntervalNs, static_cast<msptiPmSamplingTriggerMode>(99), 0,
                            MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST));
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              obj.SetConfig(1, ids.data(), kValidIntervalNs, MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL, 0,
                            static_cast<msptiPmSamplingBufferAppendMode>(99)));
    // bufferSize 小于单样本字节、超过 512MB 上界
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER, Config(obj, ids, kValidIntervalNs, kPowerSampleBytes - 1));
    ASSERT_EQ(MSPTI_ERROR_INVALID_PARAMETER, Config(obj, ids, kValidIntervalNs, 513ULL * 1024 * 1024));
    // 越界 metricId 返回专用错误码
    const std::vector<uint32_t> badIds = {MSPTI_PM_SAMPLING_METRIC_ID_COUNT};
    ASSERT_EQ(MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC, Config(obj, badIds, kValidIntervalNs));
}

/*
 * 测试目的：校验状态机流转：Created 直接 Start 拒绝、Configured/Stopped 可 Start、
 *           Running 可 Stop、Stopped 重复 Stop 拒绝、Running 重配拒绝
 */
TEST_F(PmSamplingObjectUtest, StartStopShouldFollowStateMachineWhenCalledInOrder)
{
    PmSamplingObject obj(0);
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};

    EXPECT_EQ(MSPTI_ERROR_INVALID_OPERATION, obj.Start());
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, ids, kValidIntervalNs));
    ASSERT_EQ(MSPTI_SUCCESS, obj.Start());
    EXPECT_EQ(PmSamplingObject::State::Running, obj.GetState());
    ASSERT_EQ(MSPTI_SUCCESS, obj.Stop());
    EXPECT_EQ(PmSamplingObject::State::Stopped, obj.GetState());
    EXPECT_EQ(MSPTI_ERROR_INVALID_OPERATION, obj.Stop());
    ASSERT_EQ(MSPTI_SUCCESS, obj.Start());
    EXPECT_TRUE(obj.IsRunning());
    EXPECT_EQ(MSPTI_ERROR_INVALID_OPERATION, Config(obj, ids, kValidIntervalNs));
}

/*
 * 测试目的：校验队列满时的两种溢出策略：KEEP_OLDEST 丢新保旧、KEEP_LATEST 弹旧保新，
 *           且重配置清空旧队列、溢出标志读取即清除
 */
TEST_F(PmSamplingObjectUtest, EnqueueSampleShouldApplyOverflowPolicyWhenQueueFull)
{
    PmSamplingObject obj(0);
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    // 40 字节恰好容纳 2 个 power 样本（容量 = size/40 + 1）
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, ids, kValidIntervalNs, kPowerSampleBytes));
    ASSERT_EQ(MSPTI_SUCCESS, obj.Start());

    // KEEP_OLDEST：满后丢新保旧
    obj.EnqueueSample(MakeSample(1, 2));
    obj.EnqueueSample(MakeSample(3, 4));
    obj.EnqueueSample(MakeSample(5, 6));
    EXPECT_EQ(2U, obj.QueueCount());
    PmSessionSample sample;
    ASSERT_TRUE(obj.GetQueueSample(0, sample));
    EXPECT_EQ(1U, sample.startNs);
    ASSERT_TRUE(obj.GetQueueSample(1, sample));
    EXPECT_EQ(3U, sample.startNs);
    EXPECT_TRUE(obj.ConsumeOverflow());

    // KEEP_LATEST：满后弹旧保新（重配同时清空旧队列）
    ASSERT_EQ(MSPTI_SUCCESS, obj.Stop());
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, ids, kValidIntervalNs, kPowerSampleBytes,
                                    MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_LATEST));
    EXPECT_EQ(0U, obj.QueueCount());
    ASSERT_EQ(MSPTI_SUCCESS, obj.Start());
    obj.EnqueueSample(MakeSample(1, 2));
    obj.EnqueueSample(MakeSample(3, 4));
    obj.EnqueueSample(MakeSample(5, 6));
    EXPECT_EQ(2U, obj.QueueCount());
    ASSERT_TRUE(obj.GetQueueSample(0, sample));
    EXPECT_EQ(3U, sample.startNs);
    ASSERT_TRUE(obj.GetQueueSample(1, sample));
    EXPECT_EQ(5U, sample.startNs);
    EXPECT_TRUE(obj.ConsumeOverflow());
}

/*
 * 测试目的：校验队列读取语义对照：直读不消费、越界返回 false，Drain 消费式取走即删且保序
 */
TEST_F(PmSamplingObjectUtest, QueueReadAndDrainShouldFollowConsumeSemanticsWhenCalled)
{
    PmSamplingObject obj(0);
    const std::vector<uint32_t> ids = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, ids, kValidIntervalNs));
    ASSERT_EQ(MSPTI_SUCCESS, obj.Start());
    obj.EnqueueSample(MakeSample(1, 2));
    obj.EnqueueSample(MakeSample(3, 4));
    obj.EnqueueSample(MakeSample(5, 6));

    // 直读不消费
    PmSessionSample sample;
    ASSERT_TRUE(obj.GetQueueSample(0, sample));
    EXPECT_EQ(1U, sample.startNs);
    EXPECT_EQ(3U, obj.QueueCount());
    EXPECT_FALSE(obj.GetQueueSample(3, sample));

    // Drain 消费式：取走即删且保序，请求超过剩余数量时取空队列返回实际数
    std::vector<PmSessionSample> out;
    ASSERT_EQ(2U, obj.DrainSamples(2, out));
    ASSERT_EQ(2U, out.size());
    EXPECT_EQ(1U, out[0].startNs);
    EXPECT_EQ(3U, out[1].startNs);
    EXPECT_EQ(1U, obj.QueueCount());
    out.clear();
    ASSERT_EQ(1U, obj.DrainSamples(5, out));
    ASSERT_EQ(1U, out.size());
    EXPECT_EQ(5U, out[0].startNs);
    EXPECT_EQ(0U, obj.QueueCount());
}

/*
 * 测试目的：校验 WorstEntriesPerSample 对 HCCS 指标按 16 链路计、其余按 1 计
 */
TEST_F(PmSamplingObjectUtest, WorstEntriesPerSampleShouldCountHccsAsSixteenLinks)
{
    PmSamplingObject obj(0);
    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, {MSPTI_PM_SAMPLING_METRIC_ID_POWER}, kValidIntervalNs));
    EXPECT_EQ(1U, obj.WorstEntriesPerSample());

    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, {MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW}, kValidIntervalNs));
    EXPECT_EQ(16U, obj.WorstEntriesPerSample());

    ASSERT_EQ(MSPTI_SUCCESS, Config(obj, {MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW},
                                    kValidIntervalNs));
    EXPECT_EQ(17U, obj.WorstEntriesPerSample());
}

/*
 * 测试目的：校验镜像计划未设置返回 false，设置后原样取回，再次设置覆盖
 */
TEST_F(PmSamplingObjectUtest, ImageInfoShouldRoundTripWhenSetAndOverwritten)
{
    PmSamplingObject obj(0);
    PmSamplingObject::ImageInfo info;
    EXPECT_FALSE(obj.GetImageInfo(info));

    info.metricIds = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    info.maxEntriesPerSample = 1;
    info.maxSamples = 100;
    info.imageSize = 1040;
    obj.SetImageInfo(info);

    PmSamplingObject::ImageInfo got;
    ASSERT_TRUE(obj.GetImageInfo(got));
    EXPECT_EQ(info.metricIds, got.metricIds);
    EXPECT_EQ(1U, got.maxEntriesPerSample);
    EXPECT_EQ(100U, got.maxSamples);
    EXPECT_EQ(1040U, got.imageSize);

    PmSamplingObject::ImageInfo info2 = info;
    info2.maxSamples = 200;
    obj.SetImageInfo(info2);
    ASSERT_TRUE(obj.GetImageInfo(got));
    EXPECT_EQ(200U, got.maxSamples);
}
}  // namespace
