// SPDX-License-Identifier: Apache-2.0
// GPS processing pipeline implementation

#include "umap/core/gps_pipeline.h"

#include <cstdio>

namespace umap::core {

GpsProcessingPipeline::GpsProcessingPipeline(
    data::MessageQueue& queue,
    double position_process_noise,
    double position_measurement_noise,
    double altitude_measurement_noise) noexcept
    : kalman_(position_process_noise, position_measurement_noise, 0.01,
              altitude_measurement_noise),
      queue_(queue),
      last_timestamp_ms_(0U),
      processed_count_(0U),
      queued_count_(0U),
      initialized_(false) {}

utils::Result<void> GpsProcessingPipeline::process_location(
    const GpsPoint& raw_point) {
    if (!raw_point.is_valid()) {
        return utils::Unexpected(utils::ErrorCode::OutOfRange);
    }

    double dt = 0.0;
    if (initialized_ && raw_point.timestamp_ms > last_timestamp_ms_) {
        dt = static_cast<double>(raw_point.timestamp_ms - last_timestamp_ms_)
             / 1000.0;
    }

    kalman_.predict(dt);
    kalman_.update(raw_point.location.latitude,
                   raw_point.location.longitude,
                   raw_point.altitude_m,
                   raw_point.accuracy_m);

    GpsPoint filtered_point = raw_point;
    kalman_.populate_point(filtered_point);

    ++processed_count_;

    char buf[512];
    std::snprintf(buf, sizeof(buf),
        "{\"lat\":%.6f,\"lon\":%.6f,\"speed\":%.1f,\"bearing\":%.1f,"
        "\"altitude\":%.1f,\"accuracy\":%.1f,\"timestamp\":%lu,"
        "\"batt\":%lu,\"hdop\":%.1f}",
        filtered_point.location.latitude,
        filtered_point.location.longitude,
        filtered_point.speed_kmh,
        filtered_point.bearing_deg,
        filtered_point.altitude_m,
        filtered_point.accuracy_m,
        static_cast<unsigned long>(filtered_point.timestamp_ms),
        static_cast<unsigned long>(filtered_point.battery_percent),
        filtered_point.hdop);

    auto result = queue_.enqueue(buf);
    if (result.has_value()) {
        ++queued_count_;
    }

    last_timestamp_ms_ = raw_point.timestamp_ms;
    initialized_ = true;

    return utils::Result<void>();
}

void GpsProcessingPipeline::reset() noexcept {
    kalman_.reset();
    last_timestamp_ms_ = 0U;
    processed_count_ = 0U;
    queued_count_ = 0U;
    initialized_ = false;
}

}  // namespace umap::core