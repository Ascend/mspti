/*
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 * Licensed under Mulan PSL v2: http://license.coscl.org.cn/MulanPSL2
 */
#include <dlfcn.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <thread>

#include "csrc/common/context_manager.h"
#include "csrc/pm_sampling/dcmi/dcmi_api.h"
#include "csrc/pm_sampling/dcmi/dcmi_collector.h"
#include "csrc/pm_sampling/pm_sampling_manager.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

namespace
{
using namespace Mspti::PmSampling;
using Mspti::Common::PlatformType;

// Only the driver boundary is replaced. The real loader, dispatch, conversion
// and collection code runs; no installed libdcmi or hardware is accessed.
struct DriverState
{
    int status = 0;
    int count = 2;
    int cardFailure = -1;
    int openFailures = 0;
    int opens = 0;
    const char *missing = "";
    bool zeroMemory = false;
    std::atomic<int> calls{0}, card{-1}, chip{-1}, type{-1}, hccsMs{-1}, pcieMs{-1}, rdmaMs{-1};
};
DriverState *driver = nullptr;

int InitDriver() { return driver->status; }
int Cards(int *count, int *ids, int capacity)
{
    EXPECT_EQ(capacity, 64);
    *count = driver->count;
    ids[0] = 3;
    ids[1] = 8;
    return driver->status;
}
int Chips(int card, int *count, int *, int *)
{
    *count = card == 3 ? 2 : 1;
    return card == driver->cardFailure ? -1 : driver->status;
}
int Devices(int *ids, int *count, int capacity)
{
    EXPECT_EQ(capacity, 64);
    *count = driver->count;
    ids[0] = 3;
    ids[1] = 8;
    return driver->status;
}
void *OpenDriver(const char *, int) { return driver->opens++ < driver->openFailures ? nullptr : driver; }
void *DriverSymbol(void *, const char *name)
{
    if (std::strcmp(name, driver->missing) == 0) return nullptr;
    if (std::strcmp(name, "dcmi_init") == 0 || std::strcmp(name, "dcmiv2_init") == 0)
        return reinterpret_cast<void *>(InitDriver);
    if (std::strcmp(name, "dcmi_get_card_list") == 0) return reinterpret_cast<void *>(Cards);
    if (std::strcmp(name, "dcmi_get_device_id_in_card") == 0) return reinterpret_cast<void *>(Chips);
    if (std::strcmp(name, "dcmiv2_get_device_list") == 0) return reinterpret_cast<void *>(Devices);
    return nullptr;
}

void Fill(int *out) { *out = 42; }
void Fill(unsigned int *out) { *out = 800; }
void Fill(DcmiMultiUtilizationInfo *out) { *out = {11, 22, 33, 44, {}}; }
void Fill(DcmiHbmInfo *out)
{
    out->memory_size = driver->zeroMemory ? 0 : 200;
    out->memory_usage = 50;
    out->bandwith_util_rate = 17;
}
void Fill(DcmiHccsBandwidthInfo *out)
{
    driver->hccsMs = out->profiling_time;
    for (int i = 0; i < DCMI_HCCS_MAX_LINK_NUM; ++i)
    {
        out->tx_bandwidth[i] = i + 0.25;
        out->rx_bandwidth[i] = i + 0.5;
    }
}
void Fill(DcmiPcieBandwidthInfo *out)
{
    driver->pcieMs = out->profiling_time;
    for (int i = 0; i < DCMI_PCIE_PROF_DATA_NUM; ++i)
    {
        out->tx_p_bw[i] = 1;
        out->tx_cpl_bw[i] = 2;
        out->rx_p_bw[i] = 3;
        out->rx_cpl_bw[i] = 4;
        out->tx_np_bw[i] = out->rx_np_bw[i] = 999;  // Non-posted traffic must not be counted.
    }
}
void Fill(DcmiRdmaBandwidthInfo *out) { *out = {5, 7}; }
template <typename T>
int ReadV1(int card, int chip, T *out)
{
    ++driver->calls;
    driver->card = card;
    driver->chip = chip;
    if (driver->status != 0) return driver->status;
    Fill(out);
    return 0;
}
template <typename T>
int ReadV2(int id, T *out)
{
    return ReadV1(id, -1, out);
}
int FrequencyV1(int card, int chip, int type, unsigned int *out)
{
    driver->type = type;
    return ReadV1(card, chip, out);
}
int FrequencyV2(int id, int type, unsigned int *out) { return FrequencyV1(id, -1, type, out); }
int Rdma(int card, int chip, unsigned int port, unsigned int ms, DcmiRdmaBandwidthInfo *out)
{
    EXPECT_EQ(port, 0U);
    driver->rdmaMs = ms;
    return ReadV1(card, chip, out);
}

class PmSamplingDcmiUtest : public testing::Test
{
   protected:
    void SetUp() override { driver = &state; }
    void TearDown() override
    {
        if (session)
        {
            auto *manager = PmSamplingManager::GetInstance();
            {
                std::lock_guard<std::mutex> lock(manager->mtx_);
                manager->objects_.erase(0);
                manager->parsers_.erase(0);
            }
        }
        collector.StopCollector();  // Join every worker before removing driver hooks.
        GlobalMockObject::verify();
        driver = nullptr;
    }
    void MockLoader(PlatformType chip)
    {
        MOCKER(dlopen).stubs().will(invoke(OpenDriver));
        MOCKER(dlsym).stubs().will(invoke(DriverSymbol));
        MOCKER_CPP(&Mspti::Common::ContextManager::GetChipType).stubs().will(returnValue(chip));
    }
    void ConnectReadFunctions()
    {
        api.devices_ = {{0, 3, 1, false}, {8, -1, -1, true}};
        api.dcmiGetDevicePowerInfo_ = ReadV1<int>;
        api.dcmiGetDeviceTemperature_ = ReadV1<int>;
        api.dcmiGetDeviceMultiUtilizationRate_ = ReadV1<DcmiMultiUtilizationInfo>;
        api.dcmiGetDeviceFrequency_ = FrequencyV1;
        api.dcmiGetDeviceHbmInfo_ = ReadV1<DcmiHbmInfo>;
        api.dcmiGetHccsLinkBandwidth_ = ReadV1<DcmiHccsBandwidthInfo>;
        api.dcmiGetPcieLinkBandwidth_ = ReadV1<DcmiPcieBandwidthInfo>;
        api.dcmiGetRdmaBandwidth_ = Rdma;
        api.dcmiv2GetDevicePowerInfo_ = ReadV2<int>;
        api.dcmiv2GetDeviceTemperature_ = ReadV2<int>;
        api.dcmiv2GetDeviceMultiUtilizationRate_ = ReadV2<DcmiMultiUtilizationInfo>;
        api.dcmiv2GetDeviceFrequency_ = FrequencyV2;
        api.dcmiv2GetDeviceHbmInfo_ = ReadV2<DcmiHbmInfo>;
        api.dcmiv2GetHccsLinkBandwidth_ = ReadV2<DcmiHccsBandwidthInfo>;
        api.dcmiv2GetPcieLinkBandwidth_ = ReadV2<DcmiPcieBandwidthInfo>;
        api.inited_ = true;
    }
    void CollectScalar(PmDcmiSample &sample)
    {
        collector.CollectPower(0, sample);
        collector.CollectTemperature(0, sample);
        collector.CollectUtilization(0, sample);
        collector.CollectFrequency(0, sample);
        collector.CollectHbm(0, sample);
    }
    DriverState state;
    DcmiApi api;
    DcmiCollector collector;
    std::shared_ptr<PmSamplingObject> session;
};

TEST_F(PmSamplingDcmiUtest, LoaderV1FlattensCardsAndInitializationIsIdempotent)
{
    MockLoader(PlatformType::CHIP_910B);
    state.openFailures = 1;
    ASSERT_TRUE(api.Init());
    ASSERT_EQ(api.GetDevices().size(), 3U);
    EXPECT_EQ(api.GetDevices()[1].deviceId, 1U);
    EXPECT_EQ(api.GetDevices()[1].cardId, 3);
    EXPECT_EQ(api.GetDevices()[1].chipId, 1);
    EXPECT_EQ(api.GetDevices()[2].cardId, 8);
    EXPECT_FALSE(api.GetDevices()[2].isV2);
    EXPECT_EQ(state.opens, 2);
    EXPECT_TRUE(api.Init());
    EXPECT_EQ(state.opens, 2);
}

TEST_F(PmSamplingDcmiUtest, LoaderV2PreservesSparseDeviceIdsAndDoesNotFallback)
{
    MockLoader(PlatformType::CHIP_V6);
    ASSERT_TRUE(api.Init());
    ASSERT_EQ(api.GetDevices().size(), 2U);
    EXPECT_EQ(api.GetDevices()[0].deviceId, 3U);
    EXPECT_EQ(api.GetDevices()[1].deviceId, 8U);
    EXPECT_TRUE(api.GetDevices()[1].isV2);
    EXPECT_EQ(api.gen_, DcmiApi::DcmiGen::V2);
    EXPECT_FALSE(api.HasRdmaBandwidth());
    DcmiApi missing;
    state.missing = "dcmiv2_init";
    EXPECT_FALSE(missing.Init());  // Available v1 symbols must not substitute for v2.
    DcmiApi failed;
    state.missing = "";
    state.status = -7;
    EXPECT_FALSE(failed.Init());
}

TEST_F(PmSamplingDcmiUtest, LoaderRejectsMissingLibrarySymbolsAndInitializationFailure)
{
    MockLoader(PlatformType::CHIP_310B);
    state.openFailures = 100;
    EXPECT_FALSE(api.Init());
    EXPECT_TRUE(api.GetDevices().empty());
    state.openFailures = 0;
    state.missing = "dcmi_get_card_list";
    EXPECT_FALSE(api.Init());
    state.missing = "";
    state.status = -7;
    EXPECT_FALSE(api.Init());
    EXPECT_FALSE(api.inited_);
}

TEST_F(PmSamplingDcmiUtest, TopologyRejectsInvalidCountsAndSkipsFailedCards)
{
    api.dcmiGetCardList_ = Cards;
    api.dcmiGetDeviceIdInCard_ = Chips;
    api.dcmiv2GetDeviceList_ = Devices;
    for (int count : {0, -1, 65})
    {
        state.count = count;
        EXPECT_FALSE(api.EnumerateTopologyV1());
        EXPECT_FALSE(api.EnumerateTopologyV2());
    }
    state.count = 2;
    state.status = -7;
    EXPECT_FALSE(api.EnumerateTopologyV1());
    EXPECT_FALSE(api.EnumerateTopologyV2());
    state.status = 0;
    state.cardFailure = 3;
    ASSERT_TRUE(api.EnumerateTopologyV1());
    ASSERT_EQ(api.GetDevices().size(), 1U);
    EXPECT_EQ(api.GetDevices()[0].cardId, 8);
    EXPECT_EQ(api.GetDevices()[0].deviceId, 0U);
    state.count = 1;
    EXPECT_FALSE(api.EnumerateTopologyV1());
}

TEST_F(PmSamplingDcmiUtest, AccessorsDispatchToCorrectGenerationAndRejectInvalidArguments)
{
    ConnectReadFunctions();
    int integer = 0;
    unsigned int frequency = 0;
    DcmiMultiUtilizationInfo utilization{};
    DcmiHbmInfo hbm{};
    DcmiHccsBandwidthInfo hccs{};
    DcmiPcieBandwidthInfo pcie{};
    DcmiRdmaBandwidthInfo rdma{};
    for (uint32_t id : {0U, 8U})
    {
        EXPECT_EQ(api.GetPower(id, &integer), 0);
        EXPECT_EQ(integer, 42);
        EXPECT_EQ(api.GetTemperature(id, &integer), 0);
        EXPECT_EQ(api.GetMultiUtilizationRate(id, &utilization), 0);
        EXPECT_EQ(utilization.npu_util, 44U);
        EXPECT_EQ(api.GetFrequency(id, 7, &frequency), 0);
        EXPECT_EQ(frequency, 800U);
        EXPECT_EQ(state.type, 7);
        EXPECT_EQ(api.GetHbmInfo(id, &hbm), 0);
        EXPECT_EQ(hbm.memory_size, 200U);
        EXPECT_EQ(api.GetHccsBandwidth(id, &hccs), 0);
        EXPECT_DOUBLE_EQ(hccs.rx_bandwidth[15], 15.5);
        EXPECT_EQ(api.GetPcieBandwidth(id, &pcie), 0);
        EXPECT_EQ(state.card, id == 0 ? 3 : 8);
        EXPECT_EQ(state.chip, id == 0 ? 1 : -1);
        EXPECT_EQ(api.GetPower(id, nullptr), -1);
        EXPECT_EQ(api.GetTemperature(id, nullptr), -1);
        EXPECT_EQ(api.GetMultiUtilizationRate(id, nullptr), -1);
        EXPECT_EQ(api.GetFrequency(id, 7, nullptr), -1);
        EXPECT_EQ(api.GetHbmInfo(id, nullptr), -1);
        EXPECT_EQ(api.GetHccsBandwidth(id, nullptr), -1);
        EXPECT_EQ(api.GetPcieBandwidth(id, nullptr), -1);
    }
    EXPECT_EQ(api.GetRdmaBandwidth(0, 0, 100, &rdma), 0);
    EXPECT_EQ(rdma.tx_bandwidth, 5U);
    EXPECT_EQ(api.GetRdmaBandwidth(8, 0, 100, &rdma), -1);
    EXPECT_EQ(api.GetRdmaBandwidth(0, 0, 100, nullptr), -1);
    const int calls = state.calls;
    EXPECT_EQ(api.GetPower(99, &integer), -1);
    EXPECT_EQ(api.GetTemperature(99, &integer), -1);
    EXPECT_EQ(api.GetMultiUtilizationRate(99, &utilization), -1);
    EXPECT_EQ(api.GetFrequency(99, 7, &frequency), -1);
    EXPECT_EQ(api.GetHbmInfo(99, &hbm), -1);
    EXPECT_EQ(api.GetHccsBandwidth(99, &hccs), -1);
    EXPECT_EQ(api.GetPcieBandwidth(99, &pcie), -1);
    EXPECT_EQ(api.GetRdmaBandwidth(99, 0, 100, &rdma), -1);
    EXPECT_EQ(state.calls, calls);
}

TEST_F(PmSamplingDcmiUtest, ScalarCollectionPreservesUnitsAndHandlesZeroMemory)
{
    ConnectReadFunctions();
    MOCKER(DcmiApi::GetInstance).stubs().will(returnValue(&api));
    PmDcmiSample sample;
    CollectScalar(sample);
    EXPECT_EQ(sample.power, 42);  // Raw 0.1 W; the parser performs the conversion.
    EXPECT_EQ(sample.temperature, 42);
    EXPECT_EQ(sample.aicubeUtil, 11U);
    EXPECT_EQ(sample.aivectorUtil, 22U);
    EXPECT_EQ(sample.aicoreUtil, 33U);
    EXPECT_EQ(sample.npuUtil, 44U);
    EXPECT_EQ(sample.aicoreFreq, 800U);
    EXPECT_EQ(sample.hbmBwUtil, 17U);
    EXPECT_EQ(sample.hbmMemUtil, 25U);
    state.zeroMemory = true;
    PmDcmiSample zero;
    collector.CollectHbm(0, zero);
    EXPECT_EQ(zero.hbmMemUtil, UINT32_MAX);
}

TEST_F(PmSamplingDcmiUtest, MissingSymbolsAndReadErrorsLeaveSentinelsAndNoBandwidth)
{
    api.devices_ = {{0, 3, 1, false}, {8, -1, -1, true}};
    MOCKER(DcmiApi::GetInstance).stubs().will(returnValue(&api));
    for (bool missing : {true, false})
    {
        if (!missing)
        {
            ConnectReadFunctions();
            state.status = -7;
        }
        PmDcmiSample sample;
        CollectScalar(sample);
        collector.CollectHccsBandwidth(0, sample, 10);
        collector.CollectPcieBandwidth(0, sample, 10);
        collector.CollectRdmaBandwidth(0, sample, 100);
        EXPECT_EQ(sample.power, -1);
        EXPECT_EQ(sample.temperature, -1);
        EXPECT_EQ(sample.npuUtil, UINT32_MAX);
        EXPECT_EQ(sample.aicoreUtil, UINT32_MAX);
        EXPECT_EQ(sample.aicubeUtil, UINT32_MAX);
        EXPECT_EQ(sample.aivectorUtil, UINT32_MAX);
        EXPECT_EQ(sample.aicoreFreq, UINT32_MAX);
        EXPECT_EQ(sample.hbmBwUtil, UINT32_MAX);
        EXPECT_EQ(sample.hbmMemUtil, UINT32_MAX);
        EXPECT_TRUE(sample.bandwidths.empty());
    }
}

TEST_F(PmSamplingDcmiUtest, BandwidthCollectionClampsWindowsAndConvertsAllInstances)
{
    ConnectReadFunctions();
    MOCKER(DcmiApi::GetInstance).stubs().will(returnValue(&api));
    for (uint32_t interval : {0U, UINT32_MAX})
    {
        PmDcmiSample sample;
        DcmiCollectNeeds needs;
        collector.RunBandwidth(0, sample, needs, interval);
        EXPECT_TRUE(sample.bandwidths.empty());
        needs.hccsBw = needs.pcieBw = needs.roceBw = true;
        collector.RunBandwidth(0, sample, needs, interval);
        EXPECT_EQ(state.hccsMs, interval == 0 ? 1 : 1000);
        EXPECT_EQ(state.pcieMs, interval == 0 ? 0 : 2000);
        EXPECT_EQ(state.rdmaMs, interval == 0 ? 100 : 10000);
        ASSERT_EQ(sample.bandwidths.size(), 36U);
        for (uint32_t i = 0; i < 16; ++i)
        {
            EXPECT_EQ(sample.bandwidths[2 * i].type, PM_BW_HCCS_TX);
            EXPECT_EQ(sample.bandwidths[2 * i + 1].type, PM_BW_HCCS_RX);
            EXPECT_EQ(sample.bandwidths[2 * i].instanceId, i);
            EXPECT_EQ(sample.bandwidths[2 * i + 1].instanceId, i);
            EXPECT_DOUBLE_EQ(sample.bandwidths[2 * i].bytesPerSec, (i + 0.25) * 1e9);
            EXPECT_DOUBLE_EQ(sample.bandwidths[2 * i + 1].bytesPerSec, (i + 0.5) * 1e9);
        }
        const PmBwType types[]{PM_BW_PCIE_TX, PM_BW_PCIE_RX, PM_BW_ROCE_TX, PM_BW_ROCE_RX};
        const double values[]{9e9, 21e9, 5e6, 7e6};
        for (size_t i = 0; i < 4; ++i)
        {
            EXPECT_EQ(sample.bandwidths[32 + i].type, types[i]);
            EXPECT_EQ(sample.bandwidths[32 + i].instanceId, 0U);
            EXPECT_DOUBLE_EQ(sample.bandwidths[32 + i].bytesPerSec, values[i]);
        }
    }
}

TEST_F(PmSamplingDcmiUtest, DeviceCollectionOnlyReadsRequestedGroupsAndDeliversWindows)
{
    ConnectReadFunctions();
    MOCKER(DcmiApi::GetInstance).stubs().will(returnValue(&api));
    auto *manager = PmSamplingManager::GetInstance();
    session = std::make_shared<PmSamplingObject>(0);
    session->state_ = PmSamplingObject::State::Running;
    session->metricIds_ = {MSPTI_PM_SAMPLING_METRIC_ID_POWER, MSPTI_PM_SAMPLING_METRIC_ID_HBM_MEM_UTIL};
    manager->objects_[0] = session;
    manager->parsers_[0].reset(new PmSamplingParser(session->metricIds_));
    DcmiCollector::TickWork work;
    work.needs.power = work.needs.temperature = work.needs.utilization = work.needs.frequency = work.needs.hbm = true;
    work.tsNs = 100;
    collector.CollectDevice(0, work);
    EXPECT_EQ(session->QueueCount(), 0U);
    EXPECT_EQ(state.calls, 5);
    work.tsNs = 200;
    collector.CollectDevice(0, work);
    PmSessionSample sample;
    ASSERT_TRUE(session->GetQueueSample(0, sample));
    ASSERT_EQ(sample.entries.size(), 2U);
    EXPECT_EQ(sample.startNs, 100U);
    EXPECT_EQ(sample.endNs, 200U);
    EXPECT_DOUBLE_EQ(sample.entries[0].value, 4.2);
    EXPECT_DOUBLE_EQ(sample.entries[1].value, 25.0);
    work.needs = {};
    work.tsNs = 300;
    const int calls = state.calls;
    collector.CollectDevice(0, work);
    EXPECT_EQ(state.calls, calls);  // No unrequested driver calls.
    ASSERT_TRUE(session->GetQueueSample(1, sample));
    EXPECT_DOUBLE_EQ(sample.entries[0].value, 0.0);
    EXPECT_DOUBLE_EQ(sample.entries[1].value, 0.0);
}

TEST_F(PmSamplingDcmiUtest, CollectorStartStopAndRestartJoinWorkersAndRespectDemand)
{
    ConnectReadFunctions();
    api.devices_.resize(1);
    MOCKER(DcmiApi::GetInstance).stubs().will(returnValue(&api));
    auto *manager = PmSamplingManager::GetInstance();
    session = std::make_shared<PmSamplingObject>(0);
    session->metricIds_ = {MSPTI_PM_SAMPLING_METRIC_ID_POWER};
    session->intervalNs_ = 1000000;
    manager->objects_[0] = session;
    for (int round = 0; round < 2; ++round)
    {
        session->state_ = PmSamplingObject::State::Running;
        manager->lastDcmiNs_ = 0;
        manager->parsers_[0].reset(new PmSamplingParser(session->metricIds_));
        ASSERT_EQ(collector.StartCollector(), MSPTI_SUCCESS);
        EXPECT_EQ(collector.StartCollector(), MSPTI_SUCCESS);
        EXPECT_EQ(collector.workers_.size(), 1U);
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
        while (session->QueueCount() == 0 && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        EXPECT_GT(session->QueueCount(), 0U);
        collector.StopCollector();
        EXPECT_TRUE(collector.running_);  // A running session still requires the collector.
        {
            std::lock_guard<std::mutex> lock(manager->mtx_);
            std::lock_guard<std::mutex> sessionLock(session->mtx_);
            session->state_ = PmSamplingObject::State::Configured;
        }
        collector.StopCollector();
        EXPECT_FALSE(collector.running_);
        EXPECT_FALSE(collector.coordinator_.joinable());
        EXPECT_TRUE(collector.workers_.empty());
        EXPECT_TRUE(collector.deviceIds_.empty());
        collector.StopCollector();
        session->queue_.clear();
    }
}
}  // namespace
