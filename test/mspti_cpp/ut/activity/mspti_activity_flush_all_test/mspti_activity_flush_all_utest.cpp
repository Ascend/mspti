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
#include <iostream>

#include "gtest/gtest.h"
#include "mspti.h"

class MsptiActivityFlushAllTest : public testing::Test
{
   protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

TEST_F(MsptiActivityFlushAllTest, ExistsAndRuns)
{
    msptiResult res0 = msptiActivityFlushAll(0);
    std::cout << "msptiActivityFlushAll(0) returned: " << res0 << std::endl;

    msptiResult res1 = msptiActivityFlushAll(1);
    std::cout << "msptiActivityFlushAll(1) returned: " << res1 << std::endl;

    msptiResult res99 = msptiActivityFlushAll(99);
    std::cout << "msptiActivityFlushAll(99) returned: " << res99 << std::endl;

    std::cout << "Signature (uint32_t) confirmed. Call without crash." << std::endl;
}
