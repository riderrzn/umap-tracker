// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Logging utilities for Android NDK

#ifndef UMAP_UTILS_LOGGER_H_
#define UMAP_UTILS_LOGGER_H_

#include <cstdint>

namespace umap::utils {

enum class LogLevel : uint8_t {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
};

void log_impl(LogLevel level, const char* tag, const char* msg) noexcept;

// Macro-based logging interface (MISRA compliant)
// In production, these expand to Android NDK __android_log_* calls

#define LOG_DEBUG(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Debug, "umap", msg)
#define LOG_INFO(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Info, "umap", msg)
#define LOG_WARN(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Warning, "umap", msg)
#define LOG_ERROR(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Error, "umap", msg)

}  // namespace umap::utils

#endif  // UMAP_UTILS_LOGGER_H_
