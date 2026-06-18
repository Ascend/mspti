/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
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

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_api_parser.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti.h"

namespace
{
static int g_recordCnt = 0;

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    g_recordCnt += 1;
    if (buffer)
    {
        free(buffer);
    }
}

class CannApiParserUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    }
    virtual void TearDown()
    {
        // reset type info
        std::pair<uint16_t, uint32_t> key = {MSPROF_REPORT_ACL_LEVEL, 0};
        Mspti::Parser::CannHashCache::typeHashIdMap_.Erase(key);

        // reset record cnt;
        g_recordCnt = 0;
    }
};

TEST_F(CannApiParserUtest, ReportApiWillReturnSuccAndNotRecordApiWhenInputApiKindNotEnabled)
{
    MsprofApi apiData;
    apiData.level = MSPROF_REPORT_ACL_LEVEL;

    // MSPTI_ACTIVITY_KIND_ACL_API is not enabled
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);
}

TEST_F(CannApiParserUtest, ReportApiWillReturnSuccAndNotRecordApiWhenGetEmptyApiName)
{
    MsprofApi apiData;
    apiData.level = MSPROF_REPORT_ACL_LEVEL;

    // enable MSPTI_ACTIVITY_KIND_ACL_API
    Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_ACL_API);

    // GetTypeHashInfo will return empty string when not register type info yet
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);

    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_ACL_API);
}

TEST_F(CannApiParserUtest, ReportApiWillReturnSuccAndRecordApiWhenRecordDataSucc)
{
    MsprofApi apiData;
    apiData.level = MSPROF_REPORT_ACL_LEVEL;
    apiData.type = 0;

    // enable MSPTI_ACTIVITY_KIND_ACL_API
    Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_ACL_API);
    // register type info
    std::string apiName = "api";
    Mspti::Parser::CannHashCache::RegTypeHashInfo(MSPROF_REPORT_ACL_LEVEL, 0, apiName);
    std::string retTypeHashInfo = Mspti::Parser::CannHashCache::GetTypeHashInfo(MSPROF_REPORT_ACL_LEVEL, 0);
    EXPECT_STREQ(apiName.c_str(), retTypeHashInfo.c_str());
    // for coverage
    EXPECT_TRUE(Mspti::Parser::CannHashCache::GetTypeHashInfo(MSPROF_REPORT_ACL_LEVEL, 1).c_str());

    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    // consume api data
    msptiActivityFlushAll(1);

    EXPECT_EQ(1, g_recordCnt);
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_ACL_API);
}

TEST_F(CannApiParserUtest, ReportApiWillReturninnerErrWhenRecordDataFail)
{
    MsprofApi apiData;
    apiData.level = MSPROF_REPORT_ACL_LEVEL;
    apiData.type = 0;

    // enable MSPTI_ACTIVITY_KIND_ACL_API
    Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_ACL_API);
    // register type info
    std::string apiName = "api";
    Mspti::Parser::CannHashCache::RegTypeHashInfo(MSPROF_REPORT_ACL_LEVEL, 0, apiName);

    MOCKER_CPP(&Mspti::Activity::ActivityManager::Record).stubs().will(returnValue(MSPTI_ERROR_INNER));
    EXPECT_EQ(MSPTI_ERROR_INNER, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    // consume api data
    msptiActivityFlushAll(1);

    EXPECT_EQ(0, g_recordCnt);
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_ACL_API);
}

TEST_F(CannApiParserUtest, Level2ApiKindWillReturnRightKindWhenInputCorrespondingLevel)
{
    MsprofApi apiData;
    apiData.level = MSPROF_REPORT_ACL_LEVEL;

    // MSPTI_ACTIVITY_KIND_ACL_API is not enabled
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);

    apiData.level = MSPROF_REPORT_RUNTIME_LEVEL;
    // MSPTI_ACTIVITY_KIND_ACL_API is not enabled
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);

    apiData.level = MSPROF_REPORT_NODE_BASE_LEVEL;
    // MSPTI_ACTIVITY_KIND_ACL_API is not enabled
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);

    apiData.level = MSPROF_REPORT_ACL_LEVEL + 1;
    // MSPTI_ACTIVITY_KIND_ACL_API is not enabled
    EXPECT_EQ(MSPTI_SUCCESS, Mspti::Parser::CannApiParser::GetInstance().ReportApi(false, &apiData));
    EXPECT_EQ(0, g_recordCnt);
}
}  // namespace
