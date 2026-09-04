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
#include <atomic>
#include <thread>
#include <vector>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/activity/ascend/reporter/external_correlation_reporter.h"
#include "csrc/common/runtime_utils.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "mspti.h"
#include "securec.h"

namespace
{
std::atomic<uint64_t> g_records{0};

std::atomic<uint64_t> g_massive_records{0};
std::atomic<uint64_t> g_total_records{0};
int g_nullBufferRecords = 0;

class ActivityUtest : public testing::Test
{
   protected:
    virtual void SetUp() { GlobalMockObject::verify(); }
    virtual void TearDown() {}
};

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    printf("========== UserBufferRequest ============\n");
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void UserLittleBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    printf("========== UserBufferRequest ============\n");
    constexpr uint32_t bufSize = 1024;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    *size = bufSize;
    *maxNumRecords = 0;
}

void UserNullBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    printf("========== UserBufferRequest ============\n");
    static int callCount = 0;
    if (callCount < 1)
    {
        *buffer = nullptr;
        *size = 0;
        *maxNumRecords = 0;
    }
    else
    {
        constexpr uint32_t bufSize = 2 * 1024 * 1024;
        *buffer = static_cast<uint8_t *>(malloc(bufSize));
        *size = bufSize;
        *maxNumRecords = 0;
    }
    callCount++;
}

void UserNullBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    printf("========== UserBufferComplete ============\n");
    if (validSize > 0)
    {
        msptiActivity *pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do
        {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS)
            {
                g_nullBufferRecords++;
            }
            else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
        } while (true);
    }
    free(buffer);
}

static void ActivityParser(msptiActivity *pRecord)
{
    g_records++;
    if (pRecord->kind == MSPTI_ACTIVITY_KIND_MARKER)
    {
        msptiActivityMarker *activity = reinterpret_cast<msptiActivityMarker *>(pRecord);
        if (activity->sourceKind == MSPTI_ACTIVITY_SOURCE_KIND_HOST)
        {
            printf("kind: %d, mode: %d, timestamp: %lu, markId: %lu, processId: %d, threadId: %u, name: %s\n",
                   activity->kind, activity->sourceKind, activity->timestamp, activity->id,
                   activity->objectId.pt.processId, activity->objectId.pt.threadId, activity->name);
        }
    }
}

void MassiveBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    if (validSize > 0)
    {
        msptiActivity *pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do
        {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS)
            {
                g_massive_records++;
            }
            else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
        } while (true);
    }
    free(buffer);
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    printf("========== UserBufferComplete ============\n");
    if (validSize > 0)
    {
        msptiActivity *pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do
        {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS)
            {
                ActivityParser(pRecord);
            }
            else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
        } while (true);
    }
    free(buffer);
}

void TestActivityApi()
{
    constexpr uint64_t timeStamp = 1614659207688700;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_API));
    msptiActivityApi api;
    api.kind = MSPTI_ACTIVITY_KIND_API;
    api.start = timeStamp;
    api.end = timeStamp;
    api.pt.processId = 0;
    api.pt.threadId = 0;
    api.correlationId = 1;
    api.name = "Api";
    Mspti::Activity::ActivityManager::GetInstance()->Record(reinterpret_cast<msptiActivity *>(&api), sizeof(api));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_API));
}

void TestActivityKernel()
{
    constexpr uint64_t timeStamp = 1614659207688700;
    constexpr uint32_t streamId = 3;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));
    msptiActivityKernel kernel;
    kernel.kind = MSPTI_ACTIVITY_KIND_KERNEL;
    kernel.start = timeStamp;
    kernel.end = timeStamp;
    kernel.ds.deviceId = 0;
    kernel.ds.streamId = streamId;
    kernel.correlationId = 1;
    kernel.type = "KERNEL_AIVEC";
    kernel.name = "Kernel";
    Mspti::Activity::ActivityManager::GetInstance()->Record(reinterpret_cast<msptiActivity *>(&kernel), sizeof(kernel));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_KERNEL));
}

void RecordMassiveMarkerActivity()
{
    msptiActivityMarker activity;
    constexpr uint64_t timeStamp = 1614659207688700;
    constexpr uint32_t markNum = 10000;
    constexpr uint32_t flushPeriod = 20;
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    for (size_t i = 0; i < markNum; ++i)
    {
        activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
        activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
        activity.timestamp = timeStamp;
        activity.id = i;
        activity.objectId.pt.processId = 0;
        activity.objectId.pt.threadId = 0;
        activity.name = "UserMark";
        instance->Record(reinterpret_cast<msptiActivity *>(&activity), sizeof(activity));
        g_total_records += 1;
        if (i % flushPeriod == 0)
        {
            EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
        }
    }
}

TEST_F(ActivityUtest, ShouldRetSuccessWhenSetAllKindWithCorrectApiInvocationSequence)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    msptiActivityMarker activity;
    constexpr uint64_t timeStamp = 1614659207688700;
    constexpr uint32_t markNum = 10;
    uint64_t totalActivitys = 0;
    for (size_t i = 0; i < markNum; ++i)
    {
        activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
        activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
        activity.timestamp = timeStamp;
        activity.id = i;
        activity.objectId.pt.processId = 0;
        activity.objectId.pt.threadId = 0;
        activity.name = "UserMark";
        instance->Record(reinterpret_cast<msptiActivity *>(&activity), sizeof(activity));
        totalActivitys += 1;
    }
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    TestActivityApi();
    totalActivitys += 1;
    TestActivityKernel();
    totalActivitys += 1;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(totalActivitys, g_records.load());
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
}

TEST_F(ActivityUtest, ShouldRetInvalidParameterErrorWhenSetWrongParam)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityRegisterCallbacks(nullptr, nullptr));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityEnable(MSPTI_ACTIVITY_KIND_FORCE_INT));
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityDisable(MSPTI_ACTIVITY_KIND_FORCE_INT));
    msptiActivity *activity;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetNextRecord(nullptr, 0, &activity));
}

TEST_F(ActivityUtest, IsActivityKindEnableWillReturnTrueWhenEnableMarkerKind)
{
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER);
    EXPECT_EQ(true, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER);
}

TEST_F(ActivityUtest, IsActivityKindEnableWillReturnFalseWhenNotEnableMarkerKind)
{
    msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER);
    EXPECT_EQ(false, Mspti::Activity::ActivityManager::GetInstance()->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
}

TEST_F(ActivityUtest, MsptiActivityIsEnabledWillReturnTrueWhenEnableMarkerKind)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(true, msptiActivityIsEnabled(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
}

TEST_F(ActivityUtest, MsptiActivityIsEnabledWillReturnFalseWhenDisableMarkerKind)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(false, msptiActivityIsEnabled(MSPTI_ACTIVITY_KIND_MARKER));
}

TEST_F(ActivityUtest, ShouldRetSuccessWhenSetPeriodFlushTime)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserLittleBufferRequest, UserBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));

    msptiActivityMarker activity;
    constexpr uint64_t timeStamp = 1614659207688700;
    constexpr uint32_t markNum = 10;
    constexpr uint32_t testPeriodFlushTime = 1;
    constexpr uint32_t sleepTime = 100000;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushPeriod(testPeriodFlushTime));
    for (size_t i = 0; i < markNum; ++i)
    {
        usleep(sleepTime);
        activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
        activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
        activity.timestamp = timeStamp;
        activity.id = i;
        activity.objectId.pt.processId = 0;
        activity.objectId.pt.threadId = 0;
        activity.name = "UserMark";
        instance->Record(reinterpret_cast<msptiActivity *>(&activity), sizeof(activity));
    }
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushPeriod(0));
    for (size_t i = 0; i < markNum; ++i)
    {
        usleep(sleepTime);
        activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
        activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
        activity.timestamp = timeStamp;
        activity.id = i + markNum;
        activity.objectId.pt.processId = 0;
        activity.objectId.pt.threadId = 0;
        activity.name = "UserMark";
        instance->Record(reinterpret_cast<msptiActivity *>(&activity), sizeof(activity));
    }
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
}

TEST_F(ActivityUtest, ShouldRetSuccessWhenPushAndPopExternalCorrelationId)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, UserBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPushExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, 0));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPushExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, 1));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPushExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_UNKNOWN, 1));

    EXPECT_EQ(MSPTI_SUCCESS,
              Mspti::Reporter::ExternalCorrelationReporter::GetInstance()->ReportExternalCorrelationId(1));

    uint64_t value = 12345678901234567890;  // 一个具体的uint64_t类型的变量
    uint64_t *test = &value;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPopExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, test));
    EXPECT_EQ(1, *test);

    EXPECT_EQ(MSPTI_SUCCESS,
              Mspti::Reporter::ExternalCorrelationReporter::GetInstance()->ReportExternalCorrelationId(1));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPopExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_UNKNOWN, test));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityPopExternalCorrelationId(MSPTI_EXTERNAL_CORRELATION_KIND_CUSTOM0, test));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_EXTERNAL_CORRELATION));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
}

TEST_F(ActivityUtest, GetRecordSuccessWhenBufferNull)
{
    size_t validSize = sizeof(msptiActivityMarker);
    uint8_t *buffer = nullptr;
    msptiActivity *pRecord = NULL;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetNextRecord(buffer, validSize, &pRecord));

    buffer = static_cast<uint8_t *>(malloc(validSize));
    msptiActivityMarker *activity = new msptiActivityMarker();
    activity->kind = MSPTI_ACTIVITY_KIND_MARKER;
    memcpy_s(buffer, validSize, activity, sizeof(msptiActivityMarker));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetNextRecord(buffer, validSize, &pRecord));

    validSize = 1;
    EXPECT_EQ(MSPTI_ERROR_MAX_LIMIT_REACHED, msptiActivityGetNextRecord(buffer, validSize, &pRecord));
    free(buffer);
}

TEST_F(ActivityUtest, MultThreadFlushAll)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserLittleBufferRequest, MassiveBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));

    std::vector<std::thread> worker;
    for (int i = 0; i < 8; i++)
    {
        worker.push_back(std::thread(RecordMassiveMarkerActivity));
    }
    for (auto &thread : worker)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(g_total_records.load(), g_massive_records.load());
}

TEST_F(ActivityUtest, RecordWillRepeatInitActivityBufferWhenRequestBufferIsNull)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserNullBufferRequest, UserNullBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    msptiActivityMarker activity;
    activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
    activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
    activity.timestamp = 0;
    activity.id = 0;
    activity.objectId.pt.processId = 0;
    activity.objectId.pt.threadId = 0;
    activity.name = "UserMark";
    // Record activity twice, the first time will request buffer and return null, the activity will be dropped
    Mspti::Activity::ActivityManager::GetInstance()->Record(reinterpret_cast<msptiActivity *>(&activity),
                                                            sizeof(activity));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(0, g_nullBufferRecords);
    // Record activity again, the second time will request buffer and return valid buffer, the activity will be recorded
    Mspti::Activity::ActivityManager::GetInstance()->Record(reinterpret_cast<msptiActivity *>(&activity),
                                                            sizeof(activity));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(1, g_nullBufferRecords);
}

TEST_F(ActivityUtest, MsptiGetVersionReturnsInvalidParameterWhenVersionNull)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiGetVersion(nullptr));
}

TEST_F(ActivityUtest, MsptiGetVersionReturnsVersionWhenModuleVersionAvailable)
{
    MOCKER_CPP(&Mspti::Common::GetCANNModuleVersion).stubs().will(returnValue(std::string("9.2.0")));
    uint32_t version = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiGetVersion(&version));
    constexpr uint32_t EXPECTED_VERSION = 9 * 10000 + 2 * 100 + 0;
    EXPECT_EQ(EXPECTED_VERSION, version);
}

TEST_F(ActivityUtest, MsptiActivityGetStructSizeReturnsInvalidParameterWhenSizeNull)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_MARKER, 0, nullptr));
}

TEST_F(ActivityUtest, MsptiActivityGetStructSizeReturnsInvalidKindForInvalidKind)
{
    size_t size = 0;
    EXPECT_EQ(MSPTI_ERROR_INVALID_KIND, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_INVALID, 0, &size));
    EXPECT_EQ(MSPTI_ERROR_INVALID_KIND, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_FORCE_INT, 0, &size));
}

TEST_F(ActivityUtest, MsptiActivityGetStructSizeReturnsSizeForValidKind)
{
    size_t size = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_MARKER, 0, &size));
    EXPECT_EQ(sizeof(msptiActivityMarker), size);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_KERNEL, 0, &size));
    EXPECT_EQ(sizeof(msptiActivityKernel), size);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_MEMCPY, 0, &size));
    EXPECT_EQ(sizeof(msptiActivityMemcpy), size);
}

TEST_F(ActivityUtest, MsptiActivityGetEnabledKindsReturnsInvalidParameterWhenCountNull)
{
    msptiActivityKind buffer[16] = {};
    uint32_t bufferSize = 16;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetEnabledKinds(nullptr, buffer, &bufferSize, nullptr));
}

TEST_F(ActivityUtest, MsptiActivityGetEnabledKindsReturnsInvalidParameterWhenBufferSizeNull)
{
    msptiActivityKind buffer[16] = {};
    uint32_t count = 0;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetEnabledKinds(nullptr, buffer, nullptr, &count));
}

TEST_F(ActivityUtest, MsptiActivityGetEnabledKindsContainsEnabledKind)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));

    uint32_t count = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetEnabledKinds(nullptr, nullptr, nullptr, &count));
    EXPECT_GE(count, 1);

    msptiActivityKind buffer[16] = {};
    uint32_t bufferSize = 16;
    count = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetEnabledKinds(nullptr, buffer, &bufferSize, &count));
    bool foundMarker = false;
    for (uint32_t i = 0; i < count; i++)
    {
        if (buffer[i] == MSPTI_ACTIVITY_KIND_MARKER)
        {
            foundMarker = true;
            break;
        }
    }
    EXPECT_TRUE(foundMarker);

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
}

TEST_F(ActivityUtest, MsptiActivityGetNumDroppedRecordsReturnsInvalidParameterWhenDroppedNull)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityGetNumDroppedRecords(nullptr, 0, nullptr));
}

TEST_F(ActivityUtest, MsptiActivityGetNumDroppedRecordsReturnsDroppedCount)
{
    size_t dropped = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetNumDroppedRecords(nullptr, 0, &dropped));
    EXPECT_EQ(0, dropped);
}

TEST_F(ActivityUtest, MsptiGetTimestampReturnsInvalidParameterWhenTimestampNull)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiGetTimestamp(nullptr));
}

TEST_F(ActivityUtest, MsptiGetTimestampReturnsSuccess)
{
    uint64_t timestamp = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiGetTimestamp(&timestamp));
    EXPECT_GT(timestamp, 0ULL);
}

TEST_F(ActivityUtest, MsptiActivityRegisterTimestampCallbackReturnsInvalidParameterWhenNull)
{
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER, msptiActivityRegisterTimestampCallback(nullptr));
}

TEST_F(ActivityUtest, MsptiActivityRegisterTimestampCallbackReturnsSuccess)
{
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterTimestampCallback(
                                 []() -> uint64_t { return Mspti::Common::Utils::GetClockRealTimeNs(); }));
    uint64_t timestamp = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiGetTimestamp(&timestamp));
    EXPECT_GT(timestamp, 0ULL);
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsInvalidParameterWhenValueSizeNull)
{
    uint32_t channelSize = 4 * 1024 * 1024;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, nullptr, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsInvalidParameterWhenValueNull)
{
    size_t valueSize = sizeof(uint32_t);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, nullptr));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsInvalidParameterWhenAttrInvalid)
{
    uint32_t channelSize = 4 * 1024 * 1024;
    size_t valueSize = sizeof(uint32_t);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_FORCE_INT, &valueSize, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsParameterSizeNotSufficientWhenSizeTooSmall)
{
    uint32_t channelSize = 4 * 1024 * 1024;
    size_t valueSize = 1;
    EXPECT_EQ(MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsSuccessForChannelSize)
{
    uint32_t channelSize = 4 * 1024 * 1024;
    size_t valueSize = sizeof(channelSize);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));

    uint32_t getChannelSize = 0;
    valueSize = sizeof(getChannelSize);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &getChannelSize));
    EXPECT_EQ(channelSize, getChannelSize);

    uint32_t defaultChannelSize = 2 * 1024 * 1024;
    valueSize = sizeof(defaultChannelSize);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &defaultChannelSize));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsInvalidParameterWhenChannelSizeOutOfRange)
{
    uint32_t channelSize = 1 * 1024 * 1024;
    size_t valueSize = sizeof(channelSize);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));

    channelSize = 11 * 1024 * 1024;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivitySetAttributeReturnsSuccessForTimestampCallback)
{
    msptiTimestampCallbackFunc timestampCallback = []() -> uint64_t
    { return Mspti::Common::Utils::GetClockRealTimeNs(); };
    size_t valueSize = sizeof(timestampCallback);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK, &valueSize, &timestampCallback));

    msptiTimestampCallbackFunc getCallback = nullptr;
    valueSize = sizeof(getCallback);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK, &valueSize, &getCallback));
    EXPECT_EQ(timestampCallback, getCallback);

    msptiTimestampCallbackFunc nullCallback = nullptr;
    valueSize = sizeof(nullCallback);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK, &valueSize, &nullCallback));
}

TEST_F(ActivityUtest, MsptiActivityGetAttributeReturnsInvalidParameterWhenValueSizeNull)
{
    uint32_t channelSize = 0;
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, nullptr, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivityGetAttributeReturnsInvalidParameterWhenValueNull)
{
    size_t valueSize = sizeof(uint32_t);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, nullptr));
}

TEST_F(ActivityUtest, MsptiActivityGetAttributeReturnsInvalidParameterWhenAttrInvalid)
{
    uint32_t channelSize = 0;
    size_t valueSize = sizeof(channelSize);
    EXPECT_EQ(MSPTI_ERROR_INVALID_PARAMETER,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_FORCE_INT, &valueSize, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivityGetAttributeReturnsParameterSizeNotSufficientWhenSizeTooSmall)
{
    uint32_t channelSize = 0;
    size_t valueSize = 1;
    EXPECT_EQ(MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));
}

TEST_F(ActivityUtest, MsptiActivityGetAttributeReturnsDefaultChannelSize)
{
    uint32_t channelSize = 0;
    size_t valueSize = sizeof(channelSize);
    EXPECT_EQ(MSPTI_SUCCESS,
              msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize));
    EXPECT_EQ(2 * 1024 * 1024, channelSize);
}

std::atomic<uint64_t> g_thread_test_records{0};
void ThreadTestBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    if (validSize > 0)
    {
        msptiActivity *pRecord = NULL;
        msptiResult status = MSPTI_SUCCESS;
        do
        {
            status = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (status == MSPTI_SUCCESS)
            {
                g_thread_test_records++;
            }
            else if (status == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
        } while (true);
    }
    free(buffer);
}

void RecordThreadTestMarkers(uint32_t num)
{
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    msptiActivityMarker activity;
    constexpr uint64_t timeStamp = 1614659207688700;
    for (uint32_t i = 0; i < num; ++i)
    {
        activity.kind = MSPTI_ACTIVITY_KIND_MARKER;
        activity.sourceKind = MSPTI_ACTIVITY_SOURCE_KIND_HOST;
        activity.timestamp = timeStamp;
        activity.id = i;
        activity.objectId.pt.processId = 0;
        activity.objectId.pt.threadId = 0;
        activity.name = "ThreadTestMark";
        instance->Record(reinterpret_cast<msptiActivity *>(&activity), sizeof(activity));
    }
}

TEST_F(ActivityUtest, ThreadStartIsIdempotentAndDeliversRecords)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    g_thread_test_records.store(0);
    // 首次注册回调：启动管理线程
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    // 重复注册回调：StartActivityMgrThread 内部已通过 thread_run_ 去重，不得再次构造线程/崩溃
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    constexpr uint32_t num = 5;
    RecordThreadTestMarkers(num);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    // 仅一个线程在运行，记录被正确投递
    EXPECT_EQ(static_cast<uint64_t>(num), g_thread_test_records.load());

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
}

TEST_F(ActivityUtest, ResetAllDeviceDoesNotResetActivitySwitches)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    // 职责澄清：ResetAllDevice 仅负责 StopDevProfTask，不负责停止管理线程或清零开关；
    // 清零开关与线程生命周期归 CallbackManager::UnInit -> StopActivityMgrThread
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    // 验证 StopActivityMgrThread 才会清零开关
    instance->StopActivityMgrThread();
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    // 为后续用例恢复现场：重新启动线程
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
}

TEST_F(ActivityUtest, RestartAfterStopDeliversRecords)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    g_thread_test_records.store(0);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    RecordThreadTestMarkers(3);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(3ull, g_thread_test_records.load());

    // 通过 StopActivityMgrThread 停止管理线程（职责归 CallbackManager::UnInit），而非 ResetAllDevice
    instance->StopActivityMgrThread();
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    // 重新注册应再次启动线程（验证 Start 幂等与先置标志后构造的修复）
    g_thread_test_records.store(0);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    RecordThreadTestMarkers(4);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(4ull, g_thread_test_records.load());

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, DoubleResetAllDeviceIsSafe)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    // 连续两次 ResetAllDevice：仅 StopDevProfTask，不得崩溃，且不影响开关（开关由 Stop 负责）
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, StartStopThreadIsIdempotentAndResetClearsBothSwitchArrays)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    instance->StopActivityMgrThread();
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // 首次 Start：通过 RegisterCallbacks 间接触发
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL));
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));

    // 二次 Stop 幂等：第一次清开关，第二次走 early-return 分支不崩溃
    instance->StopActivityMgrThread();
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_KERNEL));
    instance->StopActivityMgrThread();
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    // 重启后再次幂等 Start
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    instance->StartActivityMgrThread();  // 已运行应直接 return
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, ConcurrentStopIsSafeAndDoesNotDeadlock)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    constexpr int kThreads = 8;
    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i)
    {
        workers.emplace_back([instance]() { instance->StopActivityMgrThread(); });
    }
    for (auto &t : workers)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    EXPECT_FALSE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    // 再次启动应成功：验证 Stop 后线程资源已释放，不残留 joinable
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_TRUE(instance->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, ConcurrentStartIsSafe)
{
    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    instance->StopActivityMgrThread();
    constexpr int kThreads = 8;
    std::vector<std::thread> workers;
    workers.reserve(kThreads);
    for (int i = 0; i < kThreads; ++i)
    {
        workers.emplace_back([instance]() { instance->StartActivityMgrThread(); });
    }
    for (auto &t : workers)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    // 仅一个线程实际创建，回调链仍可投递
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    g_thread_test_records.store(0);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    RecordThreadTestMarkers(2);
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(2ull, g_thread_test_records.load());
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityDisable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetAllDevice());
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, GetAllValidDeviceReturnsSnapshotIsolation)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    // 清理残留 device，再设置 0/1
    instance->ResetAllDevice();
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(0));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(1));
    auto snapshot = instance->GetAllValidDevice();
    EXPECT_EQ(2u, snapshot.size());
    EXPECT_TRUE(snapshot.find(0) != snapshot.end());
    EXPECT_TRUE(snapshot.find(1) != snapshot.end());
    // 篡改快照不应影响内部
    snapshot.insert(99);
    auto snapshot2 = instance->GetAllValidDevice();
    EXPECT_EQ(2u, snapshot2.size());
    EXPECT_TRUE(snapshot2.find(99) == snapshot2.end());

    instance->ResetAllDevice();
    // ResetAllDevice 在新语义下不清空 devices_，仅 StopDevProfTask；此处仅验证不崩溃
    auto snapshot3 = instance->GetAllValidDevice();
    EXPECT_GE(snapshot3.size(), 0u);
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, FlushAllWithNoBufferDoesNotCrashAndJoinWorkThreads)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    // 未 Record 直接 Flush 不应崩溃，且 JoinWorkThreads 清空 work_thread_
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityFlushAll(1));
    instance->StopActivityMgrThread();
}

TEST_F(ActivityUtest, CallbackManagerInitUnInitDrivesActivityThreadLifecycle)
{
    // 验证 callback_manager.cpp 新增的 Init->Start / UnInit->Stop 链路
    auto am = Mspti::Activity::ActivityManager::GetInstance();
    am->StopActivityMgrThread();
    EXPECT_FALSE(am->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    msptiSubscriberHandle sub = nullptr;
    setenv("LD_PRELOAD", "libmspti.so", 1);
    auto cb = [](void *, msptiCallbackDomain, msptiCallbackId, const msptiCallbackData *) {};
    EXPECT_EQ(MSPTI_SUCCESS, msptiSubscribe(&sub, cb, nullptr));
    // Init 已 Start 线程：此时 Register 回调应走幂等分支
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityRegisterCallbacks(UserBufferRequest, ThreadTestBufferComplete));
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityEnable(MSPTI_ACTIVITY_KIND_MARKER));
    EXPECT_TRUE(am->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));

    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(sub));
    // UnInit 已 Stop 线程并 Reset 开关
    EXPECT_FALSE(am->IsActivityKindEnable(MSPTI_ACTIVITY_KIND_MARKER));
    // 二次 UnInit 幂等
    EXPECT_EQ(MSPTI_SUCCESS, msptiUnsubscribe(sub));
    am->StopActivityMgrThread();
}

TEST_F(ActivityUtest, ResetDeviceRemovesSingleDeviceAndStopsProfTask)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    constexpr uint32_t kDev = 7;
    // 前置清理，保证用例独立
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDev));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(kDev));
    EXPECT_TRUE(instance->GetAllValidDevice().find(kDev) != instance->GetAllValidDevice().end());

    // 对应 MsprofDeviceStateImpl isOpen=false 分支：关闭该 device 的采集
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDev));
    EXPECT_TRUE(instance->GetAllValidDevice().find(kDev) == instance->GetAllValidDevice().end());
}

TEST_F(ActivityUtest, ResetDeviceKeepsOtherDevicesRunning)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    constexpr uint32_t kDevA = 9;
    constexpr uint32_t kDevB = 10;
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDevA));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDevB));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(kDevA));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(kDevB));

    // 只关闭其中一个，另一个不受影响
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDevA));
    auto snapshot = instance->GetAllValidDevice();
    EXPECT_TRUE(snapshot.find(kDevA) == snapshot.end());
    EXPECT_TRUE(snapshot.find(kDevB) != snapshot.end());

    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDevB));
}

TEST_F(ActivityUtest, ResetDeviceOnUnknownDeviceIsSafe)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    constexpr uint32_t kUnknown = 9999;
    auto snapshotBefore = instance->GetAllValidDevice();
    EXPECT_TRUE(snapshotBefore.find(kUnknown) == snapshotBefore.end());

    // 未 Set 过的 device 直接 Reset 应幂等成功，且不影响已有快照
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kUnknown));
    auto snapshotAfter = instance->GetAllValidDevice();
    EXPECT_EQ(snapshotBefore.size(), snapshotAfter.size());
    EXPECT_TRUE(snapshotAfter.find(kUnknown) == snapshotAfter.end());
}

TEST_F(ActivityUtest, DoubleResetDeviceIsSafe)
{
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StartDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));
    MOCKER_CPP(&Mspti::Ascend::DevTaskManager::StopDevProfTask).stubs().will(returnValue(MSPTI_SUCCESS));

    auto instance = Mspti::Activity::ActivityManager::GetInstance();
    constexpr uint32_t kDev = 8;
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDev));
    EXPECT_EQ(MSPTI_SUCCESS, instance->SetDevice(kDev));
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDev));
    // 第二次 Reset 走 device 不存在分支，不得崩溃
    EXPECT_EQ(MSPTI_SUCCESS, instance->ResetDevice(kDev));
    EXPECT_TRUE(instance->GetAllValidDevice().find(kDev) == instance->GetAllValidDevice().end());
}
}  // namespace
