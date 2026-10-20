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

#include "csrc/pm_sampling/dcmi/dcmi_collector.h"

#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <iterator>
#include <mutex>
#include <string>
#include <unordered_set>

#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/pm_sampling/dcmi/dcmi_api.h"
#include "csrc/pm_sampling/pm_sampling_contract.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
// 采集节拍
constexpr uint32_t DCMI_SAMPLE_INTERVAL_MS = 10;
constexpr uint32_t SLEEP_CHUNK_MS = 1;  // 分段睡眠，保证 Stop 及时
// 带宽接口 profiling_time 的合法范围（DCMI API 参考）
constexpr uint32_t HCCS_PROF_MIN_MS = 1;
constexpr uint32_t HCCS_PROF_MAX_MS = 1000;
constexpr uint32_t PCIE_PROF_MAX_MS = 2000;
constexpr uint32_t RDMA_PROF_MIN_MS = 100;
constexpr uint32_t RDMA_PROF_MAX_MS = 10000;

inline uint32_t ClampU32(uint32_t v, uint32_t lo, uint32_t hi) { return v < lo ? lo : (v > hi ? hi : v); }

// 采集逐拍执行，同类失败仅首次告警，避免持续失败刷屏
void WarnOnce(const std::string &key, const char *fmt, ...)
{
    static std::mutex mtx;
    static std::unordered_set<std::string> logged;
    std::lock_guard<std::mutex> lk(mtx);
    if (!logged.insert(key).second)
    {
        return;
    }
    char buf[256] = {0};
    va_list args;
    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    MSPTI_LOGW("%s", buf);
}
}  // namespace

DcmiCollector *DcmiCollector::GetInstance()
{
    // leaky 单例：进程退出时静态析构顺序不可控，采集线程可能在其他单例析构后仍跑最后一拍；
    // 服务型单例常驻至进程结束，由 OS 回收
    static DcmiCollector *instance = new DcmiCollector();
    return instance;
}

msptiResult DcmiCollector::StartCollector()
{
    std::lock_guard<std::mutex> lk(mtx_);
    if (running_)
    {
        return MSPTI_SUCCESS;
    }
    if (!DcmiApi::GetInstance()->Init())
    {
        // dcmi 不可用：Start 显式失败，避免会话 Running 却永远无数据的静默失败
        MSPTI_LOGE("DcmiCollector: dcmi init failed, pm sampling start aborted");
        return MSPTI_ERROR_DEVICE_OFFLINE;
    }
    const auto &devices = DcmiApi::GetInstance()->GetDevices();
    for (const auto &dev : devices)
    {
        deviceIds_.push_back(dev.deviceId);
    }
    running_ = true;
    // 上一轮停止可能残留汇聚进度（Stop 中途退出不保证归零），重启前清零
    {
        std::lock_guard<std::mutex> tickLk(tickMtx_);
        tickGen_ = 0;
        doneGen_ = 0;
        doneCount_ = 0;
    }
    const uint32_t totalWorkers = static_cast<uint32_t>(deviceIds_.size());
    for (uint32_t deviceId : deviceIds_)
    {
        workers_.emplace_back(&DcmiCollector::DeviceWorkerLoop, this, deviceId, totalWorkers);
    }
    coordinator_ = std::thread(&DcmiCollector::CoordinatorLoop, this);
    MSPTI_LOGI("DcmiCollector started, %zu device workers", deviceIds_.size());
    return MSPTI_SUCCESS;
}

void DcmiCollector::StopCollector()
{
    {
        std::lock_guard<std::mutex> lk(mtx_);
        // 需求检查与 join 均在锁内：防止并发 StartCollector 对 joinable 线程 move-assign 触发 std::terminate
        if (!running_ || PmSamplingManager::GetInstance()->HasDcmiDemand())
        {
            return;
        }
        running_ = false;
        // 持 tickMtx_ 唤醒两类等待者：设备线程（等新拍）、协调线程（等本拍汇聚），
        // 两者的谓词都含 !running_，持锁通知不会丢唤醒
        {
            std::lock_guard<std::mutex> tickLk(tickMtx_);
            tickCv_.notify_all();
            doneCv_.notify_all();
        }
        if (coordinator_.joinable())
        {
            coordinator_.join();
        }
        tickCv_.notify_all();
        for (auto &worker : workers_)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
        workers_.clear();
        deviceIds_.clear();
    }
    MSPTI_LOGI("DcmiCollector stopped");
}

void DcmiCollector::CoordinatorLoop()
{
    while (running_)
    {
        CollectTick();
        for (uint32_t slept = 0; running_ && slept < DCMI_SAMPLE_INTERVAL_MS; slept += SLEEP_CHUNK_MS)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_CHUNK_MS));
        }
    }
    // 退出前持锁唤醒设备线程：设备线程在 tickCv_ 上等新拍，谓词含 !running_
    {
        std::lock_guard<std::mutex> lk(tickMtx_);
        tickCv_.notify_all();
    }
}

void DcmiCollector::DeviceWorkerLoop(uint32_t deviceId, uint32_t totalWorkers)
{
    uint64_t seenGen = 0;
    std::unique_lock<std::mutex> lk(tickMtx_);
    while (true)
    {
        tickCv_.wait(lk, [&] { return !running_.load() || tickGen_ > seenGen; });
        if (!running_.load())
        {
            return;
        }
        seenGen = tickGen_;
        const TickWork work = tickWork_;
        lk.unlock();

        CollectDevice(deviceId, work);

        // 汇聚：最后一个完成的设备线程代广播代数追平并唤醒协调线程
        lk.lock();
        ++doneCount_;
        if (doneCount_ >= totalWorkers)
        {
            doneCount_ = 0;
            doneGen_ = seenGen;
            doneCv_.notify_one();
        }
    }
}

void DcmiCollector::CollectTick()
{
    auto *pmMgr = PmSamplingManager::GetInstance();
    // 按各 Running 会话的最小采样周期节流，由 manager 统一裁决
    uint64_t timestamp = Mspti::Common::ContextManager::GetInstance()->GetHostTimeStampNs();
    if (!pmMgr->ShouldCollectDcmi(timestamp))
    {
        return;
    }
    // 仅点查 Running 会话配置涉及的接口组；
    // 带宽接口自带采样窗口（profiling_time），按会话最小间隔夹取
    const DcmiCollectNeeds needs = pmMgr->CollectNeeds();
    const bool needBw = needs.hccsBw || needs.pcieBw || needs.roceBw;
    const uint32_t intervalMs = needBw ? pmMgr->MinDcmiIntervalMs() : 0;
    if (deviceIds_.empty())
    {
        return;
    }

    // 广播本拍工作包（时间戳单源，同拍各卡同 tsNs）
    {
        std::lock_guard<std::mutex> lk(tickMtx_);
        ++tickGen_;
        tickWork_.needs = needs;
        tickWork_.tsNs = timestamp;
        tickWork_.intervalMs = intervalMs;
        tickCv_.notify_all();
    }
    // 等全部设备线程完成本拍：保证拍间不重叠、各卡时间窗对齐
    const uint64_t gen = tickGen_;
    std::unique_lock<std::mutex> lk(tickMtx_);
    doneCv_.wait(lk, [&] { return !running_.load() || doneGen_ >= gen; });
}

void DcmiCollector::CollectDevice(uint32_t deviceId, const TickWork &work)
{
    auto *pmMgr = PmSamplingManager::GetInstance();
    PmDcmiSample sample;
    sample.deviceId = deviceId;
    sample.tsNs = work.tsNs;

    if (work.needs.power)
    {
        CollectPower(deviceId, sample);
    }
    if (work.needs.temperature)
    {
        CollectTemperature(deviceId, sample);
    }
    if (work.needs.utilization)
    {
        CollectUtilization(deviceId, sample);
    }
    if (work.needs.frequency)
    {
        CollectFrequency(deviceId, sample);
    }
    if (work.needs.hbm)
    {
        CollectHbm(deviceId, sample);
    }
    RunBandwidth(deviceId, sample, work.needs, work.intervalMs);

    pmMgr->OnDcmiSample(sample);
}

void DcmiCollector::RunBandwidth(uint32_t deviceId, PmDcmiSample &sample, const DcmiCollectNeeds &needs,
                                 uint32_t intervalMs)
{
    if (!needs.hccsBw && !needs.pcieBw && !needs.roceBw)
    {
        return;
    }
    // 带宽三路并发调用；各自写独立临时样本，完成后合并，
    // 避免多线程写同一 vector。线程资源不足时对未 spawn 的路退化为串行。
    PmDcmiSample hccsSample;
    PmDcmiSample pcieSample;
    PmDcmiSample roceSample;
    std::thread tHccs;
    std::thread tPcie;
    std::thread tRoce;
    try
    {
        if (needs.hccsBw)
        {
            tHccs = std::thread([this, deviceId, &hccsSample, intervalMs]
                                { CollectHccsBandwidth(deviceId, hccsSample, intervalMs); });
        }
        if (needs.pcieBw)
        {
            tPcie = std::thread([this, deviceId, &pcieSample, intervalMs]
                                { CollectPcieBandwidth(deviceId, pcieSample, intervalMs); });
        }
        if (needs.roceBw)
        {
            tRoce = std::thread([this, deviceId, &roceSample, intervalMs]
                                { CollectRdmaBandwidth(deviceId, roceSample, intervalMs); });
        }
    }
    catch (const std::exception &)
    {
        // std::thread 构造失败（线程资源不足）：已 spawn 的照常收结果，未 spawn 的串行补齐
    }
    auto drain = [](PmDcmiSample &dst, PmDcmiSample &src)
    {
        dst.bandwidths.insert(dst.bandwidths.end(), std::make_move_iterator(src.bandwidths.begin()),
                              std::make_move_iterator(src.bandwidths.end()));
    };
    if (tHccs.joinable())
    {
        tHccs.join();
        drain(sample, hccsSample);
    }
    else if (needs.hccsBw)
    {
        CollectHccsBandwidth(deviceId, sample, intervalMs);
    }
    if (tPcie.joinable())
    {
        tPcie.join();
        drain(sample, pcieSample);
    }
    else if (needs.pcieBw)
    {
        CollectPcieBandwidth(deviceId, sample, intervalMs);
    }
    if (tRoce.joinable())
    {
        tRoce.join();
        drain(sample, roceSample);
    }
    else if (needs.roceBw)
    {
        CollectRdmaBandwidth(deviceId, sample, intervalMs);
    }
}

void DcmiCollector::CollectPower(uint32_t deviceId, PmDcmiSample &sample)
{
    int power = -1;
    const int ret = DcmiApi::GetInstance()->GetPower(deviceId, &power);
    if (ret != 0)
    {
        WarnOnce("power:" + std::to_string(deviceId), "DcmiCollector: get power failed, device %u, ret=%d", deviceId,
                 ret);
    }
    sample.power = power;
}

void DcmiCollector::CollectTemperature(uint32_t deviceId, PmDcmiSample &sample)
{
    int temperature = -1;
    const int ret = DcmiApi::GetInstance()->GetTemperature(deviceId, &temperature);
    if (ret != 0)
    {
        WarnOnce("temperature:" + std::to_string(deviceId), "DcmiCollector: get temperature failed, device %u, ret=%d",
                 deviceId, ret);
    }
    sample.temperature = temperature;
}

void DcmiCollector::CollectUtilization(uint32_t deviceId, PmDcmiSample &sample)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasMultiUtilizationRate())
    {
        WarnOnce("multi_missing:" + std::to_string(deviceId),
                 "DcmiCollector: multi utilization interface missing, device %u", deviceId);
        return;
    }
    DcmiMultiUtilizationInfo info{};
    const int ret = api->GetMultiUtilizationRate(deviceId, &info);
    if (ret == 0)
    {
        sample.aicoreUtil = info.aicore_util;
        sample.aivectorUtil = info.aiv_util;
        sample.aicubeUtil = info.aic_util;
        sample.npuUtil = info.npu_util;
    }
    else
    {
        WarnOnce("multi_util:" + std::to_string(deviceId),
                 "DcmiCollector: get multi utilization failed, device %u, ret=%d", deviceId, ret);
    }
}

void DcmiCollector::CollectFrequency(uint32_t deviceId, PmDcmiSample &sample)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasFrequency())
    {
        WarnOnce("freq_missing:" + std::to_string(deviceId),
                 "DcmiCollector: frequency interface unavailable, device %u", deviceId);
        return;
    }
    unsigned int freq = 0;
    const int ret = api->GetFrequency(deviceId, FREQ_TYPE_AICORE, &freq);
    if (ret == 0)
    {
        sample.aicoreFreq = freq;
    }
    else
    {
        WarnOnce("freq:" + std::to_string(deviceId), "DcmiCollector: get frequency failed, device %u, ret=%d", deviceId,
                 ret);
    }
}

void DcmiCollector::CollectHbm(uint32_t deviceId, PmDcmiSample &sample)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasHbmInfo())
    {
        WarnOnce("hbm_missing:" + std::to_string(deviceId), "DcmiCollector: hbm info interface unavailable, device %u",
                 deviceId);
        return;
    }
    DcmiHbmInfo hbm{};
    const int ret = api->GetHbmInfo(deviceId, &hbm);
    if (ret != 0)
    {
        WarnOnce("hbm:" + std::to_string(deviceId), "DcmiCollector: get hbm info failed, device %u, ret=%d", deviceId,
                 ret);
        return;
    }
    sample.hbmBwUtil = hbm.bandwith_util_rate;
    // HBM 内存占用率：DCMI 无现成字段，按用量归一化得出
    if (hbm.memory_size != 0)
    {
        sample.hbmMemUtil = static_cast<uint32_t>(hbm.memory_usage * 100ULL / hbm.memory_size);
    }
}

void DcmiCollector::CollectHccsBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasHccsBandwidth())
    {
        WarnOnce("hccs_bw_missing:" + std::to_string(deviceId),
                 "DcmiCollector: hccs bandwidth interface unavailable, device %u", deviceId);
        return;
    }
    DcmiHccsBandwidthInfo info{};
    info.profiling_time = static_cast<int>(ClampU32(intervalMs, HCCS_PROF_MIN_MS, HCCS_PROF_MAX_MS));
    const int ret = api->GetHccsBandwidth(deviceId, &info);
    if (ret != 0)
    {
        WarnOnce("hccs_bw:" + std::to_string(deviceId), "DcmiCollector: get hccs bandwidth failed, device %u, ret=%d",
                 deviceId, ret);
        return;
    }
    // 逐链路透传，GB/s -> bytes/s
    for (int i = 0; i < DCMI_HCCS_MAX_LINK_NUM; ++i)
    {
        sample.bandwidths.push_back({PM_BW_HCCS_TX, static_cast<uint32_t>(i), info.tx_bandwidth[i] * 1e9});
        sample.bandwidths.push_back({PM_BW_HCCS_RX, static_cast<uint32_t>(i), info.rx_bandwidth[i] * 1e9});
    }
}

void DcmiCollector::CollectPcieBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasPcieBandwidth())
    {
        WarnOnce("pcie_bw_missing:" + std::to_string(deviceId),
                 "DcmiCollector: pcie bandwidth interface unavailable, device %u", deviceId);
        return;
    }
    DcmiPcieBandwidthInfo info{};
    info.profiling_time = static_cast<int>(ClampU32(intervalMs, 0, PCIE_PROF_MAX_MS));
    const int ret = api->GetPcieBandwidth(deviceId, &info);
    if (ret != 0)
    {
        WarnOnce("pcie_bw:" + std::to_string(deviceId), "DcmiCollector: get pcie bandwidth failed, device %u, ret=%d",
                 deviceId, ret);
        return;
    }
    // 有效带宽 = posted + completion（np 包不计），MB/ms == GB/s
    double tx = 0.0;
    double rx = 0.0;
    for (int i = 0; i < DCMI_PCIE_PROF_DATA_NUM; ++i)
    {
        tx += static_cast<double>(info.tx_p_bw[i]) + static_cast<double>(info.tx_cpl_bw[i]);
        rx += static_cast<double>(info.rx_p_bw[i]) + static_cast<double>(info.rx_cpl_bw[i]);
    }
    sample.bandwidths.push_back({PM_BW_PCIE_TX, 0, tx * 1e9});
    sample.bandwidths.push_back({PM_BW_PCIE_RX, 0, rx * 1e9});
}

void DcmiCollector::CollectRdmaBandwidth(uint32_t deviceId, PmDcmiSample &sample, uint32_t intervalMs)
{
    auto *api = DcmiApi::GetInstance();
    if (!api->HasRdmaBandwidth())
    {
        WarnOnce("rdma_bw_missing:" + std::to_string(deviceId),
                 "DcmiCollector: rdma bandwidth interface unavailable, device %u", deviceId);
        return;
    }
    DcmiRdmaBandwidthInfo info{};
    const int ret = api->GetRdmaBandwidth(deviceId, 0, ClampU32(intervalMs, RDMA_PROF_MIN_MS, RDMA_PROF_MAX_MS), &info);
    if (ret != 0)
    {
        WarnOnce("rdma_bw:" + std::to_string(deviceId), "DcmiCollector: get rdma bandwidth failed, device %u, ret=%d",
                 deviceId, ret);
        return;
    }
    // MB/s -> bytes/s
    sample.bandwidths.push_back({PM_BW_ROCE_TX, 0, static_cast<double>(info.tx_bandwidth) * 1e6});
    sample.bandwidths.push_back({PM_BW_ROCE_RX, 0, static_cast<double>(info.rx_bandwidth) * 1e6});
}

}  // namespace PmSampling
}  // namespace Mspti
