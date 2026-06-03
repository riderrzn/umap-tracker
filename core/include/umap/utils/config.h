// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Configuration constants and runtime overrides

#ifndef UMAP_UTILS_CONFIG_H_
#define UMAP_UTILS_CONFIG_H_

#include <cstdint>
#include <mutex>

namespace umap::utils {

// Queue management
constexpr uint32_t MAX_QUEUE_SIZE = 50000U;
constexpr uint32_t MESSAGE_TTL_HOURS = 168U;

// Track storage
constexpr uint32_t TRACK_RETENTION_DAYS = 90U;
constexpr uint32_t MAX_TRACK_POINTS = 1000000U;

// GPS parameters
constexpr uint32_t DEFAULT_GPS_INTERVAL_MS = 5000U;
constexpr uint32_t MIN_GPS_ACCURACY_M = 10U;
constexpr uint32_t GPS_TIMEOUT_MS = 60000U;

// Performance targets
constexpr uint32_t GPS_UI_LATENCY_MAX_MS = 200U;
constexpr uint32_t UPLOAD_TIMEOUT_MS = 500U;
constexpr uint32_t COLD_START_MAX_MS = 3000U;

// Battery drain target (percentage per hour in balanced mode)
constexpr float BATTERY_DRAIN_MAX_PERCENT_PER_HOUR = 5.0f;

// Runtime configuration singleton (allows overriding compile-time defaults)
class RuntimeConfig {
public:
    static RuntimeConfig& instance() noexcept {
        static RuntimeConfig config;
        return config;
    }

    RuntimeConfig(const RuntimeConfig&) = delete;
    RuntimeConfig& operator=(const RuntimeConfig&) = delete;
    RuntimeConfig(RuntimeConfig&&) = delete;
    RuntimeConfig& operator=(RuntimeConfig&&) = delete;

    [[nodiscard]] uint32_t max_queue_size() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return max_queue_size_;
    }

    [[nodiscard]] uint32_t message_ttl_hours() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return message_ttl_hours_;
    }

    [[nodiscard]] uint32_t track_retention_days() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return track_retention_days_;
    }

    [[nodiscard]] uint32_t max_track_points() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return max_track_points_;
    }

    [[nodiscard]] uint32_t gps_interval_ms() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return gps_interval_ms_;
    }

    [[nodiscard]] uint32_t min_gps_accuracy_m() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return min_gps_accuracy_m_;
    }

    [[nodiscard]] uint32_t gps_timeout_ms() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return gps_timeout_ms_;
    }

    [[nodiscard]] uint32_t gps_ui_latency_max_ms() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return gps_ui_latency_max_ms_;
    }

    [[nodiscard]] uint32_t upload_timeout_ms() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return upload_timeout_ms_;
    }

    [[nodiscard]] uint32_t cold_start_max_ms() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return cold_start_max_ms_;
    }

    [[nodiscard]] float battery_drain_max_percent_per_hour() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return battery_drain_max_percent_per_hour_;
    }

    void set_max_queue_size(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        max_queue_size_ = (value > 0U) ? value : MAX_QUEUE_SIZE;
    }

    void set_message_ttl_hours(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        message_ttl_hours_ = (value > 0U) ? value : MESSAGE_TTL_HOURS;
    }

    void set_track_retention_days(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        track_retention_days_ = (value > 0U) ? value : TRACK_RETENTION_DAYS;
    }

    void set_max_track_points(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        max_track_points_ = (value > 0U) ? value : MAX_TRACK_POINTS;
    }

    void set_gps_interval_ms(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        gps_interval_ms_ = (value >= 1000U) ? value : DEFAULT_GPS_INTERVAL_MS;
    }

    void set_min_gps_accuracy_m(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        min_gps_accuracy_m_ = (value > 0U) ? value : MIN_GPS_ACCURACY_M;
    }

    void set_gps_timeout_ms(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        gps_timeout_ms_ = (value > DEFAULT_GPS_INTERVAL_MS) ? value : GPS_TIMEOUT_MS;
    }

    void set_gps_ui_latency_max_ms(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        gps_ui_latency_max_ms_ = (value > 0U) ? value : GPS_UI_LATENCY_MAX_MS;
    }

    void set_upload_timeout_ms(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        upload_timeout_ms_ = (value > 0U) ? value : UPLOAD_TIMEOUT_MS;
    }

    void set_cold_start_max_ms(uint32_t value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        cold_start_max_ms_ = (value > 0U) ? value : COLD_START_MAX_MS;
    }

    void set_battery_drain_max_percent_per_hour(float value) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        battery_drain_max_percent_per_hour_ = (value > 0.0f) ? value : BATTERY_DRAIN_MAX_PERCENT_PER_HOUR;
    }

    void reset_to_defaults() noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        max_queue_size_ = MAX_QUEUE_SIZE;
        message_ttl_hours_ = MESSAGE_TTL_HOURS;
        track_retention_days_ = TRACK_RETENTION_DAYS;
        max_track_points_ = MAX_TRACK_POINTS;
        gps_interval_ms_ = DEFAULT_GPS_INTERVAL_MS;
        min_gps_accuracy_m_ = MIN_GPS_ACCURACY_M;
        gps_timeout_ms_ = GPS_TIMEOUT_MS;
        gps_ui_latency_max_ms_ = GPS_UI_LATENCY_MAX_MS;
        upload_timeout_ms_ = UPLOAD_TIMEOUT_MS;
        cold_start_max_ms_ = COLD_START_MAX_MS;
        battery_drain_max_percent_per_hour_ = BATTERY_DRAIN_MAX_PERCENT_PER_HOUR;
    }

private:
    RuntimeConfig() noexcept {
        reset_to_defaults();
    }

    mutable std::mutex mutex_;

    uint32_t max_queue_size_{};
    uint32_t message_ttl_hours_{};
    uint32_t track_retention_days_{};
    uint32_t max_track_points_{};
    uint32_t gps_interval_ms_{};
    uint32_t min_gps_accuracy_m_{};
    uint32_t gps_timeout_ms_{};
    uint32_t gps_ui_latency_max_ms_{};
    uint32_t upload_timeout_ms_{};
    uint32_t cold_start_max_ms_{};
    float battery_drain_max_percent_per_hour_{};
};

// Load configuration overrides from environment variables
// Safe to call multiple times; only overrides if env var is set
void load_config_from_environment() noexcept;

}  // namespace umap::utils

#endif  // UMAP_UTILS_CONFIG_H_
