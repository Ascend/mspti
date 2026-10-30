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

#ifndef MSPTI_ACTIVITY_ASCEND_CHANNEL_CHANNEL_READER_H
#define MSPTI_ACTIVITY_ASCEND_CHANNEL_CHANNEL_READER_H

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "csrc/common/bound_queue.h"
#include "csrc/common/config.h"
#include "csrc/common/context_manager.h"
#include "csrc/common/inject/inject_base.h"
#include "csrc/common/task.h"
#include "csrc/include/mspti_result.h"

namespace Mspti
{
namespace Ascend
{
namespace Channel
{
class ChannelReader : public Mspti::Common::Task
{
   public:
    ChannelReader(uint32_t deviceId, AI_DRV_CHANNEL channelId, uint32_t bufferSize = DEFAULT_CHANNEL_BUFFER_SIZE);
    virtual ~ChannelReader();
    virtual msptiResult Execute();
    virtual size_t HashId();
    msptiResult Init();
    msptiResult UnInit();
    msptiResult FlushDrvBuff();
    // 非破坏性排空：把 buffer_ 里残留的完整结构体交接给消费线程；wait=true 时还会等
    // 消费线程把队列（含在途解析）里的数据全部处理完。不停止消费线程，供 UnRegister 前调用
    msptiResult DrainToConsumer(bool wait = true);

    void SetChannelStopped();
    bool GetSchedulingStatus() const;
    void SetSchedulingStatus(bool isScheduling);

   private:
    struct BufferData;
    size_t TransDataToActivityBuffer(const char buffer[], size_t valid_size, uint32_t deviceId,
                                     AI_DRV_CHANNEL channelId);
    size_t TransTsFwData(const char buffer[], size_t valid_size, uint32_t deviceId);
    size_t TransStarsLog(const char buffer[], size_t valid_size, uint32_t deviceId);
    void CheckIfSendFlush(int currLen);
    void SendFlushFinished();

    // AICPU/AiCustomCpu 数据读后即丢弃，不走异步消费队列/线程
    bool IsDiscardChannel() const;
    // 仅 SOC_LOG(50) 走异步消费队列/线程，其余通道读后直接同步消费
    bool IsAsyncChannel() const;
    void ConsumerThreadFunc();
    msptiResult StartConsumerThread();
    void StopConsumerThread();
    // 把 buffer_ 已读数据对齐切分后交接给消费队列；成功返回 true（curPos_ 收缩到残留），
    // 失败返回 false（数据完整保留，调用方可重试或告警）
    bool TryHandoffBufferedData(size_t consumedSize);
    // 关机/析构前的最后交接：不受批量阈值限制；失败或剩半包尾巴记 WARN 后丢弃
    void DrainBufferedDataToConsumer();
    void ConsumeBufferData(BufferData& buff);
    // 交接用的空闲 buffer 池：消费线程解析完把 buffer 归还，交接时优先复用，
    // 避免每次交接都重新分配并清零整块 buffer
    std::string TakeFreeBuffer();
    void ReturnFreeBuffer(std::string&& buf);

   private:
    // basic info
    uint32_t deviceId_;
    AI_DRV_CHANNEL channelId_;
    size_t hashId_{0};
    uint64_t totalSize_{0};
    size_t curPos_{0};
    uint32_t bufferSize_{0};
    std::string buffer_;
    // 解析结构体大小：仅与 device/channel 有关，Init 时查表一次算好，Execute 直接复用；
    // 查不到或非法时 Init 直接报错，该成员恒为有效值
    size_t dataStructSize_{0};

    // status info
    volatile bool isInited_{false};
    std::atomic<bool> isScheduling_{false};
    volatile bool isChannelStopped_{false};

    // flush
    bool needWait_{false};
    uint32_t flushBufSize_{0};
    uint32_t flushCurSize_{0};
    std::mutex flushMutex_;
    std::condition_variable flushFlag_;

    struct BufferData
    {
        std::string data;
        size_t validSize{0};
    };
    // 交接批量阈值：buffer 里有效数据不到 0.8 倍 bufferSize_ 不交接，减少交接次数；
    // buffer 满（LIMIT 场景）或 UnInit 关机排空时不受此限制；Init 时一次算好
    size_t handoffThreshold_{0};
    size_t consumeQueueCapacity_{0};
    std::atomic<bool> consumerRunning_{false};
    std::atomic<uint64_t> queueFullCount_{0};
    std::thread consumerThread_;
    std::unique_ptr<Common::BoundQueue<BufferData>> consumerQueue_;

    // buffer_/curPos_ 的读写锁：Execute 在 worker 线程、DrainToConsumer/DrainBufferedDataToConsumer
    // 在调用线程都会访问，必须串行化
    std::mutex bufferMutex_;
    // 已交接给消费线程但尚未解析完的块数（含队列中与正在解析的），DrainToConsumer 据此等待排空
    std::atomic<int64_t> pendingConsumeCount_{0};
    std::mutex consumeWaitMutex_;
    std::condition_variable consumeWaitCv_;
    // 空闲 buffer 池（消费线程归还、交接时取用），只保留少量，避免长期占用内存
    std::mutex freeBufferMutex_;
    std::vector<std::string> freeBuffers_;
};
}  // namespace Channel
}  // namespace Ascend
}  // namespace Mspti

#endif  // MSPTI_ACTIVITY_ASCEND_CHANNEL_CHANNEL_READER_H
