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

#include <array>
#include <new>

#include "csrc/common/context_manager.h"
#include "csrc/pm_sampling/pm_sampling_decoder.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace Mspti::PmSampling;
namespace
{
class PmSamplingImageApiUtest : public testing::Test
{
   protected:
    void SetUp() override
    {
        PmSamplingManager::GetInstance()->objects_[0] =
            std::shared_ptr<PmSamplingObject>(&object, [](PmSamplingObject *) {});
        PmSamplingManager::GetInstance()->ptrIndex_[&object] = {0, true};
        object.metricIds_ = {MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW,
                             MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ};
        object.state_ = PmSamplingObject::State::Running;
        ASSERT_EQ(msptiPmSamplingGetCounterDataSize(&sizeParams), MSPTI_SUCCESS);
        ASSERT_EQ(size, image.size());
        ASSERT_EQ(msptiPmSamplingCounterDataImageInitialize(&init), MSPTI_SUCCESS);
    }
    void TearDown() override
    {
        GlobalMockObject::verify();
        PmSamplingManager::GetInstance()->objects_.clear();
        PmSamplingManager::GetInstance()->ptrIndex_.clear();
    }
    PmSessionSample Sample(uint64_t start)
    {
        return {start, start + 10, {{ids[0], 0, 0.0}, {ids[1], 2, 123.25}, {ids[1], 15, 456.5}, {ids[2], 0, 900.0}}};
    }
    PmSamplingObject object{0};
    msptiPmSamplingObjectPtr handle = reinterpret_cast<msptiPmSamplingObjectPtr>(&object);
    uint32_t ids[3]{MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL, MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW,
                    MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ};
    alignas(16) std::array<uint8_t, 672> image{};
    size_t size = 0;
    uint64_t total = 0, populated = 0, completed = 0, start = 0, end = 0;
    uint32_t count = 0;
    bool overflow = false;
    msptiPmSamplingDecodeStopReason reason = MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER;
    msptiPmSamplingGetCounterDataSizeParams sizeParams{sizeof(sizeParams), nullptr, handle, 3, ids, 2, &size};
    msptiPmSamplingCounterDataImageInitializeParams init{sizeof(init), nullptr, handle, image.size(), image.data()};
    msptiPmSamplingDecodeDataParams decode{sizeof(decode), nullptr,   handle, image.data(),
                                           image.size(),   &overflow, &reason};
    msptiPmSamplingGetCounterDataInfoParams info{sizeof(info), nullptr,    handle,      &total,
                                                 &populated,   &completed, image.data()};
    msptiPmSamplingGetSampleInfoParams sampleInfo{sizeof(sampleInfo), nullptr, handle, image.data(), 0, &start, &end};
    msptiPmSamplingGetSampleEntriesParams entries{sizeof(entries), nullptr, handle, image.data(), 0, 0,
                                                  nullptr,         &count};
};

TEST_F(PmSamplingImageApiUtest, QueueAndImageQueriesReturnTheSameSample)
{
    const auto expected = Sample(10);
    object.queue_.push_back(expected);
    for (const uint8_t *buffer : std::array<const uint8_t *, 2>{{nullptr, image.data()}})
    {
        if (buffer != nullptr) ASSERT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_SUCCESS);
        info.pCounterDataImage = sampleInfo.pCounterDataImage = entries.pCounterDataImage = buffer;
        ASSERT_EQ(msptiPmSamplingGetCounterDataInfo(&info), MSPTI_SUCCESS);
        EXPECT_EQ(total, buffer == nullptr ? 1U : 2U);
        EXPECT_EQ(populated, 1U);
        EXPECT_EQ(completed, 1U);
        ASSERT_EQ(msptiPmSamplingGetSampleInfo(&sampleInfo), MSPTI_SUCCESS);
        EXPECT_EQ(start, 10U);
        EXPECT_EQ(end, 20U);
        entries.pEntries = nullptr;
        ASSERT_EQ(msptiPmSamplingGetSampleEntries(&entries), MSPTI_SUCCESS);
        ASSERT_EQ(count, 4U);
        std::array<msptiPmSamplingSampleEntry, 4> output{};
        output[0].value = 97.0;
        entries.pEntries = output.data();
        entries.maxEntries = 1;
        EXPECT_EQ(msptiPmSamplingGetSampleEntries(&entries), MSPTI_ERROR_INVALID_PARAMETER);
        EXPECT_EQ(count, 4U);
        EXPECT_DOUBLE_EQ(output[0].value, 97.0);
        entries.maxEntries = output.size();
        ASSERT_EQ(msptiPmSamplingGetSampleEntries(&entries), MSPTI_SUCCESS);
        for (size_t i = 0; i < output.size(); ++i)
        {
            EXPECT_EQ(output[i].metricId, expected.entries[i].metricId);
            EXPECT_EQ(output[i].instanceId, expected.entries[i].instanceId);
            EXPECT_DOUBLE_EQ(output[i].value, expected.entries[i].value);
        }
        EXPECT_EQ(object.QueueCount(), buffer == nullptr ? 1U : 0U);
    }
}

TEST_F(PmSamplingImageApiUtest, DecodeConsumesBatchesAndClearsOverflowOnlyOnce)
{
    object.state_ = PmSamplingObject::State::Stopped;
    object.queue_ = {Sample(10), Sample(20), Sample(30)};
    object.overflowed_ = true;
    const uint64_t counts[]{2, 1, 0}, starts[]{10, 30, 0}, remaining[]{1, 0, 0};
    for (size_t i = 0; i < 3; ++i)
    {
        ASSERT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_SUCCESS);
        ASSERT_EQ(msptiPmSamplingGetCounterDataInfo(&info), MSPTI_SUCCESS);
        EXPECT_EQ(populated, counts[i]);
        EXPECT_EQ(completed, counts[i]);
        EXPECT_EQ(object.QueueCount(), remaining[i]);
        EXPECT_EQ(overflow, i == 0);
        EXPECT_EQ(reason, i == 0 ? MSPTI_PM_SAMPLING_DECODE_STOP_REASON_COUNTER_DATA_FULL
                                 : MSPTI_PM_SAMPLING_DECODE_STOP_REASON_END_OF_RECORDS);
        if (counts[i] != 0)
        {
            ASSERT_EQ(msptiPmSamplingGetSampleInfo(&sampleInfo), MSPTI_SUCCESS);
            EXPECT_EQ(start, starts[i]);
        }
    }
    for (size_t i = SampleAreaOffset(3); i < image.size(); ++i) EXPECT_EQ(image[i], 0U);
}

TEST_F(PmSamplingImageApiUtest, UninitializedOrWrongSizeImageDoesNotConsumeSamples)
{
    object.queue_.push_back(Sample(10));
    object.overflowed_ = true;
    decode.counterDataImageSize--;
    EXPECT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_ERROR_INVALID_PARAMETER);
    decode.counterDataImageSize++;
    image.fill(0);  // Not initialized.
    EXPECT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(object.QueueCount(), 1U);
    EXPECT_TRUE(object.overflowed_);
}

TEST_F(PmSamplingImageApiUtest, InternalImageErrorIsPropagatedWithoutConsumingOrChangingOutputs)
{
    object.queue_.push_back(Sample(10));
    MOCKER(PmSamplingDecoder::CheckPlan).stubs().will(returnValue(MSPTI_ERROR_INNER));
    count = 97;
    EXPECT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_ERROR_INNER);
    EXPECT_EQ(object.QueueCount(), 1U);
    EXPECT_FALSE(overflow);
    EXPECT_EQ(reason, MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER);
    EXPECT_EQ(msptiPmSamplingGetSampleEntries(&entries), MSPTI_ERROR_INNER);
    EXPECT_EQ(count, 97U);
}

TEST_F(PmSamplingImageApiUtest, MetricQueriesReturnPropertiesAndAvailability)
{
    uint32_t numMetrics = 0;
    msptiPmSamplingGetNumMetricsParams num{sizeof(num), nullptr, 0, &numMetrics};
    ASSERT_EQ(msptiPmSamplingGetNumMetrics(&num), MSPTI_SUCCESS);
    EXPECT_EQ(numMetrics, MSPTI_PM_SAMPLING_METRIC_ID_COUNT);
    msptiPmSamplingMetricProperties property{};
    msptiPmSamplingGetMetricPropertiesParams props{sizeof(props), nullptr, 0, MSPTI_PM_SAMPLING_METRIC_ID_POWER,
                                                   &property};
    ASSERT_EQ(msptiPmSamplingGetMetricProperties(&props), MSPTI_SUCCESS);
    EXPECT_STREQ(property.unit, "W");
    EXPECT_TRUE(property.available);
    const uint32_t metricIds[]{MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ};
    uint8_t available[]{9, 9};
    msptiPmSamplingGetCounterAvailabilityParams availability{sizeof(availability), nullptr, 0, 2, nullptr, available};
    EXPECT_EQ(msptiPmSamplingGetCounterAvailability(&availability), MSPTI_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(available[0], 9U);
    availability.pMetricIds = metricIds;
    ASSERT_EQ(msptiPmSamplingGetCounterAvailability(&availability), MSPTI_SUCCESS);
    EXPECT_EQ(available[0], 1U);
    EXPECT_EQ(available[1], 1U);
}

TEST_F(PmSamplingImageApiUtest, InvalidConsumedBatchIsNotRestoredOnDecodeFailure)
{
    auto badTime = Sample(10), badMetric = Sample(20), tooManyEntries = Sample(30);
    badTime.startNs = badTime.endNs + 1;
    badMetric.entries[0].metricId = MSPTI_PM_SAMPLING_METRIC_ID_COUNT;
    tooManyEntries.entries.resize(object.WorstEntriesPerSample() + 1);
    const msptiResult errors[]{MSPTI_ERROR_INVALID_PARAMETER, MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC,
                               MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW};
    const PmSessionSample cases[]{badTime, badMetric, tooManyEntries};
    for (size_t i = 0; i < 3; ++i)
    {
        object.queue_ = {Sample(1), cases[i]};
        object.overflowed_ = true;
        overflow = false;
        reason = MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER;
        EXPECT_EQ(msptiPmSamplingDecodeData(&decode), errors[i]);
        EXPECT_EQ(object.QueueCount(), 0U);
        EXPECT_FALSE(overflow);
        EXPECT_EQ(reason, MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER);
        EXPECT_TRUE(object.overflowed_);
        ASSERT_EQ(msptiPmSamplingDecodeData(&decode), MSPTI_SUCCESS);
        ASSERT_EQ(msptiPmSamplingGetCounterDataInfo(&info), MSPTI_SUCCESS);
        EXPECT_EQ(completed, 0U);  // Retrying cannot recover the consumed batch.
    }
}

TEST_F(PmSamplingImageApiUtest, UnregisteredHandleAndNullParamsAreRejected)
{
    msptiPmSamplingEnableParams enable{};
    msptiPmSamplingSetConfigParams config{};
    msptiPmSamplingStartParams startParams{};
    msptiPmSamplingStopParams stop{};
    msptiPmSamplingDisableParams disable{};
    enable.structSize = sizeof(enable);
    config.structSize = sizeof(config);
    startParams.structSize = sizeof(startParams);
    stop.structSize = sizeof(stop);
    disable.structSize = sizeof(disable);
    config.pPmSamplingObject = startParams.pPmSamplingObject = stop.pPmSamplingObject = disable.pPmSamplingObject =
        reinterpret_cast<msptiPmSamplingObjectPtr>(0x8);
    EXPECT_EQ(msptiPmSamplingEnable(nullptr), MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT);
    EXPECT_EQ(msptiPmSamplingSetConfig(&config), MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED);
    EXPECT_EQ(msptiPmSamplingStart(&startParams), MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED);
    EXPECT_EQ(msptiPmSamplingStop(&stop), MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED);
    EXPECT_EQ(msptiPmSamplingDisable(&disable), MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED);
}

}  // namespace
