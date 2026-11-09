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

#ifndef MSPTI_PM_SAMPLING_EXCEPTION_INJECTION_H
#define MSPTI_PM_SAMPLING_EXCEPTION_INJECTION_H

#include <cstdint>

namespace Mspti
{
namespace PmSampling
{
namespace Test
{
enum class FailurePoint
{
    None,
    GetSize,
    CheckPlan
};

// Only the calling test thread is affected; restore state even after a fatal assertion.
class ScopedAllocationFailure
{
   public:
    explicit ScopedAllocationFailure(FailurePoint point);
    ~ScopedAllocationFailure();
    ScopedAllocationFailure(const ScopedAllocationFailure &) = delete;
    ScopedAllocationFailure &operator=(const ScopedAllocationFailure &) = delete;
    uint32_t Hits() const;

   private:
    FailurePoint previousPoint_;
    uint32_t previousHits_;
};
}  // namespace Test
}  // namespace PmSampling
}  // namespace Mspti
#endif
