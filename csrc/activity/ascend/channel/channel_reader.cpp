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
#include "csrc/activity/ascend/channel/channel_reader.h"

#include <climits>

#include "csrc/activity/activity_manager.h"
#include "csrc/activity/ascend/channel/channel_data.h"
#include "csrc/activity/ascend/channel/soclog_convert.h"
#include "csrc/activity/ascend/channel/tsfw_convert.h"
#include "csrc/activity/ascend/parser/device_task_calculator.h"
#include "csrc/activity/ascend/parser/kernel_parser.h"
#include "csrc/activity/ascend/parser/parser_manager.h"
#include "csrc/common/inject/driver_inject.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"
#include "securec.h"

namespace Mspti
{
namespace Ascend
{
namespace Channel
{
ChannelReader::ChannelReader(uint32_t deviceId, AI_DRV_CHANNEL channelId) : deviceId_(deviceId), channelId_(channelId)
{
}

msptiResult ChannelReader::Init()
{
    hashId_ = std::hash<std::string>()(std::to_string(static_cast<int>(channelId_)) + std::to_string(deviceId_) +
                                       std::to_string(Common::Utils::GetClockMonotonicRawNs()));
    isInited_ = true;
    curPos_ = 0;
    return MSPTI_SUCCESS;
}

msptiResult ChannelReader::Uinit()
{
    isInited_ = false;
    curPos_ = 0;
    MSPTI_LOGI("Uinit channel reader, deviceId=%u, channelId=%d, totalSize=%lu", deviceId_, channelId_, totalSize_);
    return MSPTI_SUCCESS;
}

msptiResult ChannelReader::FlushDrvBuff()
{
    // 1. query flush size
    std::unique_lock<std::mutex> guard(flushMutex_);
    unsigned int flushSize = 0;
    const int32_t ret = HalProfDataFlush(deviceId_, channelId_, &flushSize);
    if (ret != MSPTI_SUCCESS)
    {
        MSPTI_LOGE("HalProfDataFlush failed, deviceId:%u, channelId:%u, ret:%d", deviceId_, channelId_, ret);
        return MSPTI_ERROR_INNER;
    }
    MSPTI_LOGI("Flush deviceId:%u, channelId:%u, flushSize:%u", deviceId_, channelId_, flushSize);
    // 2. wait flush finished
    if (flushSize == 0)
    {
        MSPTI_LOGI("No drv data need flush for deviceId:%u, channelId:%u", deviceId_, channelId_);
        return MSPTI_SUCCESS;
    }
    needWait_ = true;
    flushBufSize_ = flushSize;

    constexpr uint32_t maxWaitTime = 20;
    flushFlag_.wait_for(guard, std::chrono::milliseconds(maxWaitTime), [this] { return !this->needWait_; });
    return MSPTI_SUCCESS;
}

size_t ChannelReader::HashId() { return hashId_; }

void ChannelReader::SetChannelStopped() { isChannelStopped_ = true; }

bool ChannelReader::GetSchedulingStatus() const { return isScheduling_; }

void ChannelReader::SetSchedulingStatus(bool isScheduling) { isScheduling_ = isScheduling; }

msptiResult ChannelReader::Execute()
{
    isScheduling_ = false;
    int currLen = 0;
    std::unique_lock<std::mutex> guard(flushMutex_, std::defer_lock);
    while (isInited_ && !isChannelStopped_)
    {
        guard.lock();
        currLen = ProfChannelRead(deviceId_, channelId_, buffer_ + curPos_, MAX_BUFFER_SIZE - curPos_);
        CheckIfSendFlush(currLen);
        guard.unlock();
        if (currLen <= 0)
        {
            if (currLen < 0)
            {
                MSPTI_LOGE("Read data from driver failed.");
            }
            break;
        }
        auto uintCurrLen = static_cast<size_t>(currLen);
        if (uintCurrLen > (MAX_BUFFER_SIZE - curPos_))
        {
            MSPTI_LOGE("Read invalid data len [%zu] from driver", uintCurrLen);
            break;
        }
        size_t lastPos = TransDataToActivityBuffer(buffer_, curPos_ + uintCurrLen, deviceId_, channelId_);
        if (lastPos < curPos_ + uintCurrLen)
        {
            if (memcpy_s(buffer_, MAX_BUFFER_SIZE, buffer_ + lastPos, curPos_ + uintCurrLen - lastPos) != EOK)
            {
                MSPTI_LOGE("memcpy channel buff data failed, deviceId=%u, channelId=%d, totalSize=%lu", deviceId_,
                           channelId_, totalSize_);
                break;
            }
        }
        totalSize_ += uintCurrLen;
        curPos_ = curPos_ + uintCurrLen - lastPos;
    }
    return MSPTI_SUCCESS;
}

void ChannelReader::CheckIfSendFlush(int currLen)
{
    if (needWait_)
    {
        // Check for overflow, if curLen is very large, Sure to send flush finished
        if (flushCurSize_ > UINT_MAX - currLen)
        {
            SendFlushFinished();
        }
        else
        {
            flushCurSize_ += currLen;
            if (flushBufSize_ <= flushCurSize_ || currLen == 0)
            {
                SendFlushFinished();
            }
        }
    }
}

void ChannelReader::SendFlushFinished()
{
    needWait_ = false;
    flushCurSize_ = 0;
    flushBufSize_ = 0;
    flushFlag_.notify_all();
}

size_t ChannelReader::TransDataToActivityBuffer(char buffer[], size_t valid_size, uint32_t deviceId,
                                                AI_DRV_CHANNEL channelId)
{
    switch (channelId)
    {
        case PROF_CHANNEL_TS_FW:
            return TransTsFwData(buffer, valid_size, deviceId);
        case PROF_CHANNEL_STARS_SOC_LOG:
            return TransStarsLog(buffer, valid_size, deviceId);
        default:
            return 0;
    }
}

size_t ChannelReader::TransTsFwData(char buffer[], size_t valid_size, uint32_t deviceId)
{
    size_t pos = 0;
    static size_t logStructSize = Convert::TsfwConvert::GetInstance().GetStructSize(
        deviceId, Common::ContextManager::GetInstance()->GetChipType(deviceId));
    while (valid_size - pos >= logStructSize)
    {
        StepTraceBasic stepTrace;
        TsTrackHead* tsHead = reinterpret_cast<TsTrackHead*>(buffer + pos);
        MSPTI_LOGD("ts track data type is %d", tsHead->rptType);
        switch (tsHead->rptType)
        {
            case RPT_TYPE_STEP_TRACE:
                Convert::TsfwConvert::GetInstance().TransData(buffer, valid_size, deviceId, pos, stepTrace);
                Mspti::Parser::ParserManager::GetInstance()->ReportStepTrace(deviceId, &stepTrace);
                break;
            default:
                pos += logStructSize;
                break;
        }
    }
    return pos;
}

size_t ChannelReader::TransStarsLog(char buffer[], size_t valid_size, uint32_t deviceId)
{
    size_t pos = 0;
    HalLogData logData;
    static size_t logStructSize = Convert::SocLogConvert::GetInstance().GetStructSize(
        deviceId, Common::ContextManager::GetInstance()->GetChipType(deviceId));
    while (valid_size - pos >= logStructSize)
    {
        Convert::SocLogConvert::GetInstance().TransData(buffer, valid_size, deviceId, pos, logData);
        if (Mspti::Parser::DeviceTaskCalculator::GetInstance().ReportStarsSocLog(deviceId, logData) != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("DeviceTaskCalculator parse SocLog failed");
        }
        if (Mspti::Parser::KernelParser::GetInstance().ReportStarsSocLog(deviceId, logData) != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("KernelParser parse SocLog failed");
        }
    }
    return pos;
}
}  // namespace Channel
}  // namespace Ascend
}  // namespace Mspti
