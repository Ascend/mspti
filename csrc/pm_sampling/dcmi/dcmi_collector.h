/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of the Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

/**
 * @file  dcmi_collector.h
 * @brief DCMI 采集器：广播/汇聚模型的多线程采集。
 *
 * 线程布局（StartCollector 时定型）：
 *   - 1 个协调线程（CoordinatorLoop）：控制节拍（ShouldCollectDcmi 节流）、每拍取一次
 *     统一时间戳并聚合各 Running 会话的接口组需求（CollectNeeds），广播给设备线程，
 *     等全部设备线程完成本拍后进入下一拍。
 *   - N 个设备线程（DeviceWorkerLoop，每卡一个常驻）：快接口组（power/temp/util/freq/hbm）
 *     串行点查；带宽三路（hccs/pcie/rdma，阻塞测量窗）按需 spawn 瞬态线程并发调用后 join。
 *     组装契约 PmDcmiSample 经 PmSamplingManager::OnDcmiSample 送入会话（manager 锁内串行化）。
 *
 * 生命周期由 PmSamplingManager 驱动：会话 Start 时启动线程组，StopCollector 时
 * 若无任何会话仍需要 DCMI 采集（HasDcmiDemand）则停全部线程。
 * 采样节流（按会话最小采样周期）由 PmSamplingManager::ShouldCollectDcmi 控制。
 */

#ifndef MSPTI_PM_SAMPLING_DCMI_COLLECTOR_H
#define MSPTI_PM_SAMPLING_DCMI_COLLECTOR_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "csrc/include/mspti_result.h"
#include "csrc/pm_sampling/pm_sampling_contract.h"

namespace Mspti
{
namespace PmSampling
{

class DcmiCollector
{
   public:
    static DcmiCollector *GetInstance();

    // PM Sampling 会话 Start 时调用；线程组已在跑则幂等返回。dcmi 不可用时返回 MSPTI_ERROR_DEVICE_OFFLINE。
    msptiResult StartCollector();
    // 会话 Stop 时调用；仅当无任何会话仍需要 DCMI 采集才停线程组。
    void StopCollector();

   private:
    // 每拍广播的工作包：需求、统一时间戳、带宽窗口（ms，仅带宽需求时非 0）
    struct TickWork
    {
        DcmiCollectNeeds needs;
        uint64_t tsNs = 0;
        uint32_t intervalMs = 0;
    };

    DcmiCollector() = default;
    DcmiCollector(const DcmiCollector &) = delete;
    DcmiCollector &operator=(const DcmiCollector &) = delete;

    void CoordinatorLoop();
    // 执行一拍：节流裁决 → 聚合需求 → 广播工作包 → 等全部设备线程汇聚完成
    void CollectTick();
    void DeviceWorkerLoop(uint32_t deviceId, uint32_t totalWorkers);
    void CollectDevice(uint32_t deviceId, const TickWork &work);
    // 带宽三路并发执行；线程资源不足时对未 spawn 的路退化为串行
    void RunBandwidth(uint32_t deviceId, PmDcmiSample &sample, const DcmiCollectNeeds &needs, uint32_t intervalMs);
    void CollectPower(uint32_t deviceId, PmDcmiSample &sample);
    void CollectTemperature(uint32_t deviceId, PmDcmiSample &sample);
    void CollectUtilization(uint32_t deviceId, PmDcmiSample &sample);
    void CollectFrequency(uint32_t deviceId, PmDcmiSample &sample);
    void CollectHbm(uint32_t deviceId, PmDcmiSample &sample);
    void CollectHccsBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs);
    void CollectPcieBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs);
    void CollectRdmaBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs);

    std::thread coordinator_;
    std::vector<std::thread> workers_;
    std::vector<uint32_t> deviceIds_;  // StartCollector 时定型的设备列表
    std::atomic<bool> running_{false};
    std::mutex mtx_;  // Start/Stop 生命周期锁

    // 节拍广播/汇聚（tickMtx_ 保护）：tickGen_ 已广播代数，doneGen_/doneCount_ 汇聚进度
    std::mutex tickMtx_;
    std::condition_variable tickCv_;
    std::condition_variable doneCv_;
    uint64_t tickGen_{0};
    uint64_t doneGen_{0};
    uint32_t doneCount_{0};
    TickWork tickWork_{};
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_DCMI_COLLECTOR_H
