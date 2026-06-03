// SPDX-License-Identifier: Apache-2.0
// GPS processing pipeline: raw GPS → Kalman filter → MessageQueue

#ifndef UMAP_CORE_GPS_PIPELINE_H_
#define UMAP_CORE_GPS_PIPELINE_H_

#include <cstdint>

#include "umap/core/geo_types.h"
#include "umap/core/kalman_filter.h"
#include "umap/data/message_queue.h"
#include "umap/utils/result.h"

namespace umap::core {

class GpsProcessingPipeline {
public:
    GpsProcessingPipeline(data::MessageQueue& queue,
                           double position_process_noise = 0.001,
                           double position_measurement_noise = 4.0,
                           double altitude_measurement_noise = 10.0) noexcept;

    GpsProcessingPipeline(const GpsProcessingPipeline&) = delete;
    GpsProcessingPipeline& operator=(const GpsProcessingPipeline&) = delete;
    GpsProcessingPipeline(GpsProcessingPipeline&&) = delete;
    GpsProcessingPipeline& operator=(GpsProcessingPipeline&&) = delete;

    utils::Result<void> process_location(const GpsPoint& raw_point);

    void reset() noexcept;

    [[nodiscard]] const KalmanFilterND& filter() const noexcept { return kalman_; }
    [[nodiscard]] uint32_t processed_count() const noexcept { return processed_count_; }
    [[nodiscard]] uint32_t queued_count() const noexcept { return queued_count_; }

private:
    KalmanFilterND kalman_;
    data::MessageQueue& queue_;
    uint32_t last_timestamp_ms_;
    uint32_t processed_count_;
    uint32_t queued_count_;
    bool initialized_;
};

}  // namespace umap::core

#endif  // UMAP_CORE_GPS_PIPELINE_H_