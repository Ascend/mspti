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
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti_activity.h"
#include "securec.h"

namespace
{

static int g_recordCnt = 0;
static msptiActivityMarker g_mstxRecords[100];

void UserBufferRequest(uint8_t** buffer, size_t* size, size_t* maxNumRecords)
{
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *buffer = static_cast<uint8_t*>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void UserBufferComplete(uint8_t* buffer, size_t size, size_t validSize)
{
    if (validSize > 0)
    {
        msptiActivity* pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do
        {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS)
            {
                if (pRecord->kind == MSPTI_ACTIVITY_KIND_MARKER)
                {
                    msptiActivityMarker* activity = reinterpret_cast<msptiActivityMarker*>(pRecord);
                    (void)memcpy_s(&g_mstxRecords[g_recordCnt], sizeof(msptiActivityMarker), activity,
                                   sizeof(msptiActivityMarker));
                    g_recordCnt++;
                }
            }
            else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
        } while (true);
    }
    if (buffer)
    {
        free(buffer);
    }
}

class MstxParserUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_MARKER);
        msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete);
    }
    virtual void TearDown()
    {
        // clear all data
        msptiActivityFlushAll(1);
        Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MARKER);
        g_recordCnt = 0;
        (void)memset_s(g_mstxRecords, sizeof(msptiActivityMarker) * 100, 0, sizeof(msptiActivityMarker) * 100);
        Mspti::Parser::MstxParser::GetInstance()->gMarkId_.store(0);
    }
};

TEST_F(MstxParserUtest, MstxApisShouldRetSuccessAndRecordDataWhenReportMstxData)
{
    MOCKER_CPP(Mspti::Common::ProfTrace).stubs().will(returnValue(static_cast<AclError>(MSPTI_SUCCESS)));
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    const char* message = "UserMark";
    AclrtStream stream = (void*)0x1234567;
    const char* domain = "UserDomain";
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportMark(message, stream, domain));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportMark(message, nullptr, domain));
    uint64_t markId;
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeStartA(message, stream, markId, domain));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeEnd(markId));
    EXPECT_EQ(3UL, markId);

    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeStartA(message, nullptr, markId, domain));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeEnd(markId));
    EXPECT_EQ(4UL, markId);
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeEnd(markId + 1));

    msptiActivityFlushAll(1);
    EXPECT_EQ(6, g_recordCnt);

    EXPECT_STREQ(g_mstxRecords[0].domain, domain);
    EXPECT_STREQ(g_mstxRecords[0].name, message);
    EXPECT_EQ(1, g_mstxRecords[0].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE, g_mstxRecords[0].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[0].sourceKind);

    EXPECT_STREQ(g_mstxRecords[1].domain, domain);
    EXPECT_STREQ(g_mstxRecords[1].name, message);
    EXPECT_EQ(2, g_mstxRecords[1].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS, g_mstxRecords[1].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[0].sourceKind);

    EXPECT_STREQ(g_mstxRecords[2].domain, domain);
    EXPECT_STREQ(g_mstxRecords[2].name, message);
    EXPECT_EQ(3, g_mstxRecords[2].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE, g_mstxRecords[2].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[2].sourceKind);

    EXPECT_STREQ(g_mstxRecords[3].domain, "");
    EXPECT_STREQ(g_mstxRecords[3].name, "");
    EXPECT_EQ(3, g_mstxRecords[3].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE, g_mstxRecords[3].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[3].sourceKind);

    EXPECT_STREQ(g_mstxRecords[4].domain, domain);
    EXPECT_STREQ(g_mstxRecords[4].name, message);
    EXPECT_EQ(4, g_mstxRecords[4].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_START, g_mstxRecords[4].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[4].sourceKind);

    EXPECT_STREQ(g_mstxRecords[5].domain, "");
    EXPECT_STREQ(g_mstxRecords[5].name, "");
    EXPECT_EQ(4, g_mstxRecords[5].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_END, g_mstxRecords[5].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_HOST, g_mstxRecords[5].sourceKind);
}

TEST_F(MstxParserUtest, MstxApisWillReturnErrorWhenProfTraceFailWithValidInputStream)
{
    MOCKER_CPP(Mspti::Common::ProfTrace).stubs().will(returnValue(static_cast<AclError>(MSPTI_ERROR_INNER)));
    const char* message = "UserMark";
    const char* domain = "UserDomain";
    AclrtStream stream = (void*)0x1234567;
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->ReportMark(message, stream, domain));

    uint64_t markId = 0;
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->ReportRangeStartA(message, stream, markId, domain));
    EXPECT_EQ(2UL, markId);
}

TEST_F(MstxParserUtest, MstxRangeEndWillReturnErrorWhenProfTraceFailWhithValidInputMarkId)
{
    MOCKER_CPP(Mspti::Common::ProfTrace)
        .stubs()
        .will(returnValue(static_cast<AclError>(MSPTI_SUCCESS)))
        .then(returnValue(static_cast<AclError>(MSPTI_ERROR_INNER)));
    const char* message = "UserMark";
    const char* domain = "UserDomain";
    AclrtStream stream = (void*)0x1234567;
    uint64_t markId = 0;
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportRangeStartA(message, stream, markId, domain));
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->ReportRangeEnd(markId));
}

TEST_F(MstxParserUtest, MstxApisShouldRetErrorWhenTryCacheMarkmsgFailed)
{
    const std::string* nullPtr = nullptr;
    MOCKER_CPP(&Mspti::Parser::MstxParser::TryCacheMarkMsg).stubs().will(returnValue(nullPtr));
    const char* message = "UserMark";
    const char* domain = "UserDomain";
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->ReportMark(message, nullptr, domain));
    uint64_t markId = 0;
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->ReportRangeStartA(message, nullptr, markId, domain));
    EXPECT_EQ(0UL, markId);

    msptiActivityFlushAll(1);
    EXPECT_EQ(0, g_recordCnt);
}

TEST_F(MstxParserUtest, MstxRangeApisShouldRetErrorWhenRangeMarkFail)
{
    std::shared_ptr<std::string> nullPtr{nullptr};
    MOCKER_CPP(Mspti::Common::ProfTrace).stubs().will(returnValue(static_cast<AclError>(MSPTI_ERROR_INNER)));
    uint64_t markId = 0;
    AclrtStream stream = (void*)0x1234567;
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->InnerDeviceStartA(stream, markId));
    EXPECT_EQ(1UL, markId);
    EXPECT_EQ(MSPTI_SUCCESS, instance->InnerDeviceEndA(markId));
}

TEST_F(MstxParserUtest, MstxRangeEndShouldRetErrorWhenProfTraceForRangeEndRunsFail)
{
    MOCKER_CPP(Mspti::Common::ProfTrace)
        .stubs()
        .will(returnValue(static_cast<AclError>(MSPTI_SUCCESS)))
        .then(returnValue(static_cast<AclError>(MSPTI_ERROR_INNER)));
    std::shared_ptr<std::string> nullPtr{nullptr};
    uint64_t markId = 0;
    AclrtStream stream = (void*)0x1234567;
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->InnerDeviceStartA(stream, markId));
    EXPECT_EQ(MSPTI_ERROR_INNER, instance->InnerDeviceEndA(markId));
}

TEST_F(MstxParserUtest, ReportMarkDataToActivityWillReturnAndNotRecordDataWhenInputNullStepTrace)
{
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    instance->ReportMarkDataToActivity(0, nullptr);
    EXPECT_EQ(0, g_recordCnt);
}

TEST_F(MstxParserUtest, ReportMarkDataToActivityWillRecordDataWhenInputValidStepStrace)
{
    std::unique_ptr<Mspti::Common::DevTimeInfo> dev_ptr = nullptr;
    Mspti::Common::MsptiMakeUniquePtr(dev_ptr);
    ASSERT_NE(nullptr, dev_ptr);
    Mspti::Common::ContextManager::GetInstance()->devTimeInfo_[0] = std::move(dev_ptr);
    StepTraceBasic markStepTrace = {
        .timestamp = 100,
        .indexId = 1,
        .modelId = static_cast<uint64_t>(MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE),
        .tagId = 11,
        .streamId = 1,
        .taskId = 1};
    StepTraceBasic rangeStartStepTrace = {
        .timestamp = 110,
        .indexId = 2,
        .modelId = static_cast<uint64_t>(MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE),
        .tagId = 11,
        .streamId = 1,
        .taskId = 2};
    StepTraceBasic rangeEndStepTrace = {.timestamp = 120,
                                        .indexId = 2,
                                        .modelId = static_cast<uint64_t>(MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE),
                                        .tagId = 11,
                                        .streamId = 1,
                                        .taskId = 2};
    auto instance = Mspti::Parser::MstxParser::GetInstance();
    instance->innerMarkIds.clear();
    instance->ReportMarkDataToActivity(0, &markStepTrace);
    instance->ReportMarkDataToActivity(0, &rangeStartStepTrace);
    instance->ReportMarkDataToActivity(0, &rangeEndStepTrace);
    msptiActivityFlushAll(1);
    EXPECT_EQ(3, g_recordCnt);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_DEVICE, g_mstxRecords[0].sourceKind);
    EXPECT_EQ(1, g_mstxRecords[0].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_INSTANTANEOUS_WITH_DEVICE, g_mstxRecords[0].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_DEVICE, g_mstxRecords[1].sourceKind);
    EXPECT_EQ(2, g_mstxRecords[1].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_START_WITH_DEVICE, g_mstxRecords[1].flag);
    EXPECT_EQ(MSPTI_ACTIVITY_SOURCE_KIND_DEVICE, g_mstxRecords[2].sourceKind);
    EXPECT_EQ(2, g_mstxRecords[2].id);
    EXPECT_EQ(MSPTI_ACTIVITY_FLAG_MARKER_END_WITH_DEVICE, g_mstxRecords[2].flag);
}
}  // namespace
