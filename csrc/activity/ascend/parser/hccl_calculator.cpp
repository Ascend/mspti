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
#include "csrc/activity/ascend/parser/hccl_calculator.h"

#include <atomic>
#include <cinttypes>
#include <cstring>
#include <functional>
#include <unordered_map>

#include "csrc/activity/ascend/entity/hccl_op_desc.h"
#include "csrc/common/inject/hccl_inject.h"
#include "csrc/common/plog_manager.h"

namespace Mspti
{
namespace Parser
{
using CaculatorFunc = std::function<msptiResult(HcclOpDesc* hcclOpDesc)>;

const static std::unordered_map<HcclDataType, uint8_t> dataType2Size{{HCCL_DATA_TYPE_INT8, sizeof(int8_t)},
                                                                     {HCCL_DATA_TYPE_INT16, sizeof(int16_t)},
                                                                     {HCCL_DATA_TYPE_INT32, sizeof(int32_t)},
                                                                     {HCCL_DATA_TYPE_FP16, 2},
                                                                     {HCCL_DATA_TYPE_FP32, sizeof(float)},
                                                                     {HCCL_DATA_TYPE_INT64, sizeof(int64_t)},
                                                                     {HCCL_DATA_TYPE_UINT64, sizeof(uint64_t)},
                                                                     {HCCL_DATA_TYPE_UINT8, sizeof(uint8_t)},
                                                                     {HCCL_DATA_TYPE_UINT16, sizeof(uint16_t)},
                                                                     {HCCL_DATA_TYPE_UINT32, sizeof(uint32_t)},
                                                                     {HCCL_DATA_TYPE_FP64, 8},
                                                                     {HCCL_DATA_TYPE_BFP16, 2},
                                                                     {HCCL_DATA_TYPE_INT128, 16}};

namespace
{
void LogBandwidthFailure(const char* reason)
{
    static std::atomic<uint64_t> bandwidthFailureCount{0};
    MSPTI_LOGW_EVERY_N_TOTAL(Common::LOG_EVERY_N_FAILURE, bandwidthFailureCount,
                             "%s, cannot calculate bandwidth, failed total: %" PRIu64, reason);
}
}  // namespace

msptiResult CalculateP2pBandWidth(HcclOpDesc* hcclOpDesc)
{
    auto p2pOpDesc = static_cast<P2pOpDesc*>(hcclOpDesc);
    const auto& dataType = dataType2Size.find(p2pOpDesc->dataType);
    if (dataType == dataType2Size.end())
    {
        LogBandwidthFailure("unknown p2p op type");
        return MSPTI_ERROR_INNER;
    }
    uint64_t dataSize = dataType->second * p2pOpDesc->count;
    uint64_t durationTime = hcclOpDesc->end - hcclOpDesc->start;
    hcclOpDesc->bandWidth = static_cast<double>(dataSize) / durationTime;
    return MSPTI_SUCCESS;
}

msptiResult CalculateAll2AllVBandWidth(HcclOpDesc* hcclOpDesc)
{
    auto all2AllVOpDesc = static_cast<All2AllVOpDesc*>(hcclOpDesc);
    if (all2AllVOpDesc->sendCounts == nullptr || all2AllVOpDesc->recvCounts == nullptr)
    {
        LogBandwidthFailure("sendCounts or recvCounts in op is nullptr");
        return MSPTI_ERROR_INNER;
    }
    const auto& sendType = dataType2Size.find(all2AllVOpDesc->sendType);
    if (sendType == dataType2Size.end())
    {
        LogBandwidthFailure("unknow all2Allv op sendType");
        return MSPTI_ERROR_INNER;
    }
    const auto& recvType = dataType2Size.find(all2AllVOpDesc->recvType);
    if (recvType == dataType2Size.end())
    {
        LogBandwidthFailure("unknow all2Allv op recvType");
        return MSPTI_ERROR_INNER;
    }
    uint64_t sendDataSize = 0;
    uint64_t recvDataSize = 0;
    const auto* sendCounts = reinterpret_cast<const uint64_t*>(all2AllVOpDesc->sendCounts);
    const auto* recvCounts = reinterpret_cast<const uint64_t*>(all2AllVOpDesc->recvCounts);
    for (uint32_t i = 0; i < all2AllVOpDesc->rankSize; i++)
    {
        sendDataSize += sendCounts[i] * sendType->second;
        recvDataSize += recvCounts[i] * recvType->second;
    }
    uint64_t durationTime = hcclOpDesc->end - hcclOpDesc->start;
    hcclOpDesc->bandWidth = static_cast<double>(std::max(sendDataSize, recvDataSize)) / durationTime;
    return MSPTI_SUCCESS;
}

msptiResult CalculateCollectiveBandWidth(HcclOpDesc* hcclOpDesc)
{
    auto collectiveOp = static_cast<All2AllVOpDesc*>(hcclOpDesc);
    const auto& it = dataType2Size.find(collectiveOp->dataType);
    if (it == dataType2Size.end())
    {
        LogBandwidthFailure("unknow collective op type");
        return MSPTI_ERROR_INNER;
    }
    uint64_t dataSize = it->second * collectiveOp->count * collectiveOp->rankSize;
    uint64_t durationTime = hcclOpDesc->end - hcclOpDesc->start;
    hcclOpDesc->bandWidth = static_cast<double>(dataSize) / durationTime;
    return MSPTI_SUCCESS;
}

msptiResult CalculateBatchSendRecvBandWidth(HcclOpDesc* hcclOpDesc)
{
    auto collectiveOp = static_cast<BatchSendRecvOp*>(hcclOpDesc);
    uint64_t dataSize = 0;
    for (const auto& sendOp : collectiveOp->batchSendRecvItem)
    {
        const auto& it = dataType2Size.find(sendOp.dataType);
        if (it == dataType2Size.end())
        {
            LogBandwidthFailure("unknow batchSendRecv op type");
            return MSPTI_ERROR_INNER;
        }
        dataSize += sendOp.count * it->second;
    }
    uint64_t durationTime = hcclOpDesc->end - hcclOpDesc->start;
    hcclOpDesc->bandWidth = static_cast<double>(dataSize) / durationTime;
    return MSPTI_SUCCESS;
}

static const std::unordered_map<std::string, CaculatorFunc> calculateFunc{
    {"HcclSend", CalculateP2pBandWidth},
    {"HcclRecv", CalculateP2pBandWidth},
    {"HcclAllReduce", CalculateP2pBandWidth},
    {"HcclBroadcast", CalculateP2pBandWidth},
    {"HcclAllGather", CalculateCollectiveBandWidth},
    {"HcclReduceScatter", CalculateCollectiveBandWidth},
    {"HcclScatter", CalculateCollectiveBandWidth},
    {"HcclReduce", CalculateP2pBandWidth},
    {"HcclAlltoAll", CalculateCollectiveBandWidth},
    {"HcclAlltoAllV", CalculateAll2AllVBandWidth},
    {"HcclBatchSendRecv", CalculateBatchSendRecvBandWidth}};

msptiResult HcclCalculator::CalculateBandWidth(HcclOpDesc* hcclOpDesc)
{
    if (hcclOpDesc == nullptr)
    {
        LogBandwidthFailure("hcclOpDesc is nullptr");
        return MSPTI_ERROR_INNER;
    }
    const auto& it = calculateFunc.find(hcclOpDesc->opName);
    if (it == calculateFunc.end())
    {
        LogBandwidthFailure("bandwidth calculation formula without corresponding operator");
        return MSPTI_ERROR_INNER;
    }
    if (hcclOpDesc->end <= hcclOpDesc->start)
    {
        LogBandwidthFailure("duration time is zero or start time is later than end time");
        return MSPTI_ERROR_INNER;
    }
    return it->second(hcclOpDesc);
}
}  // namespace Parser
}  // namespace Mspti
