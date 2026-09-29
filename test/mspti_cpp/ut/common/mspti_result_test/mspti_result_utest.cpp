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
#include <cstring>

#include "csrc/include/mspti_result.h"
#include "gtest/gtest.h"

namespace
{
class MsptiResultUtest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

/*
 * 测试目的：校验连续码位（0..9）逐一返回可读描述且函数返回 MSPTI_SUCCESS
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnMappedStringWhenInputContinuousCode)
{
    for (int32_t i = 0; i <= MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT; i++)
    {
        const char *str = nullptr;
        auto ret = msptiGetResultString(static_cast<msptiResult>(i), &str);
        ASSERT_EQ(ret, MSPTI_SUCCESS);
        ASSERT_NE(str, nullptr);
        ASSERT_GT(strlen(str), 0U);
    }
}

/*
 * 测试目的：校验各错误码与映射表的描述文本一致（抽检）
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnExactTextWhenInputKnownCode)
{
    const char *str = nullptr;
    ASSERT_EQ(msptiGetResultString(MSPTI_SUCCESS, &str), MSPTI_SUCCESS);
    EXPECT_STREQ(str, "Success.");

    ASSERT_EQ(msptiGetResultString(MSPTI_ERROR_INVALID_PARAMETER, &str), MSPTI_SUCCESS);
    EXPECT_STREQ(str, "One or more of the parameters is invalid.");

    ASSERT_EQ(msptiGetResultString(MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED, &str), MSPTI_SUCCESS);
    EXPECT_STREQ(str, "Multiple subscribers are not supported.");

    ASSERT_EQ(msptiGetResultString(MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT, &str), MSPTI_SUCCESS);
    EXPECT_STREQ(str, "The parameter size is not sufficient.");
}

/*
 * 测试目的：校验 MSPTI_ERROR_INNER(999) 码位不连续时的特判返回
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnInnerStringWhenInputInnerCode)
{
    const char *str = nullptr;
    auto ret = msptiGetResultString(MSPTI_ERROR_INNER, &str);
    ASSERT_EQ(ret, MSPTI_SUCCESS);
    ASSERT_NE(str, nullptr);
    EXPECT_STREQ(str, "An unknown internal error has occurred.");
}

/*
 * 测试目的：校验 str 为 NULL 时返回 MSPTI_ERROR_INVALID_PARAMETER
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnInvalidParameterWhenStrIsNull)
{
    auto ret = msptiGetResultString(MSPTI_SUCCESS, nullptr);
    ASSERT_EQ(ret, MSPTI_ERROR_INVALID_PARAMETER);
}

/*
 * 测试目的：校验未知码位返回 MSPTI_ERROR_INVALID_PARAMETER
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnInvalidParameterWhenInputUnknownCode)
{
    const char *str = nullptr;
    auto ret = msptiGetResultString(static_cast<msptiResult>(100), &str);
    ASSERT_EQ(ret, MSPTI_ERROR_INVALID_PARAMETER);
}

/*
 * 测试目的：校验负数码位返回 MSPTI_ERROR_INVALID_PARAMETER
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnInvalidParameterWhenInputNegativeCode)
{
    const char *str = nullptr;
    auto ret = msptiGetResultString(static_cast<msptiResult>(-1), &str);
    ASSERT_EQ(ret, MSPTI_ERROR_INVALID_PARAMETER);
}

/*
 * 测试目的：校验 MSPTI_ERROR_FORCE_INT 按非法码位处理
 */
TEST_F(MsptiResultUtest, GetResultStringShouldReturnInvalidParameterWhenInputForceInt)
{
    const char *str = nullptr;
    auto ret = msptiGetResultString(MSPTI_ERROR_FORCE_INT, &str);
    ASSERT_EQ(ret, MSPTI_ERROR_INVALID_PARAMETER);
}

/*
 * 测试目的：校验失败路径不修改 *str（失败仅返回 INVALID_PARAMETER）
 */
TEST_F(MsptiResultUtest, GetResultStringShouldNotTouchStrWhenInputInvalid)
{
    const char *sentinel = "sentinel";
    const char *str = sentinel;

    ASSERT_EQ(msptiGetResultString(static_cast<msptiResult>(100), &str), MSPTI_ERROR_INVALID_PARAMETER);
    ASSERT_EQ(str, sentinel);

    ASSERT_EQ(msptiGetResultString(MSPTI_ERROR_FORCE_INT, &str), MSPTI_ERROR_INVALID_PARAMETER);
    ASSERT_EQ(str, sentinel);
}
}  // namespace
