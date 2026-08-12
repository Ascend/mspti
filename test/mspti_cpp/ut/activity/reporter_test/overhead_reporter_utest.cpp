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
#include <atomic>
#include <cstdio>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/reporter/overhead_reporter.h"
#include "csrc/common/utils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
std::atomic_uint32_t g_overheadRecord{0};
std::atomic<msptiActivityOverheadKind> g_overheadKind{MSPTI_ACTIVITY_OVERHEAD_UNKNOWN};
std::atomic<msptiActivityObjectKind> g_objectKind{MSPTI_ACTIVITY_OBJECT_UNKNOWN};
std::atomic<uint32_t> g_objectProcessId{0};
std::atomic<uint32_t> g_objectThreadId{0};
std::atomic<uint32_t> g_objectDeviceId{0};
std::atomic<uint32_t> g_objectStreamId{0};

void ResetOverheadCounters()
{
    g_overheadRecord = 0;
    g_overheadKind = MSPTI_ACTIVITY_OVERHEAD_UNKNOWN;
    g_objectKind = MSPTI_ACTIVITY_OBJECT_UNKNOWN;
    g_objectProcessId = 0;
    g_objectThreadId = 0;
    g_objectDeviceId = 0;
    g_objectStreamId = 0;
}

void UserBufferRequest(uint8_t **buffer, size_t *size, size_t *maxNumRecords)
{
    constexpr uint32_t bufSize = 2 * 1024 * 1024;
    *maxNumRecords = 0;
    *buffer = static_cast<uint8_t *>(malloc(bufSize));
    if (*buffer == nullptr)
    {
        *size = 0;
        return;
    }
    *size = bufSize;
}

void ActivityParser(msptiActivity *pRecord)
{
    if (pRecord->kind == MSPTI_ACTIVITY_KIND_OVERHEAD)
    {
        auto *overhead = reinterpret_cast<msptiActivityOverhead *>(pRecord);
        g_overheadRecord++;
        g_overheadKind.store(overhead->overheadKind);
        g_objectKind.store(overhead->objectKind);
        switch (overhead->objectKind)
        {
            case MSPTI_ACTIVITY_OBJECT_DEVICE:
                g_objectDeviceId.store(overhead->objectId.ds.deviceId);
                g_objectStreamId.store(overhead->objectId.ds.streamId);
                break;
            case MSPTI_ACTIVITY_OBJECT_PROCESS:
            case MSPTI_ACTIVITY_OBJECT_THREAD:
                g_objectProcessId.store(overhead->objectId.pt.processId);
                g_objectThreadId.store(overhead->objectId.pt.threadId);
                break;
            default:
                break;
        }
    }
}

void UserBufferComplete(uint8_t *buffer, size_t size, size_t validSize)
{
    if (validSize > 0)
    {
        msptiActivity *pRecord = nullptr;
        msptiResult ret = MSPTI_SUCCESS;
        do
        {
            ret = msptiActivityGetNextRecord(buffer, validSize, &pRecord);
            if (ret == MSPTI_SUCCESS)
            {
                ActivityParser(pRecord);
            }
            else if (ret == MSPTI_ERROR_MAX_LIMIT_REACHED)
            {
                break;
            }
            else
            {
                printf("[ERROR] GetNextRecord failed, result: %d\n", ret);
                break;
            }
        } while (true);
    }
    free(buffer);
}

class OverheadReporterUtest : public testing::Test
{
   protected:
    virtual void SetUp()
    {
        ResetOverheadCounters();
        Mspti::Activity::ActivityManager::GetInstance()->RegisterCallbacks(UserBufferRequest, UserBufferComplete);
        GlobalMockObject::verify();
    }
    virtual void TearDown() {}
};

TEST_F(OverheadReporterUtest, ShouldNotReportOverheadWhenKindDisabled)
{
    auto *manager = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, manager->UnRegister(MSPTI_ACTIVITY_KIND_OVERHEAD));
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    ResetOverheadCounters();
    {
        Mspti::Reporter::OverheadRecord resourceRecord(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE,
                                                       MSPTI_ACTIVITY_OBJECT_THREAD);
        Mspti::Reporter::OverheadRecord flushRecord(MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH,
                                                    MSPTI_ACTIVITY_OBJECT_THREAD);
    }
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    EXPECT_EQ(0U, g_overheadRecord.load());
}

TEST_F(OverheadReporterUtest, ShouldRetSuccessWhenReportOverheadActivity)
{
    auto *manager = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, manager->UnRegister(MSPTI_ACTIVITY_KIND_OVERHEAD));
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    ResetOverheadCounters();
    EXPECT_EQ(MSPTI_SUCCESS, manager->Register(MSPTI_ACTIVITY_KIND_OVERHEAD));
    {
        Mspti::Reporter::OverheadRecord resourceRecord(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE,
                                                       MSPTI_ACTIVITY_OBJECT_DEVICE);
        Mspti::Reporter::OverheadRecord requestRecord(MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_REQUEST,
                                                      MSPTI_ACTIVITY_OBJECT_THREAD);
        Mspti::Reporter::OverheadRecord flushRecord(MSPTI_ACTIVITY_OVERHEAD_ACTIVITY_BUFFER_FLUSH,
                                                    MSPTI_ACTIVITY_OBJECT_THREAD);
    }
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    // 3 records created in the scope above, plus 1 buffer request record generated
    // when the first record triggers the activity buffer initialization.
    const uint32_t expectCnt = 4;
    EXPECT_EQ(expectCnt, g_overheadRecord.load());
    EXPECT_EQ(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE, g_overheadKind.load());
    EXPECT_EQ(MSPTI_ACTIVITY_OBJECT_DEVICE, g_objectKind.load());
    EXPECT_EQ(Mspti::Common::Utils::GetPid(), g_objectProcessId.load());
    EXPECT_EQ(Mspti::Common::Utils::GetTid(), g_objectThreadId.load());
}

TEST_F(OverheadReporterUtest, ShouldRetSuccessWhenReportDeviceOverhead)
{
    auto *manager = Mspti::Activity::ActivityManager::GetInstance();
    EXPECT_EQ(MSPTI_SUCCESS, manager->UnRegister(MSPTI_ACTIVITY_KIND_OVERHEAD));
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    ResetOverheadCounters();
    EXPECT_EQ(MSPTI_SUCCESS, manager->Register(MSPTI_ACTIVITY_KIND_OVERHEAD));
    constexpr uint32_t deviceId = 7;
    {
        Mspti::Reporter::OverheadRecord deviceRecord(MSPTI_ACTIVITY_OVERHEAD_MSPTI_RESOURCE,
                                                     MSPTI_ACTIVITY_OBJECT_DEVICE);
        deviceRecord.SetDeviceId(deviceId);
    }
    EXPECT_EQ(MSPTI_SUCCESS, manager->FlushAll());
    EXPECT_GE(g_overheadRecord.load(), 1U);
    EXPECT_EQ(MSPTI_ACTIVITY_OBJECT_DEVICE, g_objectKind.load());
    EXPECT_EQ(deviceId, g_objectDeviceId.load());
    EXPECT_EQ(MSPTI_INVALID_STREAM_ID, g_objectStreamId.load());
}

TEST_F(OverheadReporterUtest, ShouldGetOverheadStructSizeForOverheadKind)
{
    size_t size = 0;
    EXPECT_EQ(MSPTI_SUCCESS, msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_OVERHEAD, 0, &size));
    EXPECT_EQ(sizeof(msptiActivityOverhead), size);
}
}  // namespace
