/*
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 * This file is part of the MindStudio project.
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include "pm_sampling_exception_injection.h"

#include <cstddef>
#include <new>
#include <vector>

#include "csrc/pm_sampling/pm_sampling_decoder.h"

namespace Mspti
{
namespace PmSampling
{
namespace Test
{
namespace
{
thread_local FailurePoint activePoint = FailurePoint::None;
thread_local uint32_t hitCount = 0;
}  // namespace

ScopedAllocationFailure::ScopedAllocationFailure(FailurePoint point)
    : previousPoint_(activePoint), previousHits_(hitCount)
{
    activePoint = point;
    hitCount = 0;
}

ScopedAllocationFailure::~ScopedAllocationFailure()
{
    activePoint = previousPoint_;
    hitCount = previousHits_;
}

uint32_t ScopedAllocationFailure::Hits() const { return hitCount; }

static void ThrowIfRequested(FailurePoint point)
{
    if (activePoint == point)
    {
        ++hitCount;
        throw std::bad_alloc();
    }
}
}  // namespace Test
}  // namespace PmSampling
}  // namespace Mspti

// GNU ld --wrap redirects unresolved references from the API translation unit.
// Normal calls delegate to the real Decoder. Exceptions use ordinary C++ frames,
// without runtime instruction patching. Keep these names in sync with CMakeLists.txt.
extern "C" msptiResult __real__ZN5Mspti10PmSampling17PmSamplingDecoder7GetSizeEjjmRm(uint32_t numMetrics,
                                                                                     uint32_t maxEntries,
                                                                                     uint64_t maxSamples, size_t &size);

extern "C" msptiResult __wrap__ZN5Mspti10PmSampling17PmSamplingDecoder7GetSizeEjjmRm(uint32_t numMetrics,
                                                                                     uint32_t maxEntries,
                                                                                     uint64_t maxSamples, size_t &size)
{
    Mspti::PmSampling::Test::ThrowIfRequested(Mspti::PmSampling::Test::FailurePoint::GetSize);
    return __real__ZN5Mspti10PmSampling17PmSamplingDecoder7GetSizeEjjmRm(numMetrics, maxEntries, maxSamples, size);
}

extern "C" msptiResult __real__ZN5Mspti10PmSampling17PmSamplingDecoder9CheckPlanEPKhmRKSt6vectorIjSaIjEEjm(
    const uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds, uint32_t maxEntries,
    uint64_t maxSamples);

extern "C" msptiResult __wrap__ZN5Mspti10PmSampling17PmSamplingDecoder9CheckPlanEPKhmRKSt6vectorIjSaIjEEjm(
    const uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds, uint32_t maxEntries, uint64_t maxSamples)
{
    Mspti::PmSampling::Test::ThrowIfRequested(Mspti::PmSampling::Test::FailurePoint::CheckPlan);
    return __real__ZN5Mspti10PmSampling17PmSamplingDecoder9CheckPlanEPKhmRKSt6vectorIjSaIjEEjm(image, size, metricIds,
                                                                                               maxEntries, maxSamples);
}
