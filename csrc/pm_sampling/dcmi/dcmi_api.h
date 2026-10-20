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
 * @file  dcmi_api.h
 * @brief libdcmi.so 动态加载器：v1/v2 双代分派 + PM Sampling 所需点查接口。
 *
 * v1 为 (card_id, device_id) 双参模型，扁平 deviceId 按 (card, chip) 顺序展开；
 * v2 为扁平 dev_id 单参模型（dcmiv2_ 前缀符号，950 等新代际）。
 * 结构体按 DCMI API 参考自行声明（dlopen 方式不含真实头文件），
 * 带宽结构体布局与 Atlas 系列 DCMI API 参考一致。
 */

#ifndef MSPTI_PM_SAMPLING_DCMI_API_H
#define MSPTI_PM_SAMPLING_DCMI_API_H

#include <cstdint>
#include <vector>

namespace Mspti
{
namespace PmSampling
{

// dcmi_get_device_frequency 的 freq_type 码位：7=AI Core 当前频率
// （接口仅支持 2/6/7/9，Vector 频率无可用码位）
constexpr int FREQ_TYPE_AICORE = 7;

// HCCS 带宽结构体的逐链路数组长度（DCMI 固定 16 槽位，未使用链路值为 0）
constexpr int DCMI_HCCS_MAX_LINK_NUM = 16;
// PCIe 带宽结构体的分组数组长度（AGENTDRV_PROF_DATA_NUM）
constexpr int DCMI_PCIE_PROF_DATA_NUM = 3;

// DCMI 结构体布局（自行声明，与 dcmi_interface_api.h 一致）
struct DcmiMultiUtilizationInfo
{
    unsigned int aic_util;     // AI Cube 利用率
    unsigned int aiv_util;     // AI Vector 利用率
    unsigned int aicore_util;  // AI Core 利用率
    unsigned int npu_util;     // NPU 整体利用率
    unsigned int reserved[8];  // 预留
};

struct DcmiHbmInfo
{
    unsigned long long memory_size;   // 总容量 MB
    unsigned int freq;                // 频率 MHz
    unsigned long long memory_usage;  // 已用容量 MB
    int temp;                         // 温度 ℃
    unsigned int bandwith_util_rate;  // 带宽利用率 %
};

// dcmi_get_hccs_link_bandwidth_info：profiling_time 为入参（1~1000ms），
// 带宽单位 GB/s，逐链路 0~15
struct DcmiHccsBandwidthInfo
{
    int profiling_time;
    double total_txbw;
    double total_rxbw;
    double tx_bandwidth[DCMI_HCCS_MAX_LINK_NUM];
    double rx_bandwidth[DCMI_HCCS_MAX_LINK_NUM];
};

// dcmi_get_pcie_link_bandwidth_info：profiling_time 为入参（0~2000ms），
// 带宽单位 MB/ms，np 包不计入有效带宽（MSPTI 口径 TX=tx_p+tx_cpl）
struct DcmiPcieBandwidthInfo
{
    int profiling_time;
    unsigned int tx_p_bw[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int tx_np_bw[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int tx_cpl_bw[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int tx_np_lantency[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int rx_p_bw[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int rx_np_bw[DCMI_PCIE_PROF_DATA_NUM];
    unsigned int rx_cpl_bw[DCMI_PCIE_PROF_DATA_NUM];
};

// dcmi_get_rdma_bandwidth_info：单位 MB/s，port_id 当前仅支持 0
struct DcmiRdmaBandwidthInfo
{
    unsigned int tx_bandwidth;
    unsigned int rx_bandwidth;
};

// libdcmi.so 动态加载器：dlopen 候选路径 + dlsym 预取 + init + 拓扑枚举。
// 统一按扁平 deviceId 提供访问器，内部按设备代际分派 v1/v2 符号。
class DcmiApi
{
   public:
    static DcmiApi *GetInstance();

    // 加载 libdcmi.so、init 并枚举设备拓扑；失败返回 false（Start 显式失败）。
    // 幂等：成功后重复调用直接返回 true。
    bool Init();

    struct DcmiDevice
    {
        uint32_t deviceId;  // 扁平设备号（与 ACL 逻辑 deviceId 一致）
        int cardId;         // v1 二级寻址用；v2 设备置 -1
        int chipId;
        bool isV2;
    };
    const std::vector<DcmiDevice> &GetDevices() const { return devices_; }

    // 统一采集访问器（按扁平 deviceId），返回 0 表示成功；符号缺失返回 -1。
    int GetPower(uint32_t deviceId, int *out) const;
    int GetTemperature(uint32_t deviceId, int *out) const;
    int GetMultiUtilizationRate(uint32_t deviceId, DcmiMultiUtilizationInfo *out) const;
    int GetFrequency(uint32_t deviceId, int type, unsigned int *out) const;
    int GetHbmInfo(uint32_t deviceId, DcmiHbmInfo *out) const;
    int GetHccsBandwidth(uint32_t deviceId, DcmiHccsBandwidthInfo *out) const;
    int GetPcieBandwidth(uint32_t deviceId, DcmiPcieBandwidthInfo *out) const;
    int GetRdmaBandwidth(uint32_t deviceId, unsigned int portId, unsigned int profTimeMs,
                         DcmiRdmaBandwidthInfo *out) const;

    // 会话代际内符号可用性（采集侧决定是否产对应指标点，防空调符号指针）；
    // 对外可采预期由指标表按芯片型号判定，不依赖本组接口
    bool HasMultiUtilizationRate() const
    {
        return gen_ == DcmiGen::V2 ? dcmiv2GetDeviceMultiUtilizationRate_ != nullptr
                                   : dcmiGetDeviceMultiUtilizationRate_ != nullptr;
    }
    bool HasFrequency() const
    {
        return gen_ == DcmiGen::V2 ? dcmiv2GetDeviceFrequency_ != nullptr : dcmiGetDeviceFrequency_ != nullptr;
    }
    bool HasHbmInfo() const
    {
        return gen_ == DcmiGen::V2 ? dcmiv2GetDeviceHbmInfo_ != nullptr : dcmiGetDeviceHbmInfo_ != nullptr;
    }
    bool HasHccsBandwidth() const
    {
        return gen_ == DcmiGen::V2 ? dcmiv2GetHccsLinkBandwidth_ != nullptr : dcmiGetHccsLinkBandwidth_ != nullptr;
    }
    bool HasPcieBandwidth() const
    {
        return gen_ == DcmiGen::V2 ? dcmiv2GetPcieLinkBandwidth_ != nullptr : dcmiGetPcieLinkBandwidth_ != nullptr;
    }
    // rdma 仅 v1 有接口；v2 及之后代际不可用（V6/950 驱动确认不支持）
    bool HasRdmaBandwidth() const { return gen_ == DcmiGen::V1 && dcmiGetRdmaBandwidth_ != nullptr; }

   private:
    DcmiApi() = default;
    DcmiApi(const DcmiApi &) = delete;
    DcmiApi &operator=(const DcmiApi &) = delete;

    bool EnumerateTopologyV2();
    bool EnumerateTopologyV1();
    // 按扁平 deviceId 查设备寻址信息，未找到返回 nullptr
    const DcmiDevice *ResolveDevice(uint32_t deviceId) const;

    void *handle_ = nullptr;
    bool inited_ = false;
    // DCMI 接口代际：Init 成功路径决定；Get*/Has* 分发以此为口径。
    // 新增代际（如 v3）时扩展枚举、Init 追加分支，并同步带 v1/v2 二选一分发的访问器
    enum class DcmiGen
    {
        V1 = 1,
        V2 = 2
    };
    DcmiGen gen_ = DcmiGen::V1;
    std::vector<DcmiDevice> devices_;

    // v1 符号：(card_id, device_id) 双参
    int (*dcmiInit_)(void) = nullptr;
    int (*dcmiGetCardList_)(int *, int *, int) = nullptr;
    int (*dcmiGetDeviceIdInCard_)(int, int *, int *, int *) = nullptr;
    int (*dcmiGetDevicePowerInfo_)(int, int, int *) = nullptr;
    int (*dcmiGetDeviceTemperature_)(int, int, int *) = nullptr;
    int (*dcmiGetDeviceMultiUtilizationRate_)(int, int, DcmiMultiUtilizationInfo *) = nullptr;
    int (*dcmiGetDeviceFrequency_)(int, int, int, unsigned int *) = nullptr;
    int (*dcmiGetDeviceHbmInfo_)(int, int, DcmiHbmInfo *) = nullptr;
    int (*dcmiGetHccsLinkBandwidth_)(int, int, DcmiHccsBandwidthInfo *) = nullptr;
    int (*dcmiGetPcieLinkBandwidth_)(int, int, DcmiPcieBandwidthInfo *) = nullptr;
    int (*dcmiGetRdmaBandwidth_)(int, int, unsigned int, unsigned int, DcmiRdmaBandwidthInfo *) = nullptr;

    // v2 符号：扁平 dev_id 单参（V6/950 无 v2 hccs，v2 rdma 驱动确认不支持故不加载）
    int (*dcmiv2Init_)(void) = nullptr;
    int (*dcmiv2GetDeviceList_)(int *, int *, int) = nullptr;
    int (*dcmiv2GetDevicePowerInfo_)(int, int *) = nullptr;
    int (*dcmiv2GetDeviceTemperature_)(int, int *) = nullptr;
    int (*dcmiv2GetDeviceMultiUtilizationRate_)(int, DcmiMultiUtilizationInfo *) = nullptr;
    int (*dcmiv2GetDeviceFrequency_)(int, int, unsigned int *) = nullptr;
    int (*dcmiv2GetDeviceHbmInfo_)(int, DcmiHbmInfo *) = nullptr;
    int (*dcmiv2GetHccsLinkBandwidth_)(int, DcmiHccsBandwidthInfo *) = nullptr;
    int (*dcmiv2GetPcieLinkBandwidth_)(int, DcmiPcieBandwidthInfo *) = nullptr;
};

}  // namespace PmSampling
}  // namespace Mspti

#endif  // MSPTI_PM_SAMPLING_DCMI_API_H
