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
 * @file  pm_sampling_manager.h
 * @brief PM Sampling 管理器：会话生命周期（单设备单会话）、采集线程启停、样本分发。
 *
 * 数据流（全部指标经 DCMI 点查）：
 *   DcmiCollector（节拍点查，组装契约 PmDcmiSample）
 *     -> OnDcmiSample -> PmSamplingParser（窗口组装，产出 PmSessionSample）
 *     -> DeliverLocked -> 该设备 Running 会话的样本队列。
 */

#ifndef MSPTI_PM_SAMPLING_PM_SAMPLING_MANAGER_H
#define MSPTI_PM_SAMPLING_PM_SAMPLING_MANAGER_H

#include <map>
#include <memory>
#include <mutex>

#include "csrc/include/mspti_pm_sampling.h"
#include "csrc/pm_sampling/pm_sampling_contract.h"
#include "csrc/pm_sampling/pm_sampling_object.h"
#include "csrc/pm_sampling/pm_sampling_parser.h"

namespace Mspti
{
namespace PmSampling
{

class PmSamplingManager
{
   public:
    static PmSamplingManager *GetInstance();

    // ---- 会话生命周期（单设备单会话）----
    msptiResult Enable(uint32_t deviceId, msptiPmSamplingObjectPtr *pObject);
    msptiResult Disable(msptiPmSamplingObjectPtr pObject);
    // 按指针值查会话并共享持有：只比较指针、不解引用句柄（过期/野指针均安全返回 nullptr），
    // 返回的 shared_ptr 保证调用期间对象存活（并发 Disable 不会销毁正在使用的会话）
    std::shared_ptr<PmSamplingObject> GetLiveObject(msptiPmSamplingObjectPtr pObject);

    // ---- 采集线程启停（Start / Stop 会话时调用）----
    msptiResult StartCollectors(PmSamplingObject &obj);
    void StopCollectors(PmSamplingObject &obj);

    // ---- 原始样本入口 ----
    // DCMI 点查样本（DcmiCollector 产出；sentinel 字段由解析方按 0 组装）。
    // 全程持 manager 锁：parser 组窗 -> DeliverLocked -> 会话入队。
    void OnDcmiSample(const PmDcmiSample &sample);
    // parser 产出的窗口样本 -> 该设备 Running 会话。
    // 注意：调用方必须已持有 manager 锁（parser -> DeliverLocked 全程在锁内）。
    void DeliverLocked(uint32_t deviceId, PmSessionSample &&sample);

    // ---- DCMI 采集需求查询（DcmiCollector 节流用）----
    // 是否存在 Running 会话，且距上次产出已超过会话最小采样周期
    bool ShouldCollectDcmi(uint64_t nowNs);
    // 全部 Running 会话配置 metricId 并集对应的 DCMI 接口组需求
    DcmiCollectNeeds CollectNeeds();
    // 当前 Running 会话的最小采样周期（ms，clamp [1,1000]）；无 Running 会话返回 0
    uint32_t MinDcmiIntervalMs();
    // 是否仍需要 DCMI 采集线程（存在已启动采集的会话且未 Disable）
    bool HasDcmiDemand();

   private:
    PmSamplingManager() = default;

    // 无锁版本，调用方须已持有 mtx_
    uint32_t MinDcmiIntervalMsLocked();

    std::mutex mtx_;
    std::map<uint32_t, std::shared_ptr<PmSamplingObject>> objects_;  // deviceId -> 会话（单设备单会话）
    std::map<uint32_t, std::unique_ptr<PmSamplingParser>> parsers_;
    // 句柄索引：指针 -> {deviceId, 是否活跃}。只以指针值作 key，查证不解引用；
    // Disable 后保留墓碑（alive=false）不删 key，防止地址复用导致过期句柄被误判为有效
    struct HandleEntry
    {
        uint32_t deviceId;
        bool alive;
    };
    std::map<PmSamplingObject *, HandleEntry> ptrIndex_;
    uint64_t lastDcmiNs_ = 0;
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_PM_SAMPLING_MANAGER_H
