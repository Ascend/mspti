/* -------------------------------------------------------------------------
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This file is part of the MindStudio project.
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

#include "csrc/common/shutdown_flag.h"

#include <atomic>

namespace Mspti
{
namespace Common
{
namespace
{
// 常量初始化，不参与动态析构，退出阶段可安全读写。
std::atomic<bool> g_shuttingDown{false};
}  // namespace

void SetShuttingDown(bool value) noexcept { g_shuttingDown.store(value, std::memory_order_release); }

bool IsShuttingDown() noexcept { return g_shuttingDown.load(std::memory_order_acquire); }
}  // namespace Common
}  // namespace Mspti
