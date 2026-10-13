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

// System headers
#include <algorithm>
#include <vector>

// ACL headers
#include "acl/acl.h"
#include "aclnnop/aclnn_add.h"

// MSPTI headers
#include "common/helper_mspti.h"
#include "common/util_acl.h"
#include "mspti.h"

namespace
{
int32_t gCallbackCount = 0;

int64_t GetShapeSize(const std::vector<int64_t>& shape)
{
    int64_t shapeSize = 1;
    for (auto i : shape)
    {
        shapeSize *= i;
    }
    return shapeSize;
}

template <typename T>
int CreateAclTensor(const std::vector<T>& hostData, const std::vector<int64_t>& shape, void** deviceAddr,
                    aclDataType dataType, aclTensor** tensor)
{
    auto size = GetShapeSize(shape) * sizeof(T);
    ACL_CALL(aclrtMalloc(deviceAddr, size, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CALL(aclrtMemcpy(*deviceAddr, size, hostData.data(), size, ACL_MEMCPY_HOST_TO_DEVICE));

    std::vector<int64_t> strides(shape.size(), 1);
    for (int64_t i = static_cast<int64_t>(shape.size()) - 2; i >= 0; i--)
    {
        strides[i] = shape[i + 1] * strides[i + 1];
    }

    *tensor = aclCreateTensor(shape.data(), shape.size(), dataType, strides.data(), 0, aclFormat::ACL_FORMAT_ND,
                              shape.data(), shape.size(), *deviceAddr);
    return 0;
}

int DoAclAdd(aclrtStream stream)
{
    std::vector<int64_t> selfShape = {4, 2};
    std::vector<int64_t> otherShape = {4, 2};
    std::vector<int64_t> outShape = {4, 2};
    void* selfDeviceAddr = nullptr;
    void* otherDeviceAddr = nullptr;
    void* outDeviceAddr = nullptr;
    aclTensor* self = nullptr;
    aclTensor* other = nullptr;
    aclScalar* alpha = nullptr;
    aclTensor* out = nullptr;
    std::vector<float> selfHostData = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<float> otherHostData = {1, 1, 1, 2, 2, 2, 3, 3};
    std::vector<float> outHostData = {0, 0, 0, 0, 0, 0, 0, 0};
    float alphaValue = 1.2f;
    ACL_CALL(CreateAclTensor(selfHostData, selfShape, &selfDeviceAddr, aclDataType::ACL_FLOAT, &self));
    ACL_CALL(CreateAclTensor(otherHostData, otherShape, &otherDeviceAddr, aclDataType::ACL_FLOAT, &other));
    alpha = aclCreateScalar(&alphaValue, aclDataType::ACL_FLOAT);
    CHECK_RET(alpha != nullptr, return -1);
    ACL_CALL(CreateAclTensor(outHostData, outShape, &outDeviceAddr, aclDataType::ACL_FLOAT, &out));

    uint64_t workspaceSize = 0;
    aclOpExecutor* executor = nullptr;
    ACL_CALL(aclnnAddGetWorkspaceSize(self, other, alpha, out, &workspaceSize, &executor));
    void* workspaceAddr = nullptr;
    if (workspaceSize > 0)
    {
        ACL_CALL(aclrtMalloc(&workspaceAddr, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST));
    }
    ACL_CALL(aclnnAdd(workspaceAddr, workspaceSize, executor, stream));
    ACL_CALL(aclrtSynchronizeStream(stream));

    aclDestroyTensor(self);
    aclDestroyTensor(other);
    aclDestroyScalar(alpha);
    aclDestroyTensor(out);

    aclrtFree(selfDeviceAddr);
    aclrtFree(otherDeviceAddr);
    aclrtFree(outDeviceAddr);
    if (workspaceSize > 0)
    {
        aclrtFree(workspaceAddr);
    }
    return 0;
}

// Counts dispatches to show attached vs detached phases.
void FinalizeDemoCallback(void* pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                          const msptiCallbackData* pCallbackInfo)
{
    (void)pUserData;
    (void)domain;
    (void)callbackId;
    gCallbackCount++;
    if (pCallbackInfo->callbackSite == MSPTI_API_ENTER)
    {
        LOG_PRINT("  [callback] %s enter\n", pCallbackInfo->functionName);
    }
    else if (pCallbackInfo->callbackSite == MSPTI_API_EXIT)
    {
        LOG_PRINT("  [callback] %s exit\n", pCallbackInfo->functionName);
    }
}

void PrintSessionState(const char* stage)
{
    uint8_t isRunning = 1;
    msptiResult ret = msptiIsTracingSessionRunning(&isRunning);
    LOG_PRINT("[%s] msptiIsTracingSessionRunning ret: %s, isRunning: %u, callbackCount: %d\n", stage,
              GetResultCodeString(ret), isRunning, gCallbackCount);
}

void SetUpMspti()
{
    msptiSubscriberHandle* handle = InitMspti((void*)FinalizeDemoCallback, nullptr);
    msptiResult ret = msptiEnableAllDomains(1, *handle);
    LOG_PRINT("[msptiEnableAllDomains] ret: %s\n", GetResultCodeString(ret));
    PrintSessionState("session start");
}
}  // namespace

int main()
{
    int32_t deviceId = 0;
    aclrtContext context;
    aclrtStream stream;
    ACL_CALL(Init(deviceId, &context, &stream));

    // 1) Start a tracing session and collect some work.
    SetUpMspti();
    LOG_PRINT(">>> Round 1: collect with callbacks attached");
    DoAclAdd(stream);
    PrintSessionState("after round 1");

    // 2) Detach MSPTI: interposed APIs degrade to pass-through. Note:
    //    msptiFinalize does not flush activity buffers; call
    //    msptiActivityFlushAll beforehand if collecting activity data.
    LOG_PRINT(">>> msptiFinalize");
    msptiResult ret = msptiFinalize();
    LOG_PRINT("[msptiFinalize] ret: %s\n", GetResultCodeString(ret));
    PrintSessionState("after msptiFinalize");

    // 3) The process keeps running: this round executes without any callback.
    LOG_PRINT(">>> Round 2: pass-through after finalize (no callback expected)");
    int32_t countAfterFinalize = gCallbackCount;
    DoAclAdd(stream);
    if (gCallbackCount != countAfterFinalize)
    {
        LOG_PRINT("Unexpected: callbacks still fired after msptiFinalize.\n");
    }
    PrintSessionState("after round 2");

    // 4) Start a new session; collection resumes.
    LOG_PRINT(">>> Round 3: start a new session");
    SetUpMspti();
    DoAclAdd(stream);
    PrintSessionState("after round 3");

    // 5) Finalize again: repeated calls are safe no-ops.
    ret = msptiFinalize();
    LOG_PRINT("[msptiFinalize again] ret: %s\n", GetResultCodeString(ret));
    PrintSessionState("end");

    DeInitMspti();
    DeInit(deviceId, &context, &stream);
    return 0;
}
