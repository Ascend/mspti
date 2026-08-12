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

#ifndef MSPTI_PROJECT_OVERHEAD_REPORTER_H
#define MSPTI_PROJECT_OVERHEAD_REPORTER_H

#include <cstdint>

#include "csrc/include/mspti_activity.h"

namespace Mspti
{
namespace Reporter
{

struct OverheadRecord
{
    OverheadRecord(msptiActivityOverheadKind kind, msptiActivityObjectKind objectKind);
    ~OverheadRecord();
    void SetDeviceId(uint32_t deviceId) { deviceId_ = deviceId; }

    msptiActivityOverheadKind overheadKind_;
    msptiActivityObjectKind objectKind_;
    uint64_t start_{0};
    uint32_t deviceId_{0};
};
}  // namespace Reporter
}  // namespace Mspti

#endif  // MSPTI_PROJECT_OVERHEAD_REPORTER_H
