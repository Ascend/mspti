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

// MSTX headers
#include "mstx/ms_tools_ext.h"

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
    for (int64_t i = shape.size() - 2; i >= 0; i--)
    {
        strides[i] = shape[i + 1] * strides[i + 1];
    }

    // 调用aclCreateTensor接口创建aclTensor
    *tensor = aclCreateTensor(shape.data(), shape.size(), dataType, strides.data(), 0, aclFormat::ACL_FORMAT_ND,
                              shape.data(), shape.size(), *deviceAddr);
    return 0;
}

int DoAclAdd(aclrtContext context, aclrtStream stream)
{
    auto ret = ACL_SUCCESS;

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
    // 创建self aclTensor
    ACL_CALL(CreateAclTensor(selfHostData, selfShape, &selfDeviceAddr, aclDataType::ACL_FLOAT, &self));
    // 创建other aclTensor
    ACL_CALL(CreateAclTensor(otherHostData, otherShape, &otherDeviceAddr, aclDataType::ACL_FLOAT, &other));
    // 创建alpha aclScalar
    alpha = aclCreateScalar(&alphaValue, aclDataType::ACL_FLOAT);
    CHECK_RET(alpha != nullptr, return ret);
    // 创建out aclTensor
    ACL_CALL(CreateAclTensor(outHostData, outShape, &outDeviceAddr, aclDataType::ACL_FLOAT, &out));

    uint64_t workspaceSize = 0;
    aclOpExecutor* executor;
    // 调用aclnnAdd第一段接口
    ACL_CALL(aclnnAddGetWorkspaceSize(self, other, alpha, out, &workspaceSize, &executor));
    // 根据第一段接口计算出的workspaceSize申请device内存
    void* workspaceAddr = nullptr;
    if (workspaceSize > 0)
    {
        ACL_CALL(aclrtMalloc(&workspaceAddr, workspaceSize, ACL_MEM_MALLOC_HUGE_FIRST));
    }
    ACL_CALL(aclnnAdd(workspaceAddr, workspaceSize, executor, stream));
    ACL_CALL(aclrtSynchronizeStream(stream));

    auto size = GetShapeSize(outShape);
    std::vector<float> resultData(size, 0);
    ACL_CALL(aclrtMemcpy(resultData.data(), resultData.size() * sizeof(resultData[0]), outDeviceAddr,
                         size * sizeof(float), ACL_MEMCPY_DEVICE_TO_HOST));
    for (int64_t i = 0; i < size; i++)
    {
        LOG_PRINT("result[%ld] is: %f\n", i, resultData[i]);
    }

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

void MstxCallback(void* pUserData, msptiCallbackDomain domain, msptiCallbackId callbackId,
                  const msptiCallbackData* pCallbackInfo)
{
    UserData* userData = (UserData*)pUserData;
    if (domain != MSPTI_CB_DOMAIN_RUNTIME)
    {
        return;
    }
    if (pCallbackInfo->callbackSite == MSPTI_API_ENTER)
    {
        LOG_PRINT("%s func enter\n", pCallbackInfo->functionName);
    }
    else if (pCallbackInfo->callbackSite == MSPTI_API_EXIT)
    {
        LOG_PRINT("%s func exit\n", pCallbackInfo->functionName);
    }
}

void SetUpMspti(aclrtContext* context, aclrtStream* stream)
{
    msptiSubscriberHandle* handle = InitMspti((void*)MstxCallback, nullptr);

    // 查询追踪会话是否仍在运行
    uint8_t isRunning = 0;
    msptiResult ret = msptiIsTracingSessionRunning(&isRunning);
    LOG_PRINT("[msptiIsTracingSessionRunning] ret: %s, isRunning: %u\n", GetResultCodeString(ret), isRunning);

    // 获取支持的callback domain列表
    size_t domainCount = 0;
    msptiDomainTable domainTable = nullptr;
    ret = msptiSupportedDomains(&domainCount, &domainTable);
    LOG_PRINT("[msptiSupportedDomains] ret: %s, domainCount: %zu\n", GetResultCodeString(ret), domainCount);
    for (size_t i = 0; i < domainCount; i++)
    {
        LOG_PRINT("  supported domain[%zu]: %u\n", i, domainTable[i]);
    }

    // 开启所有domain的所有callback
    ret = msptiEnableAllDomains(1, *handle);
    LOG_PRINT("[msptiEnableAllDomains] ret: %s\n", GetResultCodeString(ret));

    // 获取指定domain和callbackId对应的回调名称
    const char* name = nullptr;
    ret = msptiGetCallbackName(MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MEMCPY, &name);
    LOG_PRINT("[msptiGetCallbackName] ret: %s, name: %s\n", GetResultCodeString(ret), name != nullptr ? name : "null");

    // 查询指定回调当前的开启/关闭状态
    uint32_t enable = 0;
    ret = msptiGetCallbackState(&enable, *handle, MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MEMCPY);
    LOG_PRINT("[msptiGetCallbackState] ret: %s, domain: %u, callbackId: %u, enable: %u\n", GetResultCodeString(ret),
              MSPTI_CB_DOMAIN_RUNTIME, MSPTI_CBID_RUNTIME_MEMCPY, enable);
    ret = msptiGetCallbackState(&enable, *handle, MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE);
    LOG_PRINT("[msptiGetCallbackState] ret: %s, domain: %u, callbackId: %u, enable: %u\n", GetResultCodeString(ret),
              MSPTI_CB_DOMAIN_HCCL, MSPTI_CBID_HCCL_ALLREDUCE, enable);

    // 获取指定domain下已开启的callback ID列表
    msptiCallbackId callbackIdBuffer[MSPTI_CBID_HCCL_SIZE] = {};
    uint32_t bufferSize = MSPTI_CBID_HCCL_SIZE;
    uint32_t enabledCount = 0;
    ret = msptiGetEnabledCallbacks(*handle, MSPTI_CB_DOMAIN_HCCL, callbackIdBuffer, &bufferSize, &enabledCount);
    LOG_PRINT("[msptiGetEnabledCallbacks] ret: %s, domain: %u, enabledCount: %u\n", GetResultCodeString(ret),
              MSPTI_CB_DOMAIN_HCCL, enabledCount);
    for (uint32_t i = 0, writeCount = std::min(enabledCount, bufferSize); i < writeCount; i++)
    {
        const char* callbackName = nullptr;
        msptiGetCallbackName(MSPTI_CB_DOMAIN_HCCL, callbackIdBuffer[i], &callbackName);
        LOG_PRINT("  enabled callback[%u]: domain: %u, id: %u, name: %s\n", i, MSPTI_CB_DOMAIN_HCCL,
                  callbackIdBuffer[i], callbackName != nullptr ? callbackName : "null");
    }
}

int main()
{
    int32_t deviceId = 0;
    aclrtContext context;
    aclrtStream stream;
    ACL_CALL(Init(deviceId, &context, &stream));
    SetUpMspti(&context, &stream);
    DoAclAdd(context, stream);
    DeInitMspti();

    DeInit(deviceId, &context, &stream);
    return 0;
}
