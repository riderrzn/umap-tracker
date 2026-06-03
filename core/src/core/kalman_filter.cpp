// SPDX-License-Identifier: Apache-2.0
// Kalman Filter implementation
// Note: Core filtering logic is in umap/core/kalman_filter.h (header-only)
// This file provides factory functions and configuration helpers.

#include "umap/core/kalman_filter.h"

namespace umap::core {

KalmanFilter1D create_default_1d_filter() noexcept {
    return KalmanFilter1D{0.01, 4.0};
}

KalmanFilterND create_gps_filter(double process_noise,
                                  double measurement_noise,
                                  double altitude_noise) noexcept {
    return KalmanFilterND{process_noise, measurement_noise, 0.01, altitude_noise};
}

KalmanFilterND create_default_gps_filter() noexcept {
    return KalmanFilterND{0.001, 4.0, 0.01, 10.0};
}

}  // namespace umap::core
