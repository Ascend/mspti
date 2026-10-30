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

#ifndef MSPTI_CONVERT_TSFW_CONVERT_H
#define MSPTI_CONVERT_TSFW_CONVERT_H

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <vector>

#include "csrc/activity/ascend/channel/basic_convert.h"
#include "csrc/activity/ascend/channel/channel_data.h"

namespace Mspti
{
namespace Convert
{
class TsfwConvert : public BasicConvert<TsfwConvert, StepTraceBasic>
{
   public:
    static TsfwConvert& GetInstance();
    ~TsfwConvert() = default;
    size_t GetStructSize(Common::PlatformType chipType) const;
    TransFunc GetTransFunc(Common::PlatformType chipType) const;

   private:
    TsfwConvert() = default;
};
}  // namespace Convert
}  // namespace Mspti

#endif  // MSPTI_CONVERT_TSFW_CONVERT_H
