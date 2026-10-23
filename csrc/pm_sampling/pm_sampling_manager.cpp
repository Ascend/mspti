/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
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

#include "csrc/pm_sampling/pm_sampling_manager.h"

#include <algorithm>

#include "csrc/activity/ascend/dev_task_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"
#include "csrc/pm_sampling/dcmi/dcmi_collector.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
constexpr uint64_t kMinIntervalNs = 1000000ULL;  // 1ms
}  // namespace

PmSamplingManager *PmSamplingManager::GetInstance()
{
    // leaky 单例：进程退出时静态析构顺序不可控，采集线程可能在其他单例析构后仍跑最后一拍；
    // 服务型单例常驻至进程结束，由 OS 回收
    static PmSamplingManager *instance = new PmSamplingManager();
    return instance;
}

msptiResult PmSamplingManager::Enable(uint32_t deviceId, msptiPmSamplingObjectPtr *pObject)
{
    if (pObject == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    // 复用既有设备在线校验机制
    if (!Ascend::DevTaskManager::GetInstance()->CheckDeviceOnline(deviceId))
    {
        MSPTI_LOGE("PmSampling Enable failed: device %u is offline.", deviceId);
        return MSPTI_ERROR_DEVICE_OFFLINE;
    }
    std::lock_guard<std::mutex> lk(mtx_);
    if (objects_.find(deviceId) != objects_.end())
    {
        MSPTI_LOGE("PmSampling session already exists on device %u.", deviceId);
        return MSPTI_ERROR_PM_SAMPLING_ALREADY_ENABLED;
    }
    auto obj = std::shared_ptr<PmSamplingObject>();
    Mspti::Common::MsptiMakeSharedPtr(obj, deviceId);
    objects_[deviceId] = obj;
    ptrIndex_[obj.get()] = {deviceId, true};
    *pObject = Mspti::Common::ReinterpretConvert<msptiPmSamplingObjectPtr>(obj.get());
    MSPTI_LOGI("PmSampling session enabled, device %u", deviceId);
    return MSPTI_SUCCESS;
}

msptiResult PmSamplingManager::Disable(msptiPmSamplingObjectPtr pObject)
{
    if (pObject == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    std::shared_ptr<PmSamplingObject> obj;
    uint32_t deviceId = 0;
    auto *objKey = Mspti::Common::ReinterpretConvert<PmSamplingObject *>(pObject);
    {
        std::lock_guard<std::mutex> lk(mtx_);
        // 只按指针值查索引，不解引用句柄：并发 Disable / 过期句柄均安全
        auto iter = ptrIndex_.find(objKey);
        if (iter == ptrIndex_.end() || !iter->second.alive)
        {
            MSPTI_LOGE("PmSampling session not found for device of handle %p.", static_cast<void *>(pObject));
            return MSPTI_ERROR_PM_SAMPLING_NOT_ENABLED;
        }
        deviceId = iter->second.deviceId;
        // 原子认领：置墓碑 + 摘除注册表，此后其他线程无法再凭该句柄认领
        iter->second.alive = false;
        auto objIter = objects_.find(deviceId);
        if (objIter != objects_.end() && objIter->second.get() == objKey)
        {
            obj = objIter->second;
            objects_.erase(objIter);
        }
    }
    // obj 已被本线程独占（shared_ptr 保证存活），安全收尾
    if (obj && obj->GetState() == PmSamplingObject::State::Running)
    {
        // Running 会话先停采集（含解析器重置），再出注册表
        StopCollectors(*obj);
    }
    {
        std::lock_guard<std::mutex> lk(mtx_);
        parsers_.erase(deviceId);
    }
    // obj 在函数尾析构；若其他线程正持 shared_ptr 使用该会话，等其用完才真正销毁
    MSPTI_LOGI("PmSampling session disabled, device %u", deviceId);
    return MSPTI_SUCCESS;
}

std::shared_ptr<PmSamplingObject> PmSamplingManager::GetLiveObject(msptiPmSamplingObjectPtr pObject)
{
    if (pObject == nullptr)
    {
        return nullptr;
    }
    std::lock_guard<std::mutex> lk(mtx_);
    // 只比较指针值，不解引用句柄；墓碑（alive=false）与野指针统一拒绝
    auto *objKey = Mspti::Common::ReinterpretConvert<PmSamplingObject *>(pObject);
    auto iter = ptrIndex_.find(objKey);
    if (iter == ptrIndex_.end() || !iter->second.alive)
    {
        return nullptr;
    }
    auto objIter = objects_.find(iter->second.deviceId);
    if (objIter == objects_.end() || objIter->second.get() != objKey)
    {
        return nullptr;
    }
    return objIter->second;
}

msptiResult PmSamplingManager::StartCollectors(PmSamplingObject &obj)
{
    // 先置 Running 再拉线程：并发 StopCollector 的需求检查才能看到本会话，
    // 避免“线程被误判无需求停掉、会话却 Running”的静默无数据
    const msptiResult started = obj.Start();
    if (started != MSPTI_SUCCESS)
    {
        return started;
    }
    const msptiResult ret = DcmiCollector::GetInstance()->StartCollector();
    if (ret != MSPTI_SUCCESS)
    {
        // dcmi 不可用：回滚 Running 状态，不留下永远无数据的会话
        (void)obj.Stop();
        return ret;
    }
    const uint32_t deviceId = obj.DeviceId();
    {
        std::lock_guard<std::mutex> lk(mtx_);
        // 窗口组装需要 parser 基准状态，Start 时重建（上次 Stop/Flush 已清空）
        auto parser = std::unique_ptr<PmSamplingParser>();
        Mspti::Common::MsptiMakeUniquePtr(parser, obj.MetricIds());
        parsers_[deviceId] = std::move(parser);
    }
    return MSPTI_SUCCESS;
}

void PmSamplingManager::StopCollectors(PmSamplingObject &obj)
{
    const uint32_t deviceId = obj.DeviceId();
    {
        std::lock_guard<std::mutex> lk(mtx_);
        auto parserIter = parsers_.find(deviceId);
        if (parserIter != parsers_.end())
        {
            parserIter->second->Flush();
            parsers_.erase(parserIter);
        }
    }
    (void)obj.Stop();
    DcmiCollector::GetInstance()->StopCollector();
}

void PmSamplingManager::OnDcmiSample(const PmDcmiSample &sample)
{
    std::lock_guard<std::mutex> lk(mtx_);
    auto iter = parsers_.find(sample.deviceId);
    if (iter == parsers_.end())
    {
        return;
    }
    iter->second->OnDcmiSample(sample);
}

void PmSamplingManager::DeliverLocked(uint32_t deviceId, PmSessionSample &&sample)
{
    auto iter = objects_.find(deviceId);
    if (iter == objects_.end())
    {
        return;
    }
    iter->second->EnqueueSample(std::move(sample));
}

bool PmSamplingManager::ShouldCollectDcmi(uint64_t nowNs)
{
    std::lock_guard<std::mutex> lk(mtx_);
    const uint32_t minIntervalMs = MinDcmiIntervalMsLocked();
    if (minIntervalMs == 0)
    {
        return false;
    }
    if (lastDcmiNs_ != 0 && nowNs - lastDcmiNs_ < static_cast<uint64_t>(minIntervalMs) * kMinIntervalNs)
    {
        return false;
    }
    lastDcmiNs_ = nowNs;
    return true;
}

DcmiCollectNeeds PmSamplingManager::CollectNeeds()
{
    std::lock_guard<std::mutex> lk(mtx_);
    DcmiCollectNeeds needs;
    for (const auto &kv : objects_)
    {
        if (!kv.second->IsRunning())
        {
            continue;
        }
        for (uint32_t id : kv.second->MetricIds())
        {
            switch (id)
            {
                case MSPTI_PM_SAMPLING_METRIC_ID_POWER:
                    needs.power = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_TEMPERATURE:
                    needs.temperature = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_NPU_UTIL:
                case MSPTI_PM_SAMPLING_METRIC_ID_AICORE_UTIL:
                case MSPTI_PM_SAMPLING_METRIC_ID_AIVECTOR_UTIL:
                case MSPTI_PM_SAMPLING_METRIC_ID_AICUBE_UTIL:
                    needs.utilization = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_AICORE_FREQ:
                    needs.frequency = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_HBM_BW_UTIL:
                case MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL:
                    needs.hbm = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_HCCS_TX_BW:
                case MSPTI_PM_SAMPLING_METRIC_ID_HCCS_RX_BW:
                    needs.hccsBw = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_TX_BW:
                case MSPTI_PM_SAMPLING_METRIC_ID_PCIE_RX_BW:
                    needs.pcieBw = true;
                    break;
                case MSPTI_PM_SAMPLING_METRIC_ID_ROCE_TX_BW:
                case MSPTI_PM_SAMPLING_METRIC_ID_ROCE_RX_BW:
                    needs.roceBw = true;
                    break;
                default:
                    break;
            }
        }
    }
    return needs;
}

uint32_t PmSamplingManager::MinDcmiIntervalMs()
{
    std::lock_guard<std::mutex> lk(mtx_);
    return MinDcmiIntervalMsLocked();
}

uint32_t PmSamplingManager::MinDcmiIntervalMsLocked()
{
    uint64_t minIntervalNs = 0;
    for (const auto &kv : objects_)
    {
        if (!kv.second->IsRunning())
        {
            continue;
        }
        const uint64_t interval = kv.second->IntervalNs();
        minIntervalNs = (minIntervalNs == 0) ? interval : std::min(minIntervalNs, interval);
    }
    if (minIntervalNs == 0)
    {
        return 0;
    }
    uint64_t ms = minIntervalNs / kMinIntervalNs;
    if (ms < 1)
    {
        ms = 1;
    }
    if (ms > 1000)
    {
        ms = 1000;
    }
    return static_cast<uint32_t>(ms);
}

bool PmSamplingManager::HasDcmiDemand()
{
    std::lock_guard<std::mutex> lk(mtx_);
    for (const auto &kv : objects_)
    {
        if (kv.second->GetState() != PmSamplingObject::State::Created &&
            kv.second->GetState() != PmSamplingObject::State::Configured)
        {
            return true;
        }
    }
    return false;
}

}  // namespace PmSampling
}  // namespace Mspti
