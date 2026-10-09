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

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/parser/cann_hash_cache.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/activity/ascend/parser/mstx_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/acl_inject.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "securec.h"

namespace
{
using namespace Mspti;
class DeviceTaskCalculatorUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
    virtual void TearDown() {}
};

TEST_F(DeviceTaskCalculatorUtest, ShouldReturnTaskWhenSocLogReport)
{
    uint16_t deviceId = 1;
    uint16_t streamId = 1;
    uint16_t taskId = 1;
    auto &instance = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    Mspti::Parser::DeviceTask firstTask(0, 0, streamId, taskId, deviceId);
    instance.RegisterCallBack(firstTask,
                              [deviceId](const Mspti::Parser::DeviceTask &task)
                              {
                                  EXPECT_EQ(task.deviceId, 1);
                                  return MSPTI_SUCCESS;
                              });

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

    EXPECT_EQ(instance.ReportStarsSocLog(deviceId, socLogStart), MSPTI_SUCCESS);
    EXPECT_EQ(instance.ReportStarsSocLog(deviceId, socLogEnd), MSPTI_SUCCESS);
}

TEST_F(DeviceTaskCalculatorUtest, ShouldReturnTaskWhenFftsLogReport)
{
    uint16_t deviceId = 1;
    uint16_t streamId = 1;
    uint16_t taskId = 1;
    uint16_t subTaskId = 1;
    Mspti::Parser::DeviceTask firstTask(0, 0, streamId, taskId, deviceId);
    auto &instance = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    instance.RegisterCallBack(firstTask,
                              [deviceId](const Mspti::Parser::DeviceTask &task)
                              {
                                  EXPECT_EQ(task.deviceId, deviceId);
                                  EXPECT_EQ(task.subTasks.size(), 1);
                                  return MSPTI_SUCCESS;
                              });

    HalLogData socLogStart;
    (void)memset_s(&socLogStart, sizeof(socLogStart), 0, sizeof(socLogStart));
    socLogStart.type = ACSQ_LOG;
    socLogStart.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    socLogStart.acsq.streamId = streamId;
    socLogStart.acsq.taskId = taskId;

    HalLogData socLogEnd;
    (void)memset_s(&socLogEnd, sizeof(socLogEnd), 0, sizeof(socLogEnd));
    socLogEnd.type = ACSQ_LOG;
    socLogEnd.acsq.funcType = STARS_FUNC_TYPE_END;
    socLogEnd.acsq.streamId = streamId;
    socLogEnd.acsq.taskId = taskId;

    HalLogData fftsLogStart;
    (void)memset_s(&fftsLogStart, sizeof(fftsLogStart), 0, sizeof(fftsLogStart));
    fftsLogStart.type = FFTS_LOG;
    fftsLogStart.ffts.funcType = FFTS_PLUS_TYPE_START;
    fftsLogStart.ffts.streamId = streamId;
    fftsLogStart.ffts.taskId = taskId;
    fftsLogStart.ffts.subTaskId = subTaskId;

    HalLogData fftsLogEnd;
    (void)memset_s(&fftsLogEnd, sizeof(fftsLogEnd), 0, sizeof(fftsLogEnd));
    fftsLogEnd.type = FFTS_LOG;
    fftsLogEnd.ffts.funcType = FFTS_PLUS_TYPE_END;
    fftsLogEnd.ffts.streamId = streamId;
    fftsLogEnd.ffts.taskId = taskId;
    fftsLogEnd.ffts.subTaskId = subTaskId;

    instance.ReportStarsSocLog(deviceId, socLogStart);
    instance.ReportStarsSocLog(deviceId, fftsLogStart);
    instance.ReportStarsSocLog(deviceId, fftsLogEnd);
    instance.ReportStarsSocLog(deviceId, socLogEnd);
}

TEST_F(DeviceTaskCalculatorUtest, ShouldNotReturnTask)
{
    uint16_t deviceId = 1;
    uint16_t streamId = 1;
    uint16_t taskId = 1;
    uint16_t subTaskId = 1;
    Mspti::Parser::DeviceTask firstTask(0, 0, streamId, taskId, deviceId);
    auto &instance = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    instance.RegisterCallBack(firstTask,
                              [deviceId](const Mspti::Parser::DeviceTask &task)
                              {
                                  EXPECT_TRUE(false);
                                  return MSPTI_SUCCESS;
                              });

    HalLogData socLogStart;
    (void)memset_s(&socLogStart, sizeof(socLogStart), 0, sizeof(socLogStart));
    socLogStart.type = ACSQ_LOG;
    socLogStart.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    socLogStart.acsq.streamId = streamId;
    socLogStart.acsq.taskId = taskId;

    HalLogData fftsLogEnd;
    (void)memset_s(&fftsLogEnd, sizeof(fftsLogEnd), 0, sizeof(fftsLogEnd));
    fftsLogEnd.type = FFTS_LOG;
    fftsLogEnd.ffts.funcType = FFTS_PLUS_TYPE_END;
    fftsLogEnd.ffts.streamId = streamId + 1;
    fftsLogEnd.ffts.taskId = taskId + 1;
    fftsLogEnd.ffts.subTaskId = subTaskId + 1;

    instance.ReportStarsSocLog(deviceId, socLogStart);
    instance.ReportStarsSocLog(deviceId, fftsLogEnd);
}

// Open the COMMUNICATION gate without Register side effects; the next SetUp verify() clears the stub.
TEST_F(DeviceTaskCalculatorUtest, EndLogConvertsTaskTimeConsistentlyWithScalar)
{
    MOCKER_CPP(&Mspti::Activity::ActivityManager::IsActivityKindEnable).stubs().will(returnValue(true));
    constexpr uint32_t deviceId = 1;
    constexpr uint16_t streamId = 2;
    constexpr uint32_t taskId = 8101;
    constexpr uint64_t beginNs = 1000000;
    constexpr uint64_t endNs = 2000000;
    Common::ContextManager::GetInstance()->InitDeviceTimeInfo(deviceId);

    auto &instance = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    Mspti::Parser::DeviceTask task(0, 0, streamId, taskId, deviceId);
    bool callbackFired = false;
    instance.RegisterCallBack(task,
                              [&](const Mspti::Parser::DeviceTask &reported)
                              {
                                  callbackFired = true;
                                  auto &context = *Common::ContextManager::GetInstance();
                                  EXPECT_EQ(reported.start, context.GetDeviceRealTime(deviceId, beginNs));
                                  EXPECT_EQ(reported.end, context.GetDeviceRealTime(deviceId, endNs));
                                  EXPECT_GT(reported.start, 0ULL);
                                  EXPECT_LT(reported.start, reported.end);
                                  EXPECT_NE(reported.start, beginNs);
                                  return MSPTI_SUCCESS;
                              });

    HalLogData socLogStart;
    (void)memset_s(&socLogStart, sizeof(socLogStart), 0, sizeof(socLogStart));
    socLogStart.type = ACSQ_LOG;
    socLogStart.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    socLogStart.acsq.streamId = streamId;
    socLogStart.acsq.taskId = taskId;
    socLogStart.acsq.timestamp = beginNs;

    HalLogData socLogEnd;
    (void)memset_s(&socLogEnd, sizeof(socLogEnd), 0, sizeof(socLogEnd));
    socLogEnd.type = ACSQ_LOG;
    socLogEnd.acsq.funcType = STARS_FUNC_TYPE_END;
    socLogEnd.acsq.streamId = streamId;
    socLogEnd.acsq.taskId = taskId;
    socLogEnd.acsq.timestamp = endNs;

    EXPECT_EQ(instance.ReportStarsSocLog(deviceId, socLogStart), MSPTI_SUCCESS);
    EXPECT_EQ(instance.ReportStarsSocLog(deviceId, socLogEnd), MSPTI_SUCCESS);
    EXPECT_TRUE(callbackFired);
}

// The ffts END branch uses the same pair-wise conversion as the task END above.
TEST_F(DeviceTaskCalculatorUtest, FftsEndConvertsSubTaskTimeConsistentlyWithScalar)
{
    MOCKER_CPP(&Mspti::Activity::ActivityManager::IsActivityKindEnable).stubs().will(returnValue(true));
    constexpr uint32_t deviceId = 1;
    constexpr uint16_t streamId = 2;
    constexpr uint32_t taskId = 8102;
    constexpr uint16_t subTaskId = 7;
    constexpr uint64_t beginNs = 1000000;
    constexpr uint64_t subBeginNs = 1500000;
    constexpr uint64_t subEndNs = 1900000;
    constexpr uint64_t endNs = 2000000;
    Common::ContextManager::GetInstance()->InitDeviceTimeInfo(deviceId);

    auto &instance = Mspti::Parser::DeviceTaskCalculator::GetInstance();
    Mspti::Parser::DeviceTask task(0, 0, streamId, taskId, deviceId);
    bool callbackFired = false;
    instance.RegisterCallBack(task,
                              [&](const Mspti::Parser::DeviceTask &reported)
                              {
                                  callbackFired = true;
                                  auto &context = *Common::ContextManager::GetInstance();
                                  EXPECT_EQ(reported.start, context.GetDeviceRealTime(deviceId, beginNs));
                                  EXPECT_EQ(reported.end, context.GetDeviceRealTime(deviceId, endNs));
                                  EXPECT_EQ(reported.subTasks.size(), 1U);
                                  if (reported.subTasks.size() == 1)
                                  {
                                      const auto &subTask = reported.subTasks[0];
                                      EXPECT_EQ(subTask.start, context.GetDeviceRealTime(deviceId, subBeginNs));
                                      EXPECT_EQ(subTask.end, context.GetDeviceRealTime(deviceId, subEndNs));
                                      EXPECT_LT(subTask.start, subTask.end);
                                  }
                                  return MSPTI_SUCCESS;
                              });

    HalLogData socLogStart;
    (void)memset_s(&socLogStart, sizeof(socLogStart), 0, sizeof(socLogStart));
    socLogStart.type = ACSQ_LOG;
    socLogStart.acsq.funcType = STARS_FUNC_TYPE_BEGIN;
    socLogStart.acsq.streamId = streamId;
    socLogStart.acsq.taskId = taskId;
    socLogStart.acsq.timestamp = beginNs;

    HalLogData socLogEnd;
    (void)memset_s(&socLogEnd, sizeof(socLogEnd), 0, sizeof(socLogEnd));
    socLogEnd.type = ACSQ_LOG;
    socLogEnd.acsq.funcType = STARS_FUNC_TYPE_END;
    socLogEnd.acsq.streamId = streamId;
    socLogEnd.acsq.taskId = taskId;
    socLogEnd.acsq.timestamp = endNs;

    HalLogData fftsLogStart;
    (void)memset_s(&fftsLogStart, sizeof(fftsLogStart), 0, sizeof(fftsLogStart));
    fftsLogStart.type = FFTS_LOG;
    fftsLogStart.ffts.funcType = FFTS_PLUS_TYPE_START;
    fftsLogStart.ffts.streamId = streamId;
    fftsLogStart.ffts.taskId = taskId;
    fftsLogStart.ffts.subTaskId = subTaskId;
    fftsLogStart.ffts.timestamp = subBeginNs;

    HalLogData fftsLogEnd;
    (void)memset_s(&fftsLogEnd, sizeof(fftsLogEnd), 0, sizeof(fftsLogEnd));
    fftsLogEnd.type = FFTS_LOG;
    fftsLogEnd.ffts.funcType = FFTS_PLUS_TYPE_END;
    fftsLogEnd.ffts.streamId = streamId;
    fftsLogEnd.ffts.taskId = taskId;
    fftsLogEnd.ffts.subTaskId = subTaskId;
    fftsLogEnd.ffts.timestamp = subEndNs;

    instance.ReportStarsSocLog(deviceId, socLogStart);
    instance.ReportStarsSocLog(deviceId, fftsLogStart);
    instance.ReportStarsSocLog(deviceId, fftsLogEnd);
    instance.ReportStarsSocLog(deviceId, socLogEnd);
    EXPECT_TRUE(callbackFired);
}
}  // namespace
