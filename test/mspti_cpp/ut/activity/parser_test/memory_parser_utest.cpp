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

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/memory_parser.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
using namespace Mspti;

class MemoryParserUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        MOCKER_CPP(&Mspti::Common::GetCANNModuleVersion).stubs().will(returnValue(std::string("9.2.0")));
        EXPECT_EQ(MSPTI_SUCCESS, Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_MEMORY));
        EXPECT_EQ(MSPTI_SUCCESS, Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_MEMSET));
        EXPECT_EQ(MSPTI_SUCCESS, Mspti::Activity::ActivityManager::GetInstance()->Register(MSPTI_ACTIVITY_KIND_MEMCPY));
    }
    virtual void TearDown() {}
};

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemoryWithMalloc)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memMngInfo.address = 0x1234567890ABCDEFULL;
    record.data.memMngInfo.size = 1024;
    record.data.memMngInfo.memoryType = 1;
    record.data.memMngInfo.memMngType = 0;
    record.data.memMngInfo.deviceId = 0;
    record.data.memMngInfo.streamId = 0;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemory(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemoryWithFree)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memMngInfo.address = 0x1234567890ABCDEFULL;
    record.data.memMngInfo.size = 1024;
    record.data.memMngInfo.memoryType = 1;
    record.data.memMngInfo.memMngType = 1;
    record.data.memMngInfo.deviceId = 0;
    record.data.memMngInfo.streamId = UINT32_MAX;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemory(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemoryWithHostMemory)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memMngInfo.address = 0xABCDEF1234567890ULL;
    record.data.memMngInfo.size = 2048;
    record.data.memMngInfo.memoryType = 2;
    record.data.memMngInfo.memMngType = 0;
    record.data.memMngInfo.deviceId = 0;
    record.data.memMngInfo.streamId = 1;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemory(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemoryWithManagedMemory)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memMngInfo.address = 0xFEDCBA9876543210ULL;
    record.data.memMngInfo.size = 4096;
    record.data.memMngInfo.memoryType = 3;
    record.data.memMngInfo.memMngType = 0;
    record.data.memMngInfo.deviceId = 0;
    record.data.memMngInfo.streamId = 2;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemory(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemset)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memsetInfo.bytes = 1024;
    record.data.memsetInfo.value = 0xAA;
    record.data.memsetInfo.streamId = 0;
    record.data.memsetInfo.deviceId = 0;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemset(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemsetAsync)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memsetInfo.bytes = 2048;
    record.data.memsetInfo.value = 0x55;
    record.data.memsetInfo.streamId = 3;
    record.data.memsetInfo.deviceId = 0;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemset(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemcpy)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memcpyInfo.bytes = 1024;
    record.data.memcpyInfo.copyKind = 1;
    record.data.memcpyInfo.deviceId = 0;
    record.data.memcpyInfo.streamId = UINT32_MAX;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemcpy(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemcpyH2D)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memcpyInfo.bytes = 4096;
    record.data.memcpyInfo.copyKind = 1;
    record.data.memcpyInfo.deviceId = 0;
    record.data.memcpyInfo.streamId = 2;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemcpy(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemcpyD2H)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memcpyInfo.bytes = 8192;
    record.data.memcpyInfo.copyKind = 2;
    record.data.memcpyInfo.deviceId = 0;
    record.data.memcpyInfo.streamId = 1;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemcpy(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemcpyD2D)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memcpyInfo.bytes = 16384;
    record.data.memcpyInfo.copyKind = 3;
    record.data.memcpyInfo.deviceId = 0;
    record.data.memcpyInfo.streamId = 0;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemcpy(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenRecordApiWithMemoryDisabled)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMORY);
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMSET);
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMCPY);

    MsprofApi api;
    (void)memset_s(&api, sizeof(api), 0, sizeof(api));
    api.level = MSPROF_REPORT_ACL_LEVEL;
    api.type = (ACL_RTS << 16) | 1;
    api.threadId = Mspti::Common::Utils::GetTid();
    api.beginTime = Mspti::Common::Utils::GetClockRealTimeNs();
    api.endTime = api.beginTime + 1000;

    EXPECT_EQ(MSPTI_SUCCESS, instance.RecordApi(api));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemoryDisabled)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMORY);

    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memMngInfo.address = 0x1111222233334444ULL;
    record.data.memMngInfo.size = 1024;
    record.data.memMngInfo.memoryType = 1;
    record.data.memMngInfo.memMngType = 0;

    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemory(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemsetDisabled)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMSET);

    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memsetInfo.bytes = 1024;
    record.data.memsetInfo.value = 0x00;

    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemset(record));
}

TEST_F(MemoryParserUtest, ShouldRetSuccessWhenReportMemcpyDisabled)
{
    auto& instance = Mspti::Parser::MemoryParser::GetInstance();
    Mspti::Activity::ActivityManager::GetInstance()->UnRegister(MSPTI_ACTIVITY_KIND_MEMCPY);

    MsprofCompactInfo record;
    (void)memset_s(&record, sizeof(record), 0, sizeof(record));
    record.threadId = Mspti::Common::Utils::GetTid();
    record.timeStamp = Mspti::Common::Utils::GetClockRealTimeNs();
    record.data.memcpyInfo.bytes = 1024;
    record.data.memcpyInfo.copyKind = 1;

    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportMemcpy(record));
}
}  // namespace
