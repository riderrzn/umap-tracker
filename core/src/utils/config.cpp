// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Runtime configuration loading (environment variables)

#include <umap/utils/config.h>

#ifndef __ANDROID__
#include <cstdlib>
#include <cerrno>
#include <climits>

namespace {

long parse_env_long(const char* env_val, long default_val) noexcept {
    if (env_val == nullptr) {
        return default_val;
    }
    errno = 0;
    char* end_ptr = nullptr;
    long val = std::strtol(env_val, &end_ptr, 10);
    if ((errno == ERANGE) || (end_ptr == env_val) || (*end_ptr != '\0')) {
        return default_val;
    }
    return val;
}

}  // namespace
#endif

namespace umap::utils {

void load_config_from_environment() noexcept {
#ifndef __ANDROID__
    RuntimeConfig& config = RuntimeConfig::instance();

    long val = parse_env_long(std::getenv("UMAP_MAX_QUEUE_SIZE"), -1L);
    if (val > 0L) {
        config.set_max_queue_size(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_MESSAGE_TTL_HOURS"), -1L);
    if (val > 0L) {
        config.set_message_ttl_hours(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_TRACK_RETENTION_DAYS"), -1L);
    if (val > 0L) {
        config.set_track_retention_days(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_MAX_TRACK_POINTS"), -1L);
    if (val > 0L) {
        config.set_max_track_points(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_GPS_INTERVAL_MS"), -1L);
    if (val >= 1000L) {
        config.set_gps_interval_ms(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_MIN_GPS_ACCURACY_M"), -1L);
    if (val > 0L) {
        config.set_min_gps_accuracy_m(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_GPS_TIMEOUT_MS"), -1L);
    if (val > 0L) {
        config.set_gps_timeout_ms(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_UPLOAD_TIMEOUT_MS"), -1L);
    if (val > 0L) {
        config.set_upload_timeout_ms(static_cast<uint32_t>(val));
    }

    val = parse_env_long(std::getenv("UMAP_COLD_START_MAX_MS"), -1L);
    if (val > 0L) {
        config.set_cold_start_max_ms(static_cast<uint32_t>(val));
    }
#endif
}

}  // namespace umap::utils
