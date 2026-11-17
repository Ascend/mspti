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
#ifndef MSPTI_COMMON_PLOG_MANAGER_H
#define MSPTI_COMMON_PLOG_MANAGER_H

#include <sys/syscall.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "csrc/common/inject/plog_inject.h"

#define FILENAME (strrchr("/" __FILE__, '/') + 1)
#define MSPTI_LOGD(format, ...)                                                                                   \
    do                                                                                                            \
    {                                                                                                             \
        printf("[PROFILING] [DEBUG] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                    \
    } while (0)

#define MSPTI_LOGI(format, ...)                                                                                  \
    do                                                                                                           \
    {                                                                                                            \
        printf("[PROFILING] [INFO] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                   \
    } while (0)

#define MSPTI_LOGW(format, ...)                                                                                     \
    do                                                                                                              \
    {                                                                                                               \
        printf("[PROFILING] [WARNING] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                      \
    } while (0)

#define MSPTI_LOGE(format, ...)                                                                                   \
    do                                                                                                            \
    {                                                                                                             \
        printf("[PROFILING] [ERROR] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                    \
    } while (0)

#define MSPTI_EVENT(format, ...)                                                                                  \
    do                                                                                                            \
    {                                                                                                             \
        printf("[PROFILING] [EVENT] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                    \
    } while (0)

#define PRINT_LOGW(format, ...)                                                                                     \
    do                                                                                                              \
    {                                                                                                               \
        printf("[PROFILING] [WARNING] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                      \
    } while (0)

#define PRINT_LOGE(format, ...)                                                                                   \
    do                                                                                                            \
    {                                                                                                             \
        printf("[PROFILING] [ERROR] [%s:%d] >>> (tid:%ld) " format "\n", FILENAME, __LINE__, syscall(SYS_gettid), \
               ##__VA_ARGS__);                                                                                    \
    } while (0)

// UT stub: no rate limiting, keep level for output parity.
#define MSPTI_STUB_LOG_BY_LEVEL(level, format, ...) \
    do                                              \
    {                                               \
        switch (level)                              \
        {                                           \
            case DLOG_ERROR:                        \
                MSPTI_LOGE(format, ##__VA_ARGS__);  \
                break;                              \
            case DLOG_WARN:                         \
                MSPTI_LOGW(format, ##__VA_ARGS__);  \
                break;                              \
            case DLOG_INFO:                         \
                MSPTI_LOGI(format, ##__VA_ARGS__);  \
                break;                              \
            case DLOG_EVENT:                        \
                MSPTI_EVENT(format, ##__VA_ARGS__); \
                break;                              \
            default:                                \
                MSPTI_LOGD(format, ##__VA_ARGS__);  \
                break;                              \
        }                                           \
    } while (0)

#define MSPTI_LOG_IMPL(level, format, ...) MSPTI_STUB_LOG_BY_LEVEL(level, format, ##__VA_ARGS__)
#define MSPTI_LOG_ONCE(level, format, ...) MSPTI_STUB_LOG_BY_LEVEL(level, format, ##__VA_ARGS__)
#define MSPTI_LOG_EVERY_N(level, n, format, ...) MSPTI_STUB_LOG_BY_LEVEL(level, format, ##__VA_ARGS__)

#define MSPTI_LOGE_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_ERROR, format, ##__VA_ARGS__)
#define MSPTI_LOGW_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_WARN, format, ##__VA_ARGS__)
#define MSPTI_LOGI_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_INFO, format, ##__VA_ARGS__)
#define MSPTI_LOGD_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_DEBUG, format, ##__VA_ARGS__)

#define MSPTI_LOGE_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_ERROR, n, format, ##__VA_ARGS__)
#define MSPTI_LOGW_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_WARN, n, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_INFO, n, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_DEBUG, n, format, ##__VA_ARGS__)

// UT stub logs every event but still appends the total.
static inline uint64_t MsptiStubLogTotal(std::atomic<uint64_t> &counter, uint64_t increment)
{
    return counter.fetch_add(increment, std::memory_order_relaxed) + increment;
}

#define MSPTI_LOG_EVERY_N_TOTAL(level, n, counter, format, ...) \
    MSPTI_STUB_LOG_BY_LEVEL(level, format, ##__VA_ARGS__, MsptiStubLogTotal((counter), 1))
#define MSPTI_LOG_EVERY_N_TOTAL_INC(level, n, counter, increment, format, ...) \
    MSPTI_STUB_LOG_BY_LEVEL(level, format, ##__VA_ARGS__, MsptiStubLogTotal((counter), (increment)))

#define MSPTI_LOGE_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_ERROR, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGW_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_WARN, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_INFO, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_DEBUG, n, counter, format, ##__VA_ARGS__)

#define MSPTI_LOGE_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_ERROR, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGW_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_WARN, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_INFO, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_DEBUG, n, counter, increment, format, ##__VA_ARGS__)

namespace Mspti
{
namespace Common
{
constexpr uint64_t LOG_EVERY_N_FAILURE = 5000;
constexpr uint64_t LOG_EVERY_N_HOT_PATH = 1000;
constexpr uint64_t LOG_EVERY_N_BACKPRESSURE = 100;
}  // namespace Common
}  // namespace Mspti
#endif
