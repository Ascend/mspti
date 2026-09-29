/* -------------------------------------------------------------------------
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
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

#ifndef MSPTI_RESULT_H
#define MSPTI_RESULT_H

/**
 * @brief MSPTI result codes.
 *
 * Error and result codes returned by MSPTI functions.
 *
 * @note When appending a new error code after the continuous codes (0..9),
 * the lookup table kResultStrings in csrc/common/mspti_result.cpp MUST be
 * extended in sync, otherwise the new code will be misjudged as invalid at
 * runtime. Code 999 is reserved for MSPTI_ERROR_INNER, do not occupy.
 */
typedef enum
{
    MSPTI_SUCCESS = 0,
    MSPTI_ERROR_INVALID_PARAMETER = 1,
    MSPTI_ERROR_MULTIPLE_SUBSCRIBERS_NOT_SUPPORTED = 2,
    MSPTI_ERROR_MAX_LIMIT_REACHED = 3,
    MSPTI_ERROR_DEVICE_OFFLINE = 4,
    MSPTI_ERROR_QUEUE_EMPTY = 5,
    MSPTI_ERROR_WITHOUT_LD_PRELOAD = 6,
    MSPTI_ERROR_NOT_INITIALIZED = 7,
    MSPTI_ERROR_INVALID_KIND = 8,
    MSPTI_ERROR_PARAMETER_SIZE_NOT_SUFFICIENT = 9,
    MSPTI_ERROR_INNER = 999,
    MSPTI_ERROR_FORCE_INT = 0x7fffffff
} msptiResult;

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(__GNUC__) && defined(MSPTI_LIB)
#pragma GCC visibility push(default)
#endif

    /**
     * @brief Get the descriptive string for a MSPTI result code.
     *
     * Returns a pointer to a static, NULL-terminated string in **str that
     * describes the given MSPTI result code. The returned string is a static
     * constant: the caller must not free it and it is safe to use from
     * multiple threads. This API is thread-safe and can be called at any
     * time, it does not require MSPTI to be initialized.
     *
     * @param result [in] The MSPTI result code to translate.
     * @param str [out] Returns pointer to the static result string on success;
     *            it is not modified when the call returns an error.
     *
     * @return MSPTI_SUCCESS on success
     * @return MSPTI_ERROR_INVALID_PARAMETER if @p str is NULL, or if @p result
     * is not a valid MSPTI result code (including MSPTI_ERROR_FORCE_INT and unknown values)
     */
    msptiResult msptiGetResultString(msptiResult result, const char **str);

#if defined(__GNUC__) && defined(MSPTI_LIB)
#pragma GCC visibility pop
#endif

#if defined(__cplusplus)
}
#endif

#endif  // MSPTI_RESULT_H
