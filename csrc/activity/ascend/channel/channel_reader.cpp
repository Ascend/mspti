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
#include "csrc/activity/ascend/channel/channel_reader.h"

#include <cinttypes>
#include <climits>
#include <cstring>
#include <map>

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
namespace
{
// channel → 结构体大小查询函数；新增 channel 只需加一行表项，无需改控制流。
// 注意底层实现只与 chipType 有关，不需要 deviceId，这里不透传。
// 消费队列容量：固定条目数，不随 buffer 大小换算
constexpr size_t CONSUME_QUEUE_CAPACITY = 128;

using StructSizeGetter = size_t (*)(Common::PlatformType chipType);

size_t GetTsfwStructSize(Common::PlatformType chipType)
{
    return Convert::TsfwConvert::GetInstance().GetStructSize(chipType);
}

size_t GetSocLogStructSize(Common::PlatformType chipType)
{
    return Convert::SocLogConvert::GetInstance().GetStructSize(chipType);
}

size_t GetAicpuStructSize(Common::PlatformType /*chipType*/)
{
    // AICPU/AiCustomCpu 数据不做结构体解析，返回 0 表示整块交接
    return 0;
}

const std::map<AI_DRV_CHANNEL, StructSizeGetter>& GetStructSizeTable()
{
    static const std::map<AI_DRV_CHANNEL, StructSizeGetter> table = {
        {PROF_CHANNEL_TS_FW, GetTsfwStructSize},
        {PROF_CHANNEL_STARS_SOC_LOG, GetSocLogStructSize},
        {PROF_CHANNEL_AICPU, GetAicpuStructSize},
        {PROF_CHANNEL_CUS_AICPU, GetAicpuStructSize},
    };
    return table;
}

// DrainToConsumer 等待消费线程把残留全部处理完的最长时间
constexpr uint32_t DRAIN_MAX_WAIT_MS = 1000;
}  // namespace

ChannelReader::ChannelReader(uint32_t deviceId, AI_DRV_CHANNEL channelId, uint32_t bufferSize)
    : deviceId_(deviceId), channelId_(channelId), bufferSize_(bufferSize), buffer_(bufferSize, '\0')
{
}

ChannelReader::~ChannelReader()
{
    // 兜底：未调 UnInit 即析构时同样排空并回收消费线程，避免 joinable 的 thread 触发 terminate；
    // 已 UnInit 过则 Stop 全是空操作，幂等。
    // queue 在此销毁：reader 对象由 ChannelPool 的 shared_ptr 持有，在途 Execute 结束前对象不会析构，
    // 因此不存在 UnInit 里 reset 的 UAF 窗口
    DrainBufferedDataToConsumer();
    StopConsumerThread();
    consumerQueue_.reset();
}

void ChannelReader::DrainBufferedDataToConsumer()
{
    std::lock_guard<std::mutex> bufferGuard(bufferMutex_);
    if (curPos_ == 0)
    {
        return;
    }
    if (!consumerRunning_.load(std::memory_order_acquire) || consumerQueue_ == nullptr)
    {
        size_t directSize = (dataStructSize_ == 0) ? curPos_ : curPos_ / dataStructSize_ * dataStructSize_;
        if (directSize > 0)
        {
            TransDataToActivityBuffer(buffer_.data(), directSize, deviceId_, channelId_);
        }
        if (directSize < curPos_)
        {
            MSPTI_LOGW("Discard %zu residual bytes while consumer stopped, deviceId=%u, channelId=%d",
                       curPos_ - directSize, deviceId_, channelId_);
        }
        curPos_ = 0;
        return;
    }
    size_t consumedSize = (dataStructSize_ == 0) ? curPos_ : curPos_ / dataStructSize_ * dataStructSize_;
    bool handedOff = (consumedSize > 0) && TryHandoffBufferedData(consumedSize);
    if (!handedOff || curPos_ > 0)
    {
        MSPTI_LOGW("Discard %zu residual bytes (handedOff=%d), deviceId=%u, channelId=%d", curPos_, handedOff ? 1 : 0,
                   deviceId_, channelId_);
    }
    curPos_ = 0;
}

msptiResult ChannelReader::DrainToConsumer(bool wait)
{
    // 非破坏性排空：只在 UnRegister 关开关前调用，不停止消费线程，reader 仍可继续使用
    if (consumerQueue_ == nullptr)
    {
        // AICPU/AiCustomCpu 等丢弃型 channel：数据不入队，直接返回
        return MSPTI_SUCCESS;
    }
    // 1. 把 buffer_ 里残留的完整结构体交接给消费线程（不受批量阈值限制）
    {
        std::lock_guard<std::mutex> bufferGuard(bufferMutex_);
        if (isInited_ && curPos_ > 0)
        {
            size_t consumedSize = (dataStructSize_ == 0) ? curPos_ : curPos_ / dataStructSize_ * dataStructSize_;
            if (consumedSize > 0 && !TryHandoffBufferedData(consumedSize))
            {
                MSPTI_LOGW("DrainToConsumer handoff failed, keep %zu bytes, deviceId=%u, channelId=%d", curPos_,
                           deviceId_, channelId_);
            }
        }
    }
    if (!wait)
    {
        // 只交接不等待：供 flush 重试循环内使用，避免每轮都白等一次 DRAIN_MAX_WAIT_MS
        return MSPTI_SUCCESS;
    }
    // 2. 等消费线程把队列（含刚交接的、以及在途解析的）残留全部处理完
    std::unique_lock<std::mutex> waitLock(consumeWaitMutex_);
    consumeWaitCv_.wait_for(waitLock, std::chrono::milliseconds(DRAIN_MAX_WAIT_MS),
                            [this] { return pendingConsumeCount_.load(std::memory_order_acquire) == 0; });
    if (pendingConsumeCount_.load(std::memory_order_acquire) != 0)
    {
        // 超时说明残留还没解析完，上层即将关闭 activity 开关，这些块会被 parser 丢弃：
        // 必须把失败信号传上去，不能静默返回成功
        MSPTI_LOGE("DrainToConsumer timeout, %" PRId64 " chunks still pending, deviceId=%u, channelId=%d",
                   pendingConsumeCount_.load(std::memory_order_relaxed), deviceId_, channelId_);
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

bool ChannelReader::IsDiscardChannel() const
{
    return channelId_ == PROF_CHANNEL_AICPU || channelId_ == PROF_CHANNEL_CUS_AICPU;
}

bool ChannelReader::IsAsyncChannel() const { return channelId_ == PROF_CHANNEL_STARS_SOC_LOG; }

void ChannelReader::ConsumeBufferData(BufferData& buff)
{
    if (buff.validSize > 0)
    {
        TransDataToActivityBuffer(buff.data.data(), buff.validSize, deviceId_, channelId_);
    }
    // 解析完把 buffer 归还给 reader 复用，避免下一次交接重新分配并清零整块 buffer
    ReturnFreeBuffer(std::move(buff.data));
    int64_t prev = pendingConsumeCount_.load(std::memory_order_acquire);
    while (prev > 0 && !pendingConsumeCount_.compare_exchange_weak(prev, prev - 1, std::memory_order_acq_rel))
    {
    }
    if (prev == 1)
    {
        std::lock_guard<std::mutex> lk(consumeWaitMutex_);
        consumeWaitCv_.notify_all();
    }
}

void ChannelReader::ConsumerThreadFunc()
{
    pthread_setname_np(pthread_self(), "ChConsumer");
    BufferData buff;
    while (consumerQueue_ != nullptr && consumerQueue_->Pop(buff))
    {
        ConsumeBufferData(buff);
    }
}

msptiResult ChannelReader::StartConsumerThread()
{
    if (consumerRunning_.load(std::memory_order_acquire))
    {
        return MSPTI_SUCCESS;
    }
    consumerRunning_.store(true, std::memory_order_release);
    if (consumerThread_.joinable())
    {
        return MSPTI_SUCCESS;
    }
    try
    {
        consumerThread_ = std::thread(&ChannelReader::ConsumerThreadFunc, this);
    }
    catch (const std::exception& e)
    {
        MSPTI_LOGE("Create consumer thread failed: %s, deviceId=%u, channelId=%d", e.what(), deviceId_, channelId_);
        consumerRunning_.store(false, std::memory_order_release);
        return MSPTI_ERROR_INNER;
    }
    return MSPTI_SUCCESS;
}

void ChannelReader::StopConsumerThread()
{
    // 只在“停止交接 + 关闭队列”期间持 bufferMutex_：它与 Execute 的交接互斥，保证置位
    // consumerRunning_ 之后不会再有新的块入队；join 需要等消费线程跑完当前解析，
    // 放到锁外，避免 UnInit 期间阻塞 worker 线程上的 Execute
    {
        std::lock_guard<std::mutex> bufferGuard(bufferMutex_);
        consumerRunning_.store(false, std::memory_order_release);
        if (consumerQueue_ != nullptr)
        {
            consumerQueue_->Quit();
        }
    }
    if (consumerThread_.joinable())
    {
        consumerThread_.join();
    }
    // join 之后队列里最多剩 Quit 前已入队的块，直接同步解析掉（不访问 buffer_/curPos_，无需持锁）
    if (consumerQueue_ != nullptr)
    {
        BufferData buff;
        while (consumerQueue_->TryPop(buff))
        {
            ConsumeBufferData(buff);
        }
    }
}

msptiResult ChannelReader::Init()
{
    hashId_ = std::hash<std::string>()(std::to_string(static_cast<int>(channelId_)) + std::to_string(deviceId_) +
                                       std::to_string(Common::Utils::GetClockMonotonicRawNs()));
    isInited_ = true;
    curPos_ = 0;
    queueFullCount_.store(0, std::memory_order_relaxed);
    pendingConsumeCount_.store(0, std::memory_order_relaxed);
    // 交接批量阈值取 0.8 倍 bufferSize_（默认 2MB buffer 下即 1.6MB），Init 一次算好
    handoffThreshold_ = static_cast<size_t>(bufferSize_) * 8 / 10;
    // chipType 仅与 device 有关，Init 时查一次用于 dataStructSize_ 查表即可，无需存成员
    Common::PlatformType chipType = Common::ContextManager::GetInstance()->GetChipType(deviceId_);
    // dataStructSize_ 仅与 device/channel 有关，这里查表一次算好，Execute 直接复用；
    // 查不到（未知 channel）或结果非法时直接报错返回，不带病启动
    dataStructSize_ = 0;
    const auto& structSizeTable = GetStructSizeTable();
    auto structSizeIter = structSizeTable.find(channelId_);
    if (structSizeIter == structSizeTable.end())
    {
        MSPTI_LOGE("Unsupported channel id: %d, deviceId: %u.", channelId_, deviceId_);
        isInited_ = false;
        return MSPTI_ERROR_INNER;
    }
    // dataStructSize_ == 0 表示该 channel 不做结构体解析（AICPU/AiCustomCpu），整块交接；
    // 仅 INVALID_STRUCT_SIZE 视为非法
    dataStructSize_ = structSizeIter->second(chipType);
    if (dataStructSize_ == Convert::INVALID_STRUCT_SIZE)
    {
        MSPTI_LOGE("Invalid struct size %zu for channel: %d, deviceId: %u.", dataStructSize_, channelId_, deviceId_);
        isInited_ = false;
        return MSPTI_ERROR_INNER;
    }
    // 仅 SOC_LOG(50) 走异步消费队列/线程；AICPU 等丢弃型 channel 不需要
    if (IsAsyncChannel())
    {
        // 每次 Init 重建消费队列：BoundQueue::Quit 不可逆，复用旧队列会导致新线程 Pop 立返空转
        consumeQueueCapacity_ = CONSUME_QUEUE_CAPACITY;
        Mspti::Common::MsptiMakeUniquePtr(consumerQueue_, consumeQueueCapacity_);
        if (consumerQueue_ == nullptr)
        {
            MSPTI_LOGE("Create consumer queue failed, deviceId=%u, channelId=%d", deviceId_, channelId_);
            isInited_ = false;
            return MSPTI_ERROR_INNER;
        }
        if (StartConsumerThread() != MSPTI_SUCCESS)
        {
            MSPTI_LOGE("Start consumer thread failed, deviceId=%u, channelId=%d", deviceId_, channelId_);
            // 失败路径要清掉已建的队列，否则对象会带着残留队列进入下一次 Init
            consumerQueue_.reset();
            isInited_ = false;
            return MSPTI_ERROR_INNER;
        }
    }
    return MSPTI_SUCCESS;
}

msptiResult ChannelReader::UnInit()
{
    if (!isInited_)
    {
        return MSPTI_SUCCESS;
    }
    isInited_ = false;
    DrainBufferedDataToConsumer();
    StopConsumerThread();
    MSPTI_EVENT("UnInit channel reader, deviceId=%u, channelId=%d, totalSize=%lu, queueFullCount=%lu", deviceId_,
                channelId_, totalSize_, queueFullCount_.load(std::memory_order_relaxed));
    return MSPTI_SUCCESS;
}

msptiResult ChannelReader::FlushDrvBuff()
{
    // 1. query flush size
    std::unique_lock<std::mutex> guard(flushMutex_);
    unsigned int flushSize = 0;
    const int32_t ret = HalProfDataFlush(deviceId_, channelId_, &flushSize);
    if (ret == DRV_ERROR_NOT_SUPPORT)
    {
        MSPTI_LOGW("Function HalProfDataFlush is not supported, deviceId:%u, channelId:%u", deviceId_, channelId_);
        return MSPTI_SUCCESS;
    }
    if (ret != DRV_ERROR_NONE)
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

bool ChannelReader::GetSchedulingStatus() const { return isScheduling_.load(std::memory_order_acquire); }

void ChannelReader::SetSchedulingStatus(bool isScheduling)
{
    isScheduling_.store(isScheduling, std::memory_order_release);
}

// Execute 退出（正常返回、break、异常）时自动把调度标志清零，避免入口清零导致的重复入队
struct SchedulingGuard
{
    explicit SchedulingGuard(std::atomic<bool>& flag) : flag_(flag) {}
    ~SchedulingGuard() { flag_.store(false, std::memory_order_release); }
    std::atomic<bool>& flag_;
};

msptiResult ChannelReader::Execute()
{
    // 调度标志在出口清零（RAII，覆盖所有 break/return/异常路径）；
    // 入口不清零，避免背压 break 后被重复 Dispatch（同 reader 恒落同一 worker，串行无竞态）
    SchedulingGuard schedulingGuard(isScheduling_);
    std::unique_lock<std::mutex> guard(flushMutex_, std::defer_lock);
    int currLen = 0;
    while (isInited_ && !isChannelStopped_)
    {
        // buffer_ 可能在交接中被换过，每轮重新取指针，杜绝悬空；
        // 同时用 bufferMutex_ 串行化与 DrainToConsumer 对 buffer_/curPos_ 的访问
        std::lock_guard<std::mutex> bufferGuard(bufferMutex_);
        char* buff = const_cast<char*>(buffer_.data());
        guard.lock();
        currLen = ProfChannelRead(deviceId_, channelId_, buff + curPos_, bufferSize_ - curPos_);
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
        if (uintCurrLen > (bufferSize_ - curPos_))
        {
            MSPTI_LOGE("Read invalid data len [%zu] from driver", uintCurrLen);
            break;
        }
        curPos_ += uintCurrLen;
        totalSize_ += uintCurrLen;
        // AICPU/AiCustomCpu 数据读后即丢弃，不交接给异步消费线程，省去队列/线程唤醒开销
        if (IsDiscardChannel())
        {
            curPos_ = 0;
            continue;
        }
        // 非异步通道（如 TS_FW）：读后直接同步消费，不走队列/线程
        if (!IsAsyncChannel())
        {
            size_t consumedSize = (dataStructSize_ == 0) ? curPos_ : curPos_ / dataStructSize_ * dataStructSize_;
            if (consumedSize == 0)
            {
                if (curPos_ >= bufferSize_)
                {
                    // 攒满一个 buffer 仍凑不出完整结构体（未知芯片等），避免死循环，本次先返回
                    break;
                }
                continue;
            }
            TransDataToActivityBuffer(buffer_.data(), consumedSize, deviceId_, channelId_);
            size_t remainSize = curPos_ - consumedSize;
            if (remainSize > 0)
            {
                char* buffData = const_cast<char*>(buffer_.data());
                // consumedSize>0 时 src/dest 不重叠，memcpy_s 安全
                if (memcpy_s(buffData, bufferSize_, buffData + consumedSize, remainSize) != EOK)
                {
                    MSPTI_LOGE("memcpy_s failed, deviceId=%u, channelId=%d", deviceId_, channelId_);
                }
            }
            curPos_ = remainSize;
            continue;
        }
        // 异步通道（SOC_LOG）：攒批交接；有效数据不到阈值不交接，关机残留由 UnInit 兜底交接
        if (curPos_ < handoffThreshold_)
        {
            continue;
        }
        // 对齐切分只用 Init 算好的 dataStructSize_；为 0 表示无需对齐，整块交接
        size_t consumedSize = (dataStructSize_ == 0) ? curPos_ : curPos_ / dataStructSize_ * dataStructSize_;
        if (consumedSize == 0)
        {
            if (curPos_ >= bufferSize_)
            {
                // 攒满一个 buffer 仍凑不出完整结构体（未知芯片等），避免死循环，本次先返回
                break;
            }
            continue;
        }
        if (!TryHandoffBufferedData(consumedSize))
        {
            if (!consumerRunning_.load(std::memory_order_acquire))
            {
                // 消费线程已停止（UnInit 与在途 Execute 交错）：数据留在 buffer_，由排空路径同步解析
                MSPTI_LOGW("Consumer thread stopped, keep %zu bytes for drain, deviceId=%u, channelId=%d", curPos_,
                           deviceId_, channelId_);
                break;
            }
            auto fullCount = queueFullCount_.load(std::memory_order_relaxed);
            if (fullCount % 100 == 1)
            {
                MSPTI_LOGW(
                    "Consume queue full, keep %zu bytes in buffer for retry, deviceId=%u, channelId=%d, "
                    "totalFull=%lu",
                    curPos_, deviceId_, channelId_, fullCount);
            }
            break;  // 背压：残留保留，下次调度重试，绝不丢数
        }
    }
    return MSPTI_SUCCESS;
}

bool ChannelReader::TryHandoffBufferedData(size_t consumedSize)
{
    if (consumedSize == 0 || consumedSize > curPos_ || !consumerRunning_.load(std::memory_order_acquire) ||
        consumerQueue_ == nullptr)
    {
        return false;
    }
    // 未消费的尾巴（不足一个结构体，有界且很小）先拷到预取的新 buffer 头部：
    // 放到交接之前，失败时原 buffer 完整保留，不需要回滚已入队的块
    size_t remainSize = curPos_ - consumedSize;
    std::string next = TakeFreeBuffer();  // 复用消费线程归还的空闲 buffer（无则新建）
    if (remainSize > 0 && memcpy_s(&next[0], bufferSize_, buffer_.data() + consumedSize, remainSize) != EOK)
    {
        ReturnFreeBuffer(std::move(next));
        MSPTI_LOGE("Copy residual data failed, keep %zu bytes, deviceId=%u, channelId=%d", curPos_, deviceId_,
                   channelId_);
        return false;
    }
    BufferData buffData;
    buffData.data.swap(buffer_);  // O(1)，零字节拷贝
    buffData.validSize = consumedSize;
    // 先计数再入队：保证消费线程出队前计数已加，避免计数被减成负数或漏唤醒
    pendingConsumeCount_.fetch_add(1, std::memory_order_acq_rel);
    // 入口已保证 consumerQueue_ 非空，这里不再判空
    if (!consumerQueue_->TryPush(std::move(buffData)))
    {
        pendingConsumeCount_.fetch_sub(1, std::memory_order_acq_rel);
        buffer_.swap(buffData.data);        // 零拷贝撤销，数据完整回到 buffer_ 头
        ReturnFreeBuffer(std::move(next));  // 归还预取的 buffer
        queueFullCount_.fetch_add(1, std::memory_order_relaxed);
        return false;
    }
    buffer_.swap(next);  // buffer_ 恢复 bufferSize_ 大小，残留已在头部
    curPos_ = remainSize;
    return true;
}

std::string ChannelReader::TakeFreeBuffer()
{
    std::lock_guard<std::mutex> lk(freeBufferMutex_);
    if (!freeBuffers_.empty())
    {
        std::string buf = std::move(freeBuffers_.back());
        freeBuffers_.pop_back();
        return buf;
    }
    return std::string(bufferSize_, '\0');
}

void ChannelReader::ReturnFreeBuffer(std::string&& buf)
{
    if (buf.size() != bufferSize_)
    {
        // 尺寸异常（理论上不会出现）直接丢弃，避免污染空闲池
        return;
    }
    std::lock_guard<std::mutex> lk(freeBufferMutex_);
    // 只留少量备用：稳态下交接与解析各占一块，多余的直接释放，避免长期占用内存
    constexpr size_t MAX_FREE_BUFFERS = 4;
    if (freeBuffers_.size() < MAX_FREE_BUFFERS)
    {
        freeBuffers_.push_back(std::move(buf));
    }
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

size_t ChannelReader::TransDataToActivityBuffer(const char buffer[], size_t valid_size, uint32_t deviceId,
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

size_t ChannelReader::TransTsFwData(const char buffer[], size_t valid_size, uint32_t deviceId)
{
    size_t pos = 0;
    // 用 Init 缓存的 dataStructSize_：同一 reader 同 device，顺带修掉原来 static 首调定死的跨 device 污染
    size_t logStructSize = dataStructSize_;
    if (logStructSize == 0)
    {
        return 0;
    }
    while (valid_size - pos >= logStructSize)
    {
        StepTraceBasic stepTrace;
        const TsTrackHead* tsHead = reinterpret_cast<const TsTrackHead*>(buffer + pos);
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

size_t ChannelReader::TransStarsLog(const char buffer[], size_t valid_size, uint32_t deviceId)
{
    size_t pos = 0;
    HalLogData logData;
    size_t logStructSize = dataStructSize_;
    if (logStructSize == 0)
    {
        return 0;
    }
    while (valid_size - pos >= logStructSize)
    {
        size_t prevPos = pos;
        if (!Convert::SocLogConvert::GetInstance().TransData(buffer, valid_size, deviceId, pos, logData))
        {
            // 解析失败（未知 funcType 等）的记录不参与上报：logData 里保留的是上一条记录的内容，
            // 继续上报会把残留数据当成有效记录。正常路径 TransData 已推进 pos，只有 parseFunc 为空时
            // 才不推进，这里兜底跳过一个结构体，保证消费线程不会饿死
            if (pos == prevPos)
            {
                MSPTI_LOGW("TransData made no progress, skip one struct, deviceId=%u", deviceId);
                pos += logStructSize;
            }
            continue;
        }
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
