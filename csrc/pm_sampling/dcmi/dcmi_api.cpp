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

#include "csrc/pm_sampling/dcmi/dcmi_api.h"

#include <dlfcn.h>

#include "csrc/common/context_manager.h"
#include "csrc/common/plog_manager.h"
#include "csrc/common/utils.h"

namespace Mspti
{
namespace PmSampling
{
namespace
{
constexpr int DCMI_OK = 0;
constexpr int MAX_CARD_NUM = 64;
constexpr int MAX_DEV_NUM = 64;
// libdcmi.so 常见安装路径
const char *LIBDCMI_CANDIDATES[] = {
    "libdcmi.so",  // 标准搜索路径（含 LD_LIBRARY_PATH）
    "/usr/local/dcmi/lib64/libdcmi.so",
    "/usr/local/dcmi/lib/libdcmi.so",
    "/usr/local/Ascend/driver/lib64/driver/libdcmi.so",
    "/usr/local/Ascend/driver/lib/libdcmi.so",
    "/usr/lib64/libdcmi.so",
    "/usr/lib/libdcmi.so",
};
constexpr int LIBDCMI_CANDIDATE_NUM = static_cast<int>(sizeof(LIBDCMI_CANDIDATES) / sizeof(LIBDCMI_CANDIDATES[0]));
}  // namespace

DcmiApi *DcmiApi::GetInstance()
{
    // leaky 单例：进程退出时静态析构顺序不可控，采集线程可能在其他单例析构后仍跑最后一拍；
    // 服务型单例常驻至进程结束，由 OS 回收
    static DcmiApi *instance = new DcmiApi();
    return instance;
}

bool DcmiApi::Init()
{
    if (inited_)
    {
        return true;
    }
    for (int i = 0; i < LIBDCMI_CANDIDATE_NUM; ++i)
    {
        void *handle = dlopen(LIBDCMI_CANDIDATES[i], RTLD_LAZY | RTLD_NODELETE);
        if (handle != nullptr)
        {
            handle_ = handle;
            break;
        }
    }
    if (handle_ == nullptr)
    {
        MSPTI_LOGW("DcmiApi: libdcmi.so unavailable, dcmi metrics disabled");
        return false;
    }

#define MSPTI_LOAD_DCMI_SYM(field, sym)                                           \
    do                                                                            \
    {                                                                             \
        field = Common::ReinterpretConvert<decltype(field)>(dlsym(handle_, sym)); \
    } while (0)
    // v1 符号（非关键符号缺失仅置空，访问器按可用性跳过）
    MSPTI_LOAD_DCMI_SYM(dcmiInit_, "dcmi_init");
    MSPTI_LOAD_DCMI_SYM(dcmiGetCardList_, "dcmi_get_card_list");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDeviceIdInCard_, "dcmi_get_device_id_in_card");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDevicePowerInfo_, "dcmi_get_device_power_info");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDeviceTemperature_, "dcmi_get_device_temperature");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDeviceMultiUtilizationRate_, "dcmi_get_device_multi_utilization_rate");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDeviceFrequency_, "dcmi_get_device_frequency");
    MSPTI_LOAD_DCMI_SYM(dcmiGetDeviceHbmInfo_, "dcmi_get_device_hbm_info");
    MSPTI_LOAD_DCMI_SYM(dcmiGetHccsLinkBandwidth_, "dcmi_get_hccs_link_bandwidth_info");
    MSPTI_LOAD_DCMI_SYM(dcmiGetPcieLinkBandwidth_, "dcmi_get_pcie_link_bandwidth_info");
    MSPTI_LOAD_DCMI_SYM(dcmiGetRdmaBandwidth_, "dcmi_get_rdma_bandwidth_info");
    // v2 符号（新代际扁平寻址；V6/950 无 v2 hccs，v2 rdma 驱动确认不支持故不加载）
    MSPTI_LOAD_DCMI_SYM(dcmiv2Init_, "dcmiv2_init");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDeviceList_, "dcmiv2_get_device_list");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDevicePowerInfo_, "dcmiv2_get_device_power_info");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDeviceTemperature_, "dcmiv2_get_device_temperature");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDeviceMultiUtilizationRate_, "dcmiv2_get_device_multi_utilization_rate");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDeviceFrequency_, "dcmiv2_get_device_frequency");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetDeviceHbmInfo_, "dcmiv2_get_device_hbm_info");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetHccsLinkBandwidth_, "dcmiv2_get_hccs_link_bandwidth_info");
    MSPTI_LOAD_DCMI_SYM(dcmiv2GetPcieLinkBandwidth_, "dcmiv2_get_pcie_link_bandwidth_info");
#undef MSPTI_LOAD_DCMI_SYM

    // 按芯片代际选择接口代际（驱动确认：910B/310B 仅 v1，V6/950 仅 v2，无交叉回退）；
    // 芯片型号整机一致，以设备 0 为代表查询
    const Common::PlatformType chip = Common::ContextManager::GetInstance()->GetChipType(0);
    if (chip == Common::PlatformType::CHIP_V6)
    {
        if (dcmiv2Init_ == nullptr || dcmiv2GetDeviceList_ == nullptr)
        {
            MSPTI_LOGE("DcmiApi: v2 symbols missing on v2 chip, dcmi metrics disabled");
            return false;
        }
        const int ret = dcmiv2Init_();
        if (ret == DCMI_OK && EnumerateTopologyV2())
        {
            inited_ = true;
            gen_ = DcmiGen::V2;
            MSPTI_LOGI("DcmiApi: init ok (v2), deviceNum=%zu", devices_.size());
            return true;
        }
        MSPTI_LOGE("DcmiApi: v2 init or topology enumeration failed, ret=%d, dcmi metrics disabled", ret);
        return false;
    }
    if (chip == Common::PlatformType::CHIP_910B || chip == Common::PlatformType::CHIP_310B)
    {
        if (dcmiInit_ == nullptr || dcmiGetCardList_ == nullptr || dcmiGetDeviceIdInCard_ == nullptr)
        {
            MSPTI_LOGE("DcmiApi: v1 symbols missing on v1 chip, dcmi metrics disabled");
            return false;
        }
        const int ret = dcmiInit_();
        if (ret == DCMI_OK && EnumerateTopologyV1())
        {
            inited_ = true;
            MSPTI_LOGI("DcmiApi: init ok (v1), deviceNum=%zu", devices_.size());
            return true;
        }
        MSPTI_LOGE("DcmiApi: v1 init or topology enumeration failed, ret=%d, dcmi metrics disabled", ret);
        return false;
    }
    MSPTI_LOGE("DcmiApi: unsupported chip type %d, dcmi metrics disabled", static_cast<int>(chip));
    return false;
}

bool DcmiApi::EnumerateTopologyV2()
{
    // v2：dcmiv2_get_device_list 直接返回扁平 dev_id 列表
    std::vector<int> devList(MAX_DEV_NUM, 0);
    int devNum = 0;
    int ret = dcmiv2GetDeviceList_(devList.data(), &devNum, MAX_DEV_NUM);
    if (ret != DCMI_OK)
    {
        MSPTI_LOGW("DcmiApi: dcmiv2_get_device_list failed, ret=%d", ret);
        return false;
    }
    if (devNum <= 0 || devNum > MAX_DEV_NUM)
    {
        MSPTI_LOGW("DcmiApi: dcmiv2_get_device_list returned invalid device number, devNum=%d", devNum);
        return false;
    }
    devices_.clear();
    for (int i = 0; i < devNum; ++i)
    {
        devices_.push_back(DcmiDevice{static_cast<uint32_t>(devList[i]), -1, -1, true});
    }
    return !devices_.empty();
}

bool DcmiApi::EnumerateTopologyV1()
{
    // v1: (card_id, chip_id) 两级拓扑，扁平 deviceId 按 (card, chip) 顺序展开，
    // 与 ACL 逻辑 deviceId 编号一致
    std::vector<int> cardList(MAX_CARD_NUM, 0);
    int cardNum = 0;
    int ret = dcmiGetCardList_(&cardNum, cardList.data(), MAX_CARD_NUM);
    if (ret != DCMI_OK)
    {
        MSPTI_LOGW("DcmiApi: dcmi_get_card_list failed, ret=%d", ret);
        return false;
    }
    if (cardNum <= 0 || cardNum > MAX_CARD_NUM)
    {
        MSPTI_LOGW("DcmiApi: dcmi_get_card_list returned invalid card number, cardNum=%d", cardNum);
        return false;
    }
    devices_.clear();
    uint32_t flat = 0;
    for (int i = 0; i < cardNum; ++i)
    {
        int devMax = 0;
        int mcuId = -1;
        int cpuId = -1;
        ret = dcmiGetDeviceIdInCard_(cardList[i], &devMax, &mcuId, &cpuId);
        if (ret != DCMI_OK || devMax <= 0)
        {
            MSPTI_LOGW("DcmiApi: dcmi_get_device_id_in_card failed, card=%d, ret=%d, skipped", cardList[i], ret);
            continue;
        }
        for (int chip = 0; chip < devMax; ++chip)
        {
            devices_.push_back(DcmiDevice{flat++, cardList[i], chip, false});
        }
    }
    if (devices_.empty())
    {
        MSPTI_LOGW("DcmiApi: no NPU device enumerated");
        return false;
    }
    return true;
}

const DcmiApi::DcmiDevice *DcmiApi::ResolveDevice(uint32_t deviceId) const
{
    for (const auto &dev : devices_)
    {
        if (dev.deviceId == deviceId)
        {
            return &dev;
        }
    }
    return nullptr;
}

int DcmiApi::GetPower(uint32_t deviceId, int *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetDevicePowerInfo_ != nullptr ? dcmiv2GetDevicePowerInfo_(dev->deviceId, out) : -1;
    }
    return dcmiGetDevicePowerInfo_ != nullptr ? dcmiGetDevicePowerInfo_(dev->cardId, dev->chipId, out) : -1;
}

int DcmiApi::GetTemperature(uint32_t deviceId, int *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetDeviceTemperature_ != nullptr ? dcmiv2GetDeviceTemperature_(dev->deviceId, out) : -1;
    }
    return dcmiGetDeviceTemperature_ != nullptr ? dcmiGetDeviceTemperature_(dev->cardId, dev->chipId, out) : -1;
}

int DcmiApi::GetMultiUtilizationRate(uint32_t deviceId, DcmiMultiUtilizationInfo *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetDeviceMultiUtilizationRate_ != nullptr
                   ? dcmiv2GetDeviceMultiUtilizationRate_(dev->deviceId, out)
                   : -1;
    }
    return dcmiGetDeviceMultiUtilizationRate_ != nullptr
               ? dcmiGetDeviceMultiUtilizationRate_(dev->cardId, dev->chipId, out)
               : -1;
}

int DcmiApi::GetFrequency(uint32_t deviceId, int type, unsigned int *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetDeviceFrequency_ != nullptr ? dcmiv2GetDeviceFrequency_(dev->deviceId, type, out) : -1;
    }
    return dcmiGetDeviceFrequency_ != nullptr ? dcmiGetDeviceFrequency_(dev->cardId, dev->chipId, type, out) : -1;
}

int DcmiApi::GetHbmInfo(uint32_t deviceId, DcmiHbmInfo *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetDeviceHbmInfo_ != nullptr ? dcmiv2GetDeviceHbmInfo_(dev->deviceId, out) : -1;
    }
    return dcmiGetDeviceHbmInfo_ != nullptr ? dcmiGetDeviceHbmInfo_(dev->cardId, dev->chipId, out) : -1;
}

int DcmiApi::GetHccsBandwidth(uint32_t deviceId, DcmiHccsBandwidthInfo *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetHccsLinkBandwidth_ != nullptr ? dcmiv2GetHccsLinkBandwidth_(dev->deviceId, out) : -1;
    }
    return dcmiGetHccsLinkBandwidth_ != nullptr ? dcmiGetHccsLinkBandwidth_(dev->cardId, dev->chipId, out) : -1;
}

int DcmiApi::GetPcieBandwidth(uint32_t deviceId, DcmiPcieBandwidthInfo *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr)
    {
        return -1;
    }
    if (dev->isV2)
    {
        return dcmiv2GetPcieLinkBandwidth_ != nullptr ? dcmiv2GetPcieLinkBandwidth_(dev->deviceId, out) : -1;
    }
    return dcmiGetPcieLinkBandwidth_ != nullptr ? dcmiGetPcieLinkBandwidth_(dev->cardId, dev->chipId, out) : -1;
}

int DcmiApi::GetRdmaBandwidth(uint32_t deviceId, unsigned int portId, unsigned int profTimeMs,
                              DcmiRdmaBandwidthInfo *out) const
{
    const DcmiDevice *dev = ResolveDevice(deviceId);
    if (dev == nullptr || out == nullptr || dev->isV2)  // V6/950 无 rdma 能力，v2 设备直接短路
    {
        return -1;
    }
    return dcmiGetRdmaBandwidth_ != nullptr ? dcmiGetRdmaBandwidth_(dev->cardId, dev->chipId, portId, profTimeMs, out)
                                            : -1;
}

}  // namespace PmSampling
}  // namespace Mspti
