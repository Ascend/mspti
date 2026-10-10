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

/**
 * PM Sampling 内部数据契约（采集侧 <-> 解析侧的唯一接口约定）。
 *
 * 第一层（采集 -> 解析）：DCMI 原始样本 PmDcmiSample，采集方负责点查与单位/时间戳归一化；
 * 第二层（解析 -> 引擎/解码）：会话样本 PmSessionSample 与 counterDataImage v2 布局，
 * 解析方负责窗口组装，解码方按布局读写镜像。当前全部指标经 DCMI 接口获取。
 *
 * 修改本文件的任何结构体、枚举值或布局常量，都必须同步评审采集与解析两侧的实现。
 */

#ifndef MSPTI_PMSAMPLING_PM_SAMPLING_CONTRACT_H
#define MSPTI_PMSAMPLING_PM_SAMPLING_CONTRACT_H

#include <cstdint>
#include <vector>

#include "csrc/include/mspti_pm_sampling.h"

namespace Mspti
{
namespace PmSampling
{

/**
 * 互连带宽类型。DCMI 带宽接口自带采样窗口，采集方只做单位归一化为 bytes/s：
 * HCCS(dcmi_get_hccs_link_bandwidth_info, GB/s, 逐链路 0~15)、
 * PCIe(dcmi_get_pcie_link_bandwidth_info, MB/ms)、RoCE(dcmi_get_rdma_bandwidth_info, MB/s)。
 */
enum PmBwType : uint32_t
{
    PM_BW_HCCS_TX = 0,
    PM_BW_HCCS_RX = 1,
    PM_BW_PCIE_TX = 2,
    PM_BW_PCIE_RX = 3,
    PM_BW_ROCE_TX = 4,
    PM_BW_ROCE_RX = 5,
    PM_BW_TYPE_COUNT
};

struct PmBwPoint
{
    PmBwType type = PM_BW_HCCS_TX;
    uint32_t instanceId = 0; /* HCCS 链路号 0~15；无实例概念时为 0 */
    double bytesPerSec = 0.0;
};

/**
 * DCMI 原始样本。tsNs 为 host 纳秒，同一 deviceId 内单调不减；各字段为瞬时原值，
 * 解析方透传不差分；字段本次不可用时按 sentinel 填写（< 0 或 0xFFFFFFFF），
 * 解析方对应指标按 0 组装，未查询的带宽类型不产出点。
 */
struct PmDcmiSample
{
    uint32_t deviceId = 0;
    uint64_t tsNs = 0;
    int32_t power = -1;                /* 0.1 W */
    int32_t temperature = -1;          /* 摄氏度 */
    uint32_t hbmBwUtil = 0xFFFFFFFFU;  /* HBM 带宽利用率 % */
    uint32_t aicoreUtil = 0xFFFFFFFFU; /* AI Core 利用率 % */
    uint32_t aivUtil = 0xFFFFFFFFU;    /* AI Vector 利用率 % */
    uint32_t aicUtil = 0xFFFFFFFFU;    /* AI Cube 利用率 % */
    uint32_t aicoreFreq = 0xFFFFFFFFU; /* AI Core 频率 MHz */
    uint32_t aivFreq = 0xFFFFFFFFU;    /* AI Vector 频率 MHz */
    std::vector<PmBwPoint> bandwidths;
};

/* 会话样本值条目，与公共头 msptiPmSamplingSampleEntry 字段一一对应。 */
struct PmSampleEntry
{
    uint32_t metricId = 0;
    uint32_t instanceId = 0; /* HCCS 链路号 0~15；无实例概念时为 0 */
    double value = 0.0;
};

/**
 * 会话样本。[startNs, endNs] 为样本窗口（host 纳秒），entries 逐实例一条，
 * 无实例概念的指标 instanceId 为 0，本窗口无数据时产出一条 {metricId, 0, 0.0}；
 * 带宽类指标 (1~6) 单位 bytes/s，其余为瞬时原值。
 */
struct PmSessionSample
{
    uint64_t startNs = 0;
    uint64_t endNs = 0;
    std::vector<PmSampleEntry> entries;
};

/**
 * counterDataImage v2 布局（解码镜像，对用户不透明，仅本契约与解码方知晓）。
 *
 *   偏移 0    : header，32 字节
 *               magic u32 = 0x50534D50 ("PSMP")
 *               version u32 = 2
 *               numMetrics u32
 *               maxEntriesPerSample u32
 *               populated u64（已填充样本数）
 *               maxSamples u64
 *   偏移 32   : metricId 区，4 * numMetrics 字节，向上对齐到 16 的倍数（镜像自描述）
 *   对齐后偏移: 样本区，每样本 stride = 24 + maxEntriesPerSample * 16 字节
 *               startNs u64 + endNs u64 + entryCount u32 + 保留 u32
 *               + maxEntriesPerSample 个 PmSampleEntry 槽位
 *               （entryCount 个有效，其余槽位置 0；entryCount <= maxEntriesPerSample）
 */
constexpr uint32_t PM_IMAGE_MAGIC = 0x50534D50U; /* "PSMP" */
constexpr uint32_t PM_IMAGE_VERSION = 2;
constexpr uint64_t PM_IMAGE_HEADER_SIZE = 32;
constexpr uint64_t PM_SAMPLE_FIXED_SIZE = 24; /* startNs u64 + endNs u64 + entryCount u32 + 保留 u32 = 24 */
constexpr uint64_t PM_SAMPLE_ENTRY_SIZE = 16; /* metricId + instanceId + value */

constexpr uint64_t Align16(uint64_t size) { return (size + 15) / 16 * 16; }

/* metricId 区结束、样本区起始的偏移 */
constexpr uint64_t SampleAreaOffset(uint32_t numMetrics)
{
    return PM_IMAGE_HEADER_SIZE + Align16(static_cast<uint64_t>(numMetrics) * 4);
}

/* 单样本字节数（entry 槽位按 maxEntriesPerSample 预留） */
constexpr uint64_t SampleStride(uint64_t maxEntriesPerSample)
{
    return PM_SAMPLE_FIXED_SIZE + maxEntriesPerSample * PM_SAMPLE_ENTRY_SIZE;
}

/* 镜像总大小 */
constexpr uint64_t CalcImageSize(uint32_t numMetrics, uint64_t maxEntriesPerSample, uint64_t maxSamples)
{
    return SampleAreaOffset(numMetrics) + maxSamples * SampleStride(maxEntriesPerSample);
}

/* GetCounterDataSize 入参上界：maxSamples 不设个数上限，用镜像总大小预算约束，
 * (PM_IMAGE_MAX_SIZE - 目录区) / stride 的除法判定同时排除乘法回绕与镜像超预算。
 * 镜像定位为批量搬运缓冲（长期存储是引擎内部样本队列，DecodeData 消费式分批搬运），
 * 预算相对引擎队列满载容量留足余量，正常情况下一次 decode 即可搬空队列。 */
constexpr uint32_t PM_IMAGE_MAX_ENTRIES_PER_SAMPLE = 1024; /* 宽松上界：覆盖全指标配置（含逐实例指标展开）*/
constexpr uint64_t PM_IMAGE_MAX_SIZE = 64ULL * 1024 * 1024; /* 64MiB */

/* GetCounterDataSize 入参合法性校验，全部通过返回 true；调用方校验失败返回 INVALID_PARAMETER */
inline bool ValidateImageParams(uint32_t numMetrics, uint64_t maxEntriesPerSample, uint64_t maxSamples)
{
    if (numMetrics == 0 || numMetrics > static_cast<uint32_t>(MSPTI_PM_SAMPLING_METRIC_ID_COUNT))
    {
        return false;
    }
    if (maxEntriesPerSample == 0 || maxEntriesPerSample > PM_IMAGE_MAX_ENTRIES_PER_SAMPLE)
    {
        return false;
    }
    return maxSamples > 0 &&
           maxSamples <= (PM_IMAGE_MAX_SIZE - SampleAreaOffset(numMetrics)) / SampleStride(maxEntriesPerSample);
}

}  // namespace PmSampling
}  // namespace Mspti

#endif
