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
#include <cstring>

#include "csrc/common/inject/plog_inject.h"
#include "csrc/common/utils.h"
#include "securec.h"

#define PLOG_TYPE_MSPTI 31
#define MAX_LOG_BUF_SIZE 1024
#define FILENAME (strrchr("/" __FILE__, '/') + 1)

namespace Mspti
{
namespace Common
{
// Per-site sampling windows by call-site frequency.
constexpr uint64_t LOG_EVERY_N_FAILURE = 5000;      // persistent error paths
constexpr uint64_t LOG_EVERY_N_HOT_PATH = 1000;     // per-event hot paths
constexpr uint64_t LOG_EVERY_N_BACKPRESSURE = 100;  // queue backpressure

class PlogManager
{
   public:
    static PlogManager *GetInstance()
    {
        static PlogManager instance;
        return &instance;
    }

    template <typename... T>
    void Log(int level, const char *fileName, int linNo, const char *fmt, T... args)
    {
        if (CheckLogLevelForC(type_, level) == 1)
        {
            char buf[MAX_LOG_BUF_SIZE] = {0};
            if (sprintf_s(buf, MAX_LOG_BUF_SIZE, "(mspti)[%s:%d] >>> (tid:%u) %s\n", fileName, linNo, Utils::GetTid(),
                          fmt) < 0)
            {
                return;
            }
            DlogInnerForC(type_, level, buf, args...);
        }
    }

    static bool IsLogEnabled(int level) { return CheckLogLevelForC(type_, level) == 1; }

    // Log once per site.
    static bool ShouldLogOnce(std::atomic<bool> &flag)
    {
        if (flag.load(std::memory_order_relaxed))
        {
            return false;
        }
        return !flag.exchange(true, std::memory_order_relaxed);
    }

    // Log the first event, then every n events per site.
    static bool ShouldLogEveryN(std::atomic<uint64_t> &counter, uint64_t n)
    {
        return n == 0 || counter.fetch_add(1, std::memory_order_relaxed) % n == 0;
    }

    // Add `increment` to counter; return the total if this event should be logged, else 0.
    // Sampling is by n-boundary crossing, so increment > 1 works too.
    static uint64_t LogEveryNTotal(std::atomic<uint64_t> &counter, uint64_t n, uint64_t increment = 1)
    {
        const auto old = counter.fetch_add(increment, std::memory_order_relaxed);
        const auto total = old + increment;
        if (n == 0 || old == 0 || old / n != total / n)
        {
            return total;
        }
        return 0;
    }

   private:
    PlogManager() = default;
    explicit PlogManager(const PlogManager &obj) = delete;
    PlogManager &operator=(const PlogManager &obj) = delete;
    explicit PlogManager(PlogManager &&obj) = delete;
    PlogManager &operator=(PlogManager &&obj) = delete;

   private:
    static constexpr int type_{PLOG_TYPE_MSPTI};
};

// Single logging entry shared by all macros; FILENAME/__LINE__ expand at the call site.
#define MSPTI_LOG_IMPL(level, format, ...) \
    Mspti::Common::PlogManager::GetInstance()->Log(level, FILENAME, __LINE__, format, ##__VA_ARGS__)

#define MSPTI_LOGE(format, ...) MSPTI_LOG_IMPL(DLOG_ERROR, format, ##__VA_ARGS__)
#define MSPTI_LOGI(format, ...) MSPTI_LOG_IMPL(DLOG_INFO, format, ##__VA_ARGS__)
#define MSPTI_LOGW(format, ...) MSPTI_LOG_IMPL(DLOG_WARN, format, ##__VA_ARGS__)
#define MSPTI_LOGD(format, ...) MSPTI_LOG_IMPL(DLOG_DEBUG, format, ##__VA_ARGS__)
#define MSPTI_EVENT(format, ...) MSPTI_LOG_IMPL(DLOG_EVENT, format, ##__VA_ARGS__)

// Rate-limited logging for hot paths; *_TOTAL variants append the accumulated total
// as the last format argument.
#define MSPTI_LOG_ONCE(level, format, ...)                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        static std::atomic<bool> mspti_log_once_flag{false};                                                           \
        if (!mspti_log_once_flag.load(std::memory_order_relaxed) && Mspti::Common::PlogManager::IsLogEnabled(level) && \
            Mspti::Common::PlogManager::ShouldLogOnce(mspti_log_once_flag))                                            \
        {                                                                                                              \
            MSPTI_LOG_IMPL(level, format, ##__VA_ARGS__);                                                              \
        }                                                                                                              \
    } while (0)

// Skip counting when the level is filtered out.
#define MSPTI_LOG_EVERY_N(level, n, format, ...)                                         \
    do                                                                                   \
    {                                                                                    \
        static std::atomic<uint64_t> mspti_log_every_n_counter{0};                       \
        if (Mspti::Common::PlogManager::IsLogEnabled(level) &&                           \
            Mspti::Common::PlogManager::ShouldLogEveryN(mspti_log_every_n_counter, (n))) \
        {                                                                                \
            MSPTI_LOG_IMPL(level, format, ##__VA_ARGS__);                                \
        }                                                                                \
    } while (0)

// Level check first so nothing is counted while filtered out.
#define MSPTI_LOG_EVERY_N_TOTAL(level, n, counter, format, ...)                                          \
    do                                                                                                   \
    {                                                                                                    \
        if (Mspti::Common::PlogManager::IsLogEnabled(level))                                             \
        {                                                                                                \
            const uint64_t mspti_log_total = Mspti::Common::PlogManager::LogEveryNTotal((counter), (n)); \
            if (mspti_log_total != 0)                                                                    \
            {                                                                                            \
                MSPTI_LOG_IMPL(level, format, ##__VA_ARGS__, mspti_log_total);                           \
            }                                                                                            \
        }                                                                                                \
    } while (0)

// Like EVERY_N_TOTAL with a custom increment per event.
#define MSPTI_LOG_EVERY_N_TOTAL_INC(level, n, counter, increment, format, ...)                                        \
    do                                                                                                                \
    {                                                                                                                 \
        if (Mspti::Common::PlogManager::IsLogEnabled(level))                                                          \
        {                                                                                                             \
            const uint64_t mspti_log_total = Mspti::Common::PlogManager::LogEveryNTotal((counter), (n), (increment)); \
            if (mspti_log_total != 0)                                                                                 \
            {                                                                                                         \
                MSPTI_LOG_IMPL(level, format, ##__VA_ARGS__, mspti_log_total);                                        \
            }                                                                                                         \
        }                                                                                                             \
    } while (0)

#define MSPTI_LOGE_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_ERROR, format, ##__VA_ARGS__)
#define MSPTI_LOGW_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_WARN, format, ##__VA_ARGS__)
#define MSPTI_LOGI_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_INFO, format, ##__VA_ARGS__)
#define MSPTI_LOGD_ONCE(format, ...) MSPTI_LOG_ONCE(DLOG_DEBUG, format, ##__VA_ARGS__)

#define MSPTI_LOGE_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_ERROR, n, format, ##__VA_ARGS__)
#define MSPTI_LOGW_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_WARN, n, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_INFO, n, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N(n, format, ...) MSPTI_LOG_EVERY_N(DLOG_DEBUG, n, format, ##__VA_ARGS__)

#define MSPTI_LOGE_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_ERROR, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGW_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_WARN, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_INFO, n, counter, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N_TOTAL(n, counter, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL(DLOG_DEBUG, n, counter, format, ##__VA_ARGS__)

#define MSPTI_LOGW_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_WARN, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGE_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_ERROR, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGI_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_INFO, n, counter, increment, format, ##__VA_ARGS__)
#define MSPTI_LOGD_EVERY_N_TOTAL_INC(n, counter, increment, format, ...) \
    MSPTI_LOG_EVERY_N_TOTAL_INC(DLOG_DEBUG, n, counter, increment, format, ##__VA_ARGS__)

}  // namespace Common
}  // namespace Mspti
#endif
