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

#include "csrc/pm_sampling/pm_sampling_decoder.h"
#include "gtest/gtest.h"
#include "securec.h"

using namespace Mspti::PmSampling;
namespace
{
class PmSamplingDecoderUtest : public testing::Test
{
   protected:
    // Odd metric count and two samples also exercise the 24-byte sample header.
    void SetUp() override
    {
        ASSERT_EQ(PmSamplingDecoder::GetSize(3, 4, 2, size), MSPTI_SUCCESS);
        ASSERT_EQ(size, 224U);
        ASSERT_EQ(PmSamplingDecoder::Initialize(image.data(), size, {0, 1, 12}, 4, 2), MSPTI_SUCCESS);
    }
    PmSessionSample Sample(uint64_t start = 10)
    {
        return {start, start + 10, {{0, 0, 37.5}, {1, 2, 123.25}, {1, 15, 456.5}, {12, 0, 900.0}}};
    }
    alignas(16) std::array<uint8_t, 240> image{};
    size_t size = 0;
};

TEST_F(PmSamplingDecoderUtest, WritesTheDocumentedLayoutWithoutOverrunningBuffer)
{
    image[size] = 0xA5;
    ASSERT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, {Sample(), Sample(20)}), MSPTI_SUCCESS);
    uint64_t start = 0, end = 0;
    ASSERT_EQ(PmSamplingDecoder::GetSampleInfo(image.data(), size, 1, start, end), MSPTI_SUCCESS);
    EXPECT_EQ(start, 20U);
    EXPECT_EQ(end, 30U);
    // Independent layout check: 32-byte header + 16-byte directory + 24-byte sample header + 8.
    double value = 0;
    ASSERT_EQ(memcpy_s(&value, sizeof(value), image.data() + 80, sizeof(value)), EOK);
    EXPECT_DOUBLE_EQ(value, 37.5);
    for (size_t i = 44; i < 48; ++i) EXPECT_EQ(image[i], 0U);
    EXPECT_EQ(image[size], 0xA5);
}

TEST_F(PmSamplingDecoderUtest, ReadsEntriesAndReplacesThePreviousBatch)
{
    ASSERT_EQ(PmSamplingDecoder::CheckPlan(image.data(), size, {0, 1, 12}, 4, 2), MSPTI_SUCCESS);
    const auto sample = Sample();
    ASSERT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, {sample}), MSPTI_SUCCESS);
    uint64_t total = 0, populated = 0, completed = 0;
    ASSERT_EQ(PmSamplingDecoder::GetInfo(image.data(), size, total, populated, completed), MSPTI_SUCCESS);
    EXPECT_EQ(total, 2U);
    EXPECT_EQ(populated, 1U);
    EXPECT_EQ(completed, 1U);
    uint32_t count = 0;
    ASSERT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, 0, 0, nullptr, count), MSPTI_SUCCESS);
    ASSERT_EQ(count, sample.entries.size());
    std::array<msptiPmSamplingSampleEntry, 4> entries{};
    entries[0].value = 97.0;
    count = 99;
    EXPECT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, 0, 3, entries.data(), count),
              MSPTI_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(count, 4U);
    EXPECT_DOUBLE_EQ(entries[0].value, 97.0);
    ASSERT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, 0, 4, entries.data(), count), MSPTI_SUCCESS);
    for (size_t i = 0; i < entries.size(); ++i)
    {
        EXPECT_EQ(entries[i].metricId, sample.entries[i].metricId);
        EXPECT_EQ(entries[i].instanceId, sample.entries[i].instanceId);
        EXPECT_DOUBLE_EQ(entries[i].value, sample.entries[i].value);
    }
    ASSERT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, {}), MSPTI_SUCCESS);
    ASSERT_EQ(PmSamplingDecoder::GetInfo(image.data(), size, total, populated, completed), MSPTI_SUCCESS);
    EXPECT_EQ(total, 2U);
    EXPECT_EQ(populated, 0U);
    EXPECT_EQ(completed, 0U);
}

TEST_F(PmSamplingDecoderUtest, RejectsInvalidMetricPlansWithoutChangingImage)
{
    const auto before = image;
    for (const auto &ids : std::vector<std::vector<uint32_t>>{{0, 0, 12}, {0, 1, MSPTI_PM_SAMPLING_METRIC_ID_COUNT}})
    {
        EXPECT_EQ(PmSamplingDecoder::Initialize(image.data(), size, ids, 4, 2), MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC);
        EXPECT_EQ(image, before);
    }
}

TEST_F(PmSamplingDecoderUtest, RejectsInvalidBatchesAndKeepsPublishedSamplesReadable)
{
    ASSERT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, {Sample()}), MSPTI_SUCCESS);
    const auto before = image;
    auto tooManyEntries = Sample();
    tooManyEntries.entries.push_back({0, 3, 1.0});
    auto unknownMetric = Sample();
    unknownMetric.entries[0].metricId = 2;  // Valid ID, but not configured in this image.
    auto reversedTime = Sample();
    reversedTime.endNs = reversedTime.startNs - 1;
    const std::vector<std::vector<PmSessionSample>> batches = {
        {tooManyEntries}, {unknownMetric}, {reversedTime}, {Sample(), Sample(), Sample()}};
    const std::array<msptiResult, 4> errors = {MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW,
                                               MSPTI_ERROR_PM_SAMPLING_INVALID_METRIC, MSPTI_ERROR_INVALID_PARAMETER,
                                               MSPTI_ERROR_PM_SAMPLING_BUFFER_OVERFLOW};
    for (size_t i = 0; i < batches.size(); ++i)
    {
        EXPECT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, batches[i]), errors[i]);
        EXPECT_EQ(image, before);
        uint64_t start = 0, end = 0;
        ASSERT_EQ(PmSamplingDecoder::GetSampleInfo(image.data(), size, 0, start, end), MSPTI_SUCCESS);
        EXPECT_EQ(start, 10U);
        EXPECT_EQ(end, 20U);
        std::array<msptiPmSamplingSampleEntry, 4> entries{};
        uint32_t count = 0;
        ASSERT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, 0, 4, entries.data(), count), MSPTI_SUCCESS);
        EXPECT_EQ(count, 4U);
        EXPECT_DOUBLE_EQ(entries[0].value, 37.5);
    }
}

TEST_F(PmSamplingDecoderUtest, FailedLookupsLeaveOutputCountsAndTimestampsUnchanged)
{
    ASSERT_EQ(PmSamplingDecoder::WriteSamples(image.data(), size, {Sample()}), MSPTI_SUCCESS);
    for (uint64_t index : {uint64_t{1}, uint64_t{2}, UINT64_MAX})
    {
        uint64_t start = 71, end = 73;
        uint32_t count = 97;
        EXPECT_EQ(PmSamplingDecoder::GetSampleInfo(image.data(), size, index, start, end),
                  MSPTI_ERROR_INVALID_PARAMETER);
        EXPECT_EQ(start, 71U);
        EXPECT_EQ(end, 73U);
        EXPECT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, index, 0, nullptr, count),
                  MSPTI_ERROR_INVALID_PARAMETER);
        EXPECT_EQ(count, 97U);
    }
    image[0] ^= 0xFF;  // Invalid image magic.
    uint32_t count = 97;
    EXPECT_EQ(PmSamplingDecoder::GetSampleEntries(image.data(), size, 0, 0, nullptr, count),
              MSPTI_ERROR_INVALID_PARAMETER);
    EXPECT_EQ(count, 97U);
}

}  // namespace
