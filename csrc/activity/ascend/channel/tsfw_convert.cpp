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

#include "csrc/activity/ascend/channel/tsfw_convert.h"

#include <functional>

namespace Mspti
{
namespace Convert
{
namespace
{
template <typename T>
bool TransBasicTsfwImpl(const void *const buffer, StepTraceBasic &stepTraceData)
{
    auto oriData = reinterpret_cast<const T *>(buffer);
    stepTraceData.tsTraceHead = oriData->tsTraceHead;
    stepTraceData.timestamp = oriData->timestamp;
    stepTraceData.indexId = oriData->indexId;
    stepTraceData.modelId = oriData->modelId;
    stepTraceData.tagId = oriData->tagId;
    stepTraceData.streamId = oriData->streamId;
    stepTraceData.taskId = static_cast<uint32_t>(oriData->taskId);
    return true;
}

bool TransBasicTsfw(const void *const buffer, StepTraceBasic &stepTraceData)
{
    return TransBasicTsfwImpl<StepTrace>(buffer, stepTraceData);
}

bool TransV6Tsfw(const void *const buffer, StepTraceBasic &stepTraceData)
{
    return TransBasicTsfwImpl<StepTraceV6>(buffer, stepTraceData);
}
}  // namespace

TsfwConvert::TransFunc TsfwConvert::GetTransFunc(uint32_t deviceId, Common::PlatformType chipType) const
{
    switch (chipType)
    {
        case Common::PlatformType::CHIP_V6:
            return TransV6Tsfw;
        default:
            return TransBasicTsfw;
    }
}

size_t TsfwConvert::GetStructSize(uint32_t deviceId, Common::PlatformType chipType) const
{
    switch (chipType)
    {
        case Common::PlatformType::CHIP_V6:
            return sizeof(StepTraceV6);
        default:
            return sizeof(StepTrace);
    }
}

TsfwConvert &TsfwConvert::GetInstance()
{
    static TsfwConvert instance;
    return instance;
}
}  // namespace Convert
}  // namespace Mspti
