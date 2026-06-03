// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Logger implementation (Android NDK + fallback)

#include <umap/utils/logger.h>

#ifdef __ANDROID__
    #include <android/log.h>
    #define ANDROID_LOG_AVAILABLE 1
#else
    #define ANDROID_LOG_AVAILABLE 0
#endif

#include <cstdio>

namespace umap::utils {

// Convert LogLevel to Android log priority
#if ANDROID_LOG_AVAILABLE
static int log_level_to_android(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:
            return ANDROID_LOG_DEBUG;
        case LogLevel::Info:
            return ANDROID_LOG_INFO;
        case LogLevel::Warning:
            return ANDROID_LOG_WARN;
        case LogLevel::Error:
            return ANDROID_LOG_ERROR;
        default:
            return ANDROID_LOG_INFO;
    }
}
#endif

// Convert LogLevel to string prefix
static const char* log_level_to_string(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug:
            return "[D]";
        case LogLevel::Info:
            return "[I]";
        case LogLevel::Warning:
            return "[W]";
        case LogLevel::Error:
            return "[E]";
        default:
            return "[?]";
    }
}

void log_impl(LogLevel level, const char* tag, const char* msg) noexcept {
    if (tag == nullptr || msg == nullptr) {
        return;  // MISRA: null pointer check
    }

#if ANDROID_LOG_AVAILABLE
    // Android NDK logging (primary)
    __android_log_print(log_level_to_android(level), tag, "%s", msg);
#else
    // Fallback: stderr logging (for desktop/testing)
    // NOLINTNEXTLINE (cppcoreguidelines-pro-type-vararg)
    std::fprintf(stderr, "%s %s: %s\n", log_level_to_string(level), tag, msg);
#endif
}

}  // namespace umap::utils
