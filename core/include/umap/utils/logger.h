// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Logging utilities for Android NDK

#ifndef UMAP_UTILS_LOGGER_H_
#define UMAP_UTILS_LOGGER_H_

#include <cstdint>

namespace umap::utils {

enum class LogLevel : uint8_t {
    Debug = 0U,
    Info = 1U,
    Warning = 2U,
    Error = 3U,
};

// Core logging function (implemented in logger.cpp)
void log_impl(LogLevel level, const char* tag, const char* msg) noexcept;

// Convenience macros for logging
#define LOG_DEBUG(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Debug, "umap", (msg))
#define LOG_INFO(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Info, "umap", (msg))
#define LOG_WARN(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Warning, "umap", (msg))
#define LOG_ERROR(msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Error, "umap", (msg))

// Tagged logging
#define LOG_TAG_DEBUG(tag, msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Debug, (tag), (msg))
#define LOG_TAG_INFO(tag, msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Info, (tag), (msg))
#define LOG_TAG_WARN(tag, msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Warning, (tag), (msg))
#define LOG_TAG_ERROR(tag, msg) ::umap::utils::log_impl(::umap::utils::LogLevel::Error, (tag), (msg))

}  // namespace umap::utils

#endif  // UMAP_UTILS_LOGGER_H_
