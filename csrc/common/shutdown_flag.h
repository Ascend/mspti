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

#ifndef MSPTI_COMMON_SHUTDOWN_FLAG_H
#define MSPTI_COMMON_SHUTDOWN_FLAG_H

namespace Mspti
{
namespace Common
{
// 标记库已进入关闭流程；读侧看到后丢弃设备数据，不再调用解析器单例。
void SetShuttingDown(bool value) noexcept;

bool IsShuttingDown() noexcept;
}  // namespace Common
}  // namespace Mspti

#endif  // MSPTI_COMMON_SHUTDOWN_FLAG_H
