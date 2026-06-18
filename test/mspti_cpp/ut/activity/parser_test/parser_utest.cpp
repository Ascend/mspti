/* -------------------------------------------------------------------------
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
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

#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/activity/ascend/parser/kernel_parser.h"
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/common/inject/acl_inject.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
constexpr uint32_t TS_TASK_TYPE_KERNEL_AIVEC = 66;
using namespace Mspti;
class ParserUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        GlobalMockObject::verify();
        Mspti::Parser::CannHashCache::RegTypeHashInfo(MSPROF_REPORT_RUNTIME_LEVEL, TS_TASK_TYPE_KERNEL_AIVEC,
                                                      "KERNEL_AIVEC");
    }
    virtual void TearDown() {}
};

TEST_F(ParserUtest, ShouldRetSuccessWhenReportApiSuccess)
{
    auto instance = Mspti::Parser::ParserManager::GetInstance();
    const std::string hashInfo = "aclnnAdd_AxpyAiCore_Axpy";
    auto hashId = Mspti::Parser::CannHashCache::GenHashId(hashInfo);
    EXPECT_TRUE(Mspti::Parser::CannHashCache::GetHashInfo(0).empty());  // 0: invalid hash id to get hash info
    EXPECT_STREQ(hashInfo.c_str(), Mspti::Parser::CannHashCache::GetHashInfo(hashId).c_str());
    constexpr uint16_t level = 20000;
    constexpr uint32_t typeId = 1;
    const std::string typeName = "acl_api";
    MsprofApi data;
    (void)memset_s(&data, sizeof(data), 0, sizeof(data));
    data.level = level;
    data.type = 1;
    data.threadId = Mspti::Common::Utils::GetPid();
    data.beginTime = Mspti::Common::Utils::GetClockRealTimeNs();
    data.endTime = data.beginTime;
    data.itemId = hashId;
    EXPECT_EQ(MSPTI_SUCCESS, instance->ReportApi(&data));
}

TEST_F(ParserUtest, ShouldRetSuccessWhenReportKernelInfo)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint16_t flipId = 0;
    constexpr uint16_t taskId = 1;
    constexpr uint32_t deviceId = 0;
    constexpr uint32_t streamId = 3;
    constexpr uint32_t BIT_NUM = 16;
    MsprofCompactInfo compactInfo;
    (void)memset_s(&compactInfo, sizeof(compactInfo), 0, sizeof(compactInfo));
    compactInfo.data.runtimeTrack.deviceId = deviceId;
    compactInfo.data.runtimeTrack.streamId = streamId;
    compactInfo.data.runtimeTrack.taskInfo = static_cast<uint32_t>(flipId) << BIT_NUM | static_cast<uint32_t>(taskId);
    compactInfo.data.runtimeTrack.taskType = TS_TASK_TYPE_KERNEL_AIVEC;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compactInfo));
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compactInfo));

    HalLogData socLogStart;
    (void)memset_s(&socLogStart, sizeof(socLogStart), 0, sizeof(socLogStart));
    socLogStart.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    socLogStart.acsq.streamId = streamId;
    socLogStart.acsq.taskId = taskId;

    HalLogData socLogEnd;
    (void)memset_s(&socLogEnd, sizeof(socLogEnd), 0, sizeof(socLogEnd));
    socLogEnd.acsq.funcType = STARS_FUNC_TYPE_END;
    socLogEnd.acsq.streamId = streamId;
    socLogEnd.acsq.taskId = taskId;

    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(deviceId, socLogStart));
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(deviceId, socLogEnd));
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(deviceId, socLogStart));
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportStarsSocLog(deviceId, socLogEnd));
}

TEST_F(ParserUtest, ShouldRecordKernelNameWhenReportRtTaskTrack)
{
    auto& instance = Mspti::Parser::KernelParser::GetInstance();
    constexpr uint16_t flipId = 0;
    constexpr uint16_t taskId = 1;
    constexpr uint32_t deviceId = 0;
    constexpr uint32_t streamId = 3;
    constexpr uint32_t BIT_NUM = 16;
    const std::string kernelName = "test_kernelName";
    auto kernelNameHash = Mspti::Parser::CannHashCache::GenHashId(kernelName);
    MsprofCompactInfo compactInfo;
    (void)memset_s(&compactInfo, sizeof(compactInfo), 0, sizeof(compactInfo));
    compactInfo.data.runtimeTrack.deviceId = deviceId;
    compactInfo.data.runtimeTrack.streamId = streamId;
    compactInfo.data.runtimeTrack.taskInfo = static_cast<uint32_t>(flipId) << BIT_NUM | static_cast<uint32_t>(taskId);
    compactInfo.data.runtimeTrack.taskType = TS_TASK_TYPE_KERNEL_AIVEC;
    compactInfo.data.runtimeTrack.kernelName = kernelNameHash;
    EXPECT_EQ(MSPTI_SUCCESS, instance.ReportRtTaskTrack(1, &compactInfo));
}
}  // namespace
