/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
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

// Query all available metrics through the installed public PM Sampling API.
#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "mspti_pm_sampling.h"

namespace
{
volatile std::sig_atomic_t interrupted = 0;
void Interrupt(int) { interrupted = 1; }
void Require(bool condition, const std::string &message)
{
    if (!condition) throw std::runtime_error(message);
}
void Check(msptiResult result, const char *operation)
{
    Require(result == MSPTI_SUCCESS, std::string(operation) + " failed, result=" + std::to_string(result));
}
template <typename T>
T Params(msptiPmSamplingObjectPtr handle = nullptr)
{
    T p{};
    p.structSize = sizeof(T);
    p.pPmSamplingObject = handle;
    return p;
}
uint32_t Number(const char *arg, uint32_t low, uint32_t high)
{
    const std::string value(arg);
    Require(!value.empty() && value.find_first_not_of("0123456789") == std::string::npos, "Expected unsigned integer");
    const auto n = std::stoull(value);
    Require(n >= low && n <= high, "Argument outside supported range");
    return static_cast<uint32_t>(n);
}

struct Session
{
    msptiPmSamplingObjectPtr handle = nullptr;
    bool running = false;
    void Stop()
    {
        auto p = Params<msptiPmSamplingStopParams>(handle);
        Check(msptiPmSamplingStop(&p), "Stop");
        running = false;
        std::cout << "Stop OK\n";
    }
    void Disable()
    {
        auto p = Params<msptiPmSamplingDisableParams>(handle);
        Check(msptiPmSamplingDisable(&p), "Disable");
        handle = nullptr;
        std::cout << "Disable OK\n";
    }
    ~Session()
    {
        if (!handle) return;
        try
        {
            if (running) Stop();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Cleanup: " << e.what() << '\n';
        }
        try
        {
            Disable();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Cleanup: " << e.what() << '\n';
        }
    }
};

struct MetricStats
{
    uint64_t entries = 0;
    uint64_t nonzero = 0;
    double minimum = 0;
    double maximum = 0;
    void Add(double value)
    {
        minimum = entries == 0 ? value : std::min(minimum, value);
        maximum = entries == 0 ? value : std::max(maximum, value);
        ++entries;
        nonzero += (value != 0);
    }
};

struct Reader
{
    msptiPmSamplingObjectPtr handle;
    uint8_t *image;
    size_t imageSize;
    std::map<uint32_t, msptiPmSamplingMetricProperties> properties;
    std::map<uint32_t, MetricStats> stats;
    uint64_t samples = 0;
    uint64_t previousEnd = 0;
    bool overflowSeen = false;

    uint64_t Decode()
    {
        bool overflow = false;
        msptiPmSamplingDecodeStopReason reason = MSPTI_PM_SAMPLING_DECODE_STOP_REASON_OTHER;
        auto decode = Params<msptiPmSamplingDecodeDataParams>(handle);
        decode.pCounterDataImage = image;
        decode.counterDataImageSize = imageSize;
        decode.pOverflow = &overflow;
        decode.pDecodeStopReason = &reason;
        Check(msptiPmSamplingDecodeData(&decode), "DecodeData");
        overflowSeen = overflowSeen || overflow;
        uint64_t capacity = 0, populated = 0, completed = 0;
        auto info = Params<msptiPmSamplingGetCounterDataInfoParams>(handle);
        info.pCounterDataImage = image;
        info.pNumTotalSamples = &capacity;
        info.pNumPopulatedSamples = &populated;
        info.pNumCompletedSamples = &completed;
        Check(msptiPmSamplingGetCounterDataInfo(&info), "GetCounterDataInfo");
        Require(capacity == 128 && completed == populated && populated <= capacity, "Invalid image counts");
        std::cout << "BATCH completed=" << completed << " capacity=" << capacity << " overflow=" << overflow
                  << " reason=" << static_cast<int>(reason) << '\n';
        // Read every entry before the next DecodeData overwrites this image.
        for (uint64_t index = 0; index < completed; ++index)
        {
            uint64_t start = 0, end = 0;
            auto time = Params<msptiPmSamplingGetSampleInfoParams>(handle);
            time.pCounterDataImage = image;
            time.sampleIndex = index;
            time.pStartTimestamp = &start;
            time.pEndTimestamp = &end;
            Check(msptiPmSamplingGetSampleInfo(&time), "GetSampleInfo");
            Require(start > 0 && end >= start && (samples == 0 || start >= previousEnd),
                    "Invalid, overlapping or repeated sample time");
            previousEnd = end;
            uint32_t count = 0;
            auto entries = Params<msptiPmSamplingGetSampleEntriesParams>(handle);
            entries.pCounterDataImage = image;
            entries.sampleIndex = index;
            entries.pNumEntries = &count;
            Check(msptiPmSamplingGetSampleEntries(&entries), "GetSampleEntries(size)");
            Require(count > 0 && count <= 1024, "Invalid entry count");
            std::vector<msptiPmSamplingSampleEntry> values(count);
            entries.maxEntries = count;
            entries.pEntries = values.data();
            Check(msptiPmSamplingGetSampleEntries(&entries), "GetSampleEntries(values)");
            Require(count == values.size(), "Entry count changed while reading image");
            std::set<std::pair<uint32_t, uint32_t>> seen;
            std::set<uint32_t> metricIds;
            for (const auto &entry : values)
            {
                const auto found = properties.find(entry.metricId);
                Require(found != properties.end(), "Unexpected metric ID");
                Require(seen.emplace(entry.metricId, entry.instanceId).second, "Duplicate metric/instance");
                Require(std::isfinite(entry.value), "Non-finite metric value");
                if (found->second.dataType != MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_TEMPERATURE)
                    Require(entry.value >= 0, "Negative non-temperature metric value");
                if (found->second.dataType == MSPTI_PM_SAMPLING_METRIC_DATA_TYPE_PERCENT)
                    Require(entry.value <= 100, "Percentage outside [0,100]");
                metricIds.insert(entry.metricId);
                stats[entry.metricId].Add(entry.value);
                std::cout << "ENTRY," << samples << ',' << start << ',' << end << ',' << entry.metricId << ','
                          << found->second.shortName << ',' << entry.instanceId << ',' << entry.value << ','
                          << found->second.unit << '\n';
            }
            Require(metricIds.size() == properties.size(), "Configured metric missing from sample");
            ++samples;
        }
        return completed;
    }
};
}  // namespace

int main(int argc, char **argv)
{
    std::cout << std::unitbuf;
    if (argc > 1 && std::string(argv[1]) == "--help")
    {
        std::cout << "Usage: pm_sampling [device=0] [seconds=10 (1..60)] "
                     "[interval_ms=100 (1..1000)]\n"
                     "Queries all metrics and explicitly skips unavailable metrics.\n";
        return 0;
    }
    try
    {
        Require(argc <= 4, "Too many arguments; use --help");
        const uint32_t device = argc > 1 ? Number(argv[1], 0, 63) : 0;
        const uint32_t seconds = argc > 2 ? Number(argv[2], 1, 60) : 10;
        const uint32_t interval = argc > 3 ? Number(argv[3], 1, 1000) : 100;
        std::vector<uint32_t> ids;
        std::signal(SIGINT, Interrupt);
        std::signal(SIGTERM, Interrupt);
        std::cout << "LIVE device=" << device << " duration_s=" << seconds << " interval_ms=" << interval
                  << " metrics=all" << '\n';
        Session session;
        auto enable = Params<msptiPmSamplingEnableParams>();
        enable.deviceId = device;
        Check(msptiPmSamplingEnable(&enable), "Enable");
        session.handle = enable.pPmSamplingObject;
        Require(session.handle != nullptr, "Enable returned null handle");
        std::cout << "Enable OK\n";
        {
            uint32_t count = 0;
            msptiPmSamplingGetNumMetricsParams p{};
            p.structSize = sizeof(p);
            p.deviceId = device;
            p.pNumMetrics = &count;
            Check(msptiPmSamplingGetNumMetrics(&p), "GetNumMetrics");
            // The current public contract uses contiguous IDs starting at zero.
            Require(count > 0 && count <= MSPTI_PM_SAMPLING_METRIC_ID_COUNT,
                    "Unexpected metric table size; check matching headers and library");
            for (uint32_t id = 0; id < count; ++id) ids.push_back(id);
        }
        const size_t requested = ids.size();
        std::vector<uint8_t> availability(requested, 0);
        msptiPmSamplingGetCounterAvailabilityParams available{};
        available.structSize = sizeof(available);
        available.deviceId = device;
        available.numMetrics = static_cast<uint32_t>(ids.size());
        available.pMetricIds = ids.data();
        available.pAvailability = availability.data();
        Check(msptiPmSamplingGetCounterAvailability(&available), "GetCounterAvailability");
        std::vector<uint32_t> selected;
        std::map<uint32_t, msptiPmSamplingMetricProperties> properties;
        for (auto id : ids)
        {
            msptiPmSamplingMetricProperties property{};
            msptiPmSamplingGetMetricPropertiesParams p{};
            p.structSize = sizeof(p);
            p.deviceId = device;
            p.metricId = id;
            p.pProperties = &property;
            Check(msptiPmSamplingGetMetricProperties(&p), "GetMetricProperties");
            Require(availability[id] <= 1 && property.available == (availability[id] != 0),
                    "Inconsistent metric availability: " + std::to_string(id));
            if (availability[id] == 0)
            {
                std::cout << "SKIP_UNAVAILABLE id=" << id << " name=" << property.shortName << " unit=" << property.unit
                          << '\n';
                continue;
            }
            selected.push_back(id);
            properties.emplace(id, property);
            std::cout << "METRIC id=" << id << " name=" << property.shortName << " unit=" << property.unit << '\n';
        }
        ids = std::move(selected);
        Require(!ids.empty(), "No available metrics; nothing tested");
        std::cout << "COVERAGE requested=" << requested << " selected=" << ids.size()
                  << " skipped_unavailable=" << requested - ids.size() << '\n';
        auto config = Params<msptiPmSamplingSetConfigParams>(session.handle);
        config.numMetrics = static_cast<uint32_t>(ids.size());
        config.pMetricIds = ids.data();
        config.samplingInterval = static_cast<uint64_t>(interval) * 1000000;
        config.triggerMode = MSPTI_PM_SAMPLING_TRIGGER_MODE_TIME_INTERVAL;
        config.hwBufferAppendMode = MSPTI_PM_SAMPLING_HW_BUFFER_APPEND_MODE_KEEP_OLDEST;
        config.hardwareBufferSize = 4 * 1024 * 1024;
        Check(msptiPmSamplingSetConfig(&config), "SetConfig");
        size_t imageSize = 0;
        auto size = Params<msptiPmSamplingGetCounterDataSizeParams>(session.handle);
        size.maxSamples = 128;
        size.pCounterDataSize = &imageSize;  // numMetrics=0 reuses the configured metric set.
        Check(msptiPmSamplingGetCounterDataSize(&size), "GetCounterDataSize");
        Require(imageSize > 0 && imageSize <= 512 * 1024 * 1024, "Invalid image allocation size");
        void *buffer = nullptr;
        Require(posix_memalign(&buffer, 16, imageSize) == 0, "Image allocation failed");
        std::unique_ptr<void, decltype(&std::free)> memory(buffer, &std::free);
        auto init = Params<msptiPmSamplingCounterDataImageInitializeParams>(session.handle);
        init.counterDataImageSize = imageSize;
        init.pCounterDataImage = static_cast<uint8_t *>(buffer);
        Check(msptiPmSamplingCounterDataImageInitialize(&init), "CounterDataImageInitialize");
        std::cout << "SetConfig / image initialize OK bytes=" << imageSize << '\n';
        Reader reader{session.handle, init.pCounterDataImage, imageSize, properties, {}};
        auto start = Params<msptiPmSamplingStartParams>(session.handle);
        Check(msptiPmSamplingStart(&start), "Start");
        session.running = true;
        std::cout << "Start OK\nENTRY,sample,start_ns,end_ns,metric_id,metric_name,instance_id,value,unit\n";
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(seconds);
        while (!interrupted && std::chrono::steady_clock::now() < deadline)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            reader.Decode();
        }
        session.Stop();
        // Bounded drain; no waiting for new records after Stop.
        bool empty = false;
        for (unsigned batch = 0; batch < 10000; ++batch)
        {
            if (reader.Decode() == 0)
            {
                empty = true;
                break;
            }
        }
        Require(empty && reader.Decode() == 0, "Stopped queue did not remain empty");
        session.Disable();
        Require(!interrupted, "Test interrupted");
        Require(reader.samples > 0, "No completed samples; increase duration and check collection logs");
        Require(!reader.overflowSeen, "Queue overflow observed; inspect consumption rate");
        for (const auto &p : properties)
        {
            const auto &s = reader.stats.at(p.first);
            std::cout << "SUMMARY " << p.second.shortName << " id=" << p.first << " entries=" << s.entries
                      << " nonzero_entries=" << s.nonzero << " min=" << s.minimum << " max=" << s.maximum
                      << " unit=" << p.second.unit << '\n';
        }
        std::cout << "FLOW_PASS samples=" << reader.samples
                  << " stopped_queue_empty=1\n"
                     "DATA_VALIDITY_NOT_VERIFIED: missing device values may be filled with zero. "
                     "Check DCMI logs and compare idle/load measurements.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "FLOW_FAIL: " << e.what() << '\n';
        return 1;
    }
}
