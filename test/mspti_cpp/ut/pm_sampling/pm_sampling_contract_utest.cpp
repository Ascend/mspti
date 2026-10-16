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
#include "csrc/pm_sampling/pm_sampling_contract.h"
#include "gtest/gtest.h"

namespace
{
using namespace Mspti::PmSampling;

class PmSamplingContractUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/*
 * 测试目的：校验镜像布局常量与契约文档一致（magic/version/header/entry 尺寸）
 */
TEST_F(PmSamplingContractUtest, LayoutConstantsShouldMatchContractWhenCompiled)
{
    ASSERT_EQ(0x50534D50U, PM_IMAGE_MAGIC);
    ASSERT_EQ(2U, PM_IMAGE_VERSION);
    ASSERT_EQ(32U, PM_IMAGE_HEADER_SIZE);
    ASSERT_EQ(24U, PM_SAMPLE_FIXED_SIZE);
    ASSERT_EQ(16U, PM_SAMPLE_ENTRY_SIZE);
}

/*
 * 测试目的：校验样本区偏移 = 32 字节头 + metricId 区向上对齐到 16（覆盖 Align16 非 16 倍数与已对齐两种路径）
 */
TEST_F(PmSamplingContractUtest, SampleAreaOffsetShouldBeHeaderPlusAlignedMetricArea)
{
    // 15 个指标：15*4=60 对齐到 64，偏移 = 32 + 64
    ASSERT_EQ(96U, SampleAreaOffset(15));
    // 16 个指标：16*4=64 已对齐，偏移 = 32 + 64
    ASSERT_EQ(96U, SampleAreaOffset(16));
    // 1 个指标：4 对齐到 16，偏移 = 32 + 16
    ASSERT_EQ(48U, SampleAreaOffset(1));
}

/*
 * 测试目的：校验镜像总大小 = 样本区偏移 + maxSamples * stride（隐含覆盖 stride = 24 + 槽位数 * 16）
 */
TEST_F(PmSamplingContractUtest, CalcImageSizeShouldBeOffsetPlusSampleArea)
{
    // 1 指标、1 entry 槽、1 样本：48 + 1*40
    ASSERT_EQ(88U, CalcImageSize(1, 1, 1));
    // 15 指标、16 槽、100 样本：96 + 100*280
    ASSERT_EQ(96U + 100U * 280U, CalcImageSize(15, 16, 100));
}

/*
 * 测试目的：校验合法入参通过校验（含恰好等于 512MB 预算上界的边界值）
 */
TEST_F(PmSamplingContractUtest, ValidateImageParamsShouldPassWhenInputValid)
{
    ASSERT_TRUE(ValidateImageParams(15, 16, 100));
    ASSERT_TRUE(ValidateImageParams(1, 1, 1));
    ASSERT_TRUE(ValidateImageParams(MSPTI_PM_SAMPLING_METRIC_ID_COUNT, PM_IMAGE_MAX_ENTRIES_PER_SAMPLE, 1));
    // 恰好等于预算上界（向下取整的商）应通过
    const uint64_t boundary = (PM_IMAGE_MAX_SIZE - SampleAreaOffset(1)) / SampleStride(1);
    ASSERT_TRUE(ValidateImageParams(1, 1, boundary));
}

/*
 * 测试目的：校验 numMetrics/maxEntriesPerSample/maxSamples 各维度非法入参的拒绝
 */
TEST_F(PmSamplingContractUtest, ValidateImageParamsShouldRejectWhenParamsInvalid)
{
    ASSERT_FALSE(ValidateImageParams(0, 1, 1));
    ASSERT_FALSE(ValidateImageParams(MSPTI_PM_SAMPLING_METRIC_ID_COUNT + 1, 1, 1));
    ASSERT_FALSE(ValidateImageParams(1, 0, 1));
    ASSERT_FALSE(ValidateImageParams(1, PM_IMAGE_MAX_ENTRIES_PER_SAMPLE + 1, 1));
    ASSERT_FALSE(ValidateImageParams(1, 1, 0));
    // 超出 512MB 预算的样本数必拒
    const uint64_t oversize = PM_IMAGE_MAX_SIZE / SampleStride(1) * 2;
    ASSERT_FALSE(ValidateImageParams(1, 1, oversize));
}
}  // namespace
