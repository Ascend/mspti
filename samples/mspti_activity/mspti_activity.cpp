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

// System headers
#include <chrono>
#include <vector>

// ACL Header
#include "acl/acl.h"
#include "acl/acl_prof.h"

// ACLNN Header
#include "aclnnop/aclnn_add.h"

// MSPTI header
#include "common/helper_mspti.h"
#include "common/util_acl.h"
#include "mspti.h"

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

// 自定义时间戳回调函数，返回纳秒级时间戳
uint64_t TimestampCallback()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

void ShowMsptiApiCallResult()
{
    // 获取MSPTI API版本号
    uint32_t version = 0;
    msptiResult ret = msptiGetVersion(&version);
    LOG_PRINT("msptiGetVersion result: %d (%s), version: %u\n", ret, GetResultCodeString(ret), version);

    // 获取指定Activity Kind对应的结构体大小
    size_t structSize = 0;
    ret = msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_KERNEL, version, &structSize);
    LOG_PRINT("msptiActivityGetStructSize(MSPTI_ACTIVITY_KIND_KERNEL) result: %d (%s), structSize: %zu\n", ret,
              GetResultCodeString(ret), structSize);

    // 获取已使能的Activity Kind列表
    msptiActivityKind enabledKinds[MSPTI_ACTIVITY_KIND_COUNT] = {};
    uint32_t bufferSize = MSPTI_ACTIVITY_KIND_COUNT;
    uint32_t enabledKindsCount = 0;
    ret = msptiActivityGetEnabledKinds(subscriber, enabledKinds, &bufferSize, &enabledKindsCount);
    LOG_PRINT("msptiActivityGetEnabledKinds result: %d (%s), enabledKindsCount: %u\n", ret, GetResultCodeString(ret),
              enabledKindsCount);
    for (uint32_t i = 0; i < enabledKindsCount; i++)
    {
        LOG_PRINT("  enabled kind[%u]: %s\n", i, GetActivityKindString(enabledKinds[i]));
    }

    // 获取因缓冲区空间不足而丢弃的Record数量
    size_t dropped = 0;
    ret = msptiActivityGetNumDroppedRecords(nullptr, 0, &dropped);
    LOG_PRINT("msptiActivityGetNumDroppedRecords result: %d (%s), dropped: %zu\n", ret, GetResultCodeString(ret),
              dropped);

    // 获取MSPTI时间戳
    uint64_t timestamp = 0;
    ret = msptiGetTimestamp(&timestamp);
    LOG_PRINT("msptiGetTimestamp result: %d (%s), timestamp: %lu\n", ret, GetResultCodeString(ret), timestamp);
}

void ShowActivityAttribute()
{
    // 获取Activity属性：Channel Buffer大小
    uint32_t channelSize = 0;
    size_t valueSize = sizeof(channelSize);
    msptiResult ret = msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize);
    LOG_PRINT("msptiActivityGetAttribute(CHANNEL_BUFFER_SIZE) result: %d (%s), channelSize: %u\n", ret,
              GetResultCodeString(ret), channelSize);

    // 设置Activity属性：Channel Buffer大小，取值范围为[2MB, 10MB]
    channelSize = 4 * 1024 * 1024;
    valueSize = sizeof(channelSize);
    ret = msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize);
    LOG_PRINT("msptiActivitySetAttribute(CHANNEL_BUFFER_SIZE) result: %d (%s), channelSize: %u\n", ret,
              GetResultCodeString(ret), channelSize);
    if (ret == MSPTI_SUCCESS)
    {
        // 重新获取设置后的Channel Buffer大小
        valueSize = sizeof(channelSize);
        ret = msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_CHANNEL_BUFFER_SIZE, &valueSize, &channelSize);
        LOG_PRINT("msptiActivityGetAttribute(CHANNEL_BUFFER_SIZE) after set result: %d (%s), channelSize: %u\n", ret,
                  GetResultCodeString(ret), channelSize);
    }

    // 获取Activity属性：时间戳回调函数
    msptiTimestampCallbackFunc funcTimestamp = nullptr;
    valueSize = sizeof(funcTimestamp);
    ret = msptiActivityGetAttribute(MSPTI_ACTIVITY_ATTR_TIMESTAMP_CALLBACK, &valueSize, &funcTimestamp);
    LOG_PRINT("msptiActivityGetAttribute(TIMESTAMP_CALLBACK) result: %d (%s)\n", ret, GetResultCodeString(ret));

    // 错误attr入参示例：非法属性
    uint32_t invalidValue = 0;
    valueSize = sizeof(invalidValue);
    ret = msptiActivitySetAttribute(MSPTI_ACTIVITY_ATTR_FORCE_INT, &valueSize, &invalidValue);
    LOG_PRINT("msptiActivitySetAttribute(FORCE_INT) result: %d (%s)\n", ret, GetResultCodeString(ret));
}

void SetUpMspti()
{
    // 初始化订阅mspti
    InitMspti(nullptr, nullptr);

    // 演示Activity属性设置与获取
    ShowActivityAttribute();

    // 注册时间戳回调，需在所有Activity Kind使能之前调用
    msptiResult ret = msptiActivityRegisterTimestampCallback(TimestampCallback);
    LOG_PRINT("msptiActivityRegisterTimestampCallback result: %d (%s)\n", ret, GetResultCodeString(ret));

    // 开启mspti数据采集开关
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_OVERHEAD);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_KERNEL);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMCPY);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMORY);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_MEMSET);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_ACL_API);
    msptiActivityEnable(MSPTI_ACTIVITY_KIND_RUNTIME_API);
}

int main()
{
    int32_t deviceId = 0;
    aclrtContext context;
    aclrtStream stream;
    Init(deviceId, &context, &stream);
    SetUpMspti();
    ShowMsptiApiCallResult();
    DoAclAdd(context, stream);
    ShowMsptiApiCallResult();
    DeInitMspti();

    DeInit(deviceId, &context, &stream);
    return 0;
}
