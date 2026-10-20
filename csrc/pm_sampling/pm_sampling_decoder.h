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

#ifndef MSPTI_PM_SAMPLING_DECODER_H
#define MSPTI_PM_SAMPLING_DECODER_H

#include "csrc/pm_sampling/pm_sampling_contract.h"

namespace Mspti
{
namespace PmSampling
{
// Image-only operations. The API layer owns handle validation, session locking and
// queue consumption. size must be the actual accessible buffer length, obtained
// from the session's image plan for public queries that do not carry a length.
class PmSamplingDecoder
{
   public:
    static msptiResult GetSize(uint32_t numMetrics, uint32_t maxEntries, uint64_t maxSamples, size_t &size);
    static msptiResult Initialize(uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds,
                                  uint32_t maxEntries, uint64_t maxSamples);
    // Match the initialized header/directory to the session plan before consuming samples.
    static msptiResult CheckPlan(const uint8_t *image, size_t size, const std::vector<uint32_t> &metricIds,
                                 uint32_t maxEntries, uint64_t maxSamples);
    // Replaces the populated samples, including clearing the image for an empty batch.
    // The whole batch is checked before any write; no partial record is published.
    static msptiResult WriteSamples(uint8_t *image, size_t size, const std::vector<PmSessionSample> &samples);
    static msptiResult GetInfo(const uint8_t *image, size_t size, uint64_t &total, uint64_t &populated,
                               uint64_t &completed);
    static msptiResult GetSampleInfo(const uint8_t *image, size_t size, uint64_t index, uint64_t &start, uint64_t &end);
    // A null entries pointer queries the count. An undersized array is left intact;
    // count still reports the required capacity. Other failures leave count unchanged.
    static msptiResult GetSampleEntries(const uint8_t *image, size_t size, uint64_t index, uint32_t capacity,
                                        msptiPmSamplingSampleEntry *entries, uint32_t &count);
};
}  // namespace PmSampling
}  // namespace Mspti
#endif
