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
#include "csrc/include/mspti_result.h"

#include <cstddef>

namespace
{
/* 按 mspti_result.h 枚举顺序一一对应（0..9 连续码位直接索引）。
 * 枚举尾部新增错误码时必须同步在此扩表 */
const char* const kResultStrings[] = {
    "Success.",                                     /* 0  MSPTI_SUCCESS */
    "One or more of the parameters is invalid.",    /* 1  MSPTI_ERROR_INVALID_PARAMETER */
    "Multiple subscribers are not supported.",      /* 2  MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED */
    "The maximum limit is reached.",                /* 3  MSPTI_ERROR_MAX_LIMIT_REACHED */
    "The device is offline.",                       /* 4  MSPTI_ERROR_DEVICE_OFFLINE */
    "The queue is empty.",                          /* 5  MSPTI_ERROR_QUEUE_EMPTY */
    "The operation requires LD_PRELOAD injection.", /* 6  MSPTI_ERROR_WITHOUT_LD_PRELOAD */
    "MSPTI is not initialized.",                    /* 7  MSPTI_ERROR_NOT_INITIALIZED */
    "The kind is invalid.",                         /* 8  MSPTI_ERROR_INVALID_KIND */
    "The parameter size is not sufficient.",        /* 9  MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT */
};

/* 编译期断言：表长度与枚举连续码数量自动对齐，消除硬编码魔数 */
static_assert(sizeof(kResultStrings) / sizeof(kResultStrings[0]) == MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT + 1,
              "kResultStrings must cover all continuous result codes");
/* 连续业务码必须仍以 9 结尾，防止中段插入新错误码导致索引表错位 */
static_assert(MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT == 9,
              "continuous result codes must not be reordered or inserted");

constexpr size_t kMaxContinuousResultCode = sizeof(kResultStrings) / sizeof(kResultStrings[0]);
}  // namespace

msptiResult msptiGetResultString(msptiResult result, const char** str)
{
    if (str == nullptr)
    {
        return MSPTI_ERROR_INVALID_PARAMETER;
    }
    const int code = static_cast<int>(result);
    if (code == static_cast<int>(MSPTI_ERROR_INNER))
    {
        *str = "An unknown internal error has occurred.";
        return MSPTI_SUCCESS;
    }
    if (code >= 0 && static_cast<size_t>(code) < kMaxContinuousResultCode)
    {
        *str = kResultStrings[code];
        return MSPTI_SUCCESS;
    }
    return MSPTI_ERROR_INVALID_PARAMETER;
}
