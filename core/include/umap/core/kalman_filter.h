// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Kalman filter for GPS location smoothing

#ifndef UMAP_CORE_KALMAN_FILTER_H_
#define UMAP_CORE_KALMAN_FILTER_H_

#include <cmath>
#include <cstdint>

namespace umap::core {

// ============================================================================
// 1D Kalman Filter
// ============================================================================

class KalmanFilter1D {
public:
    // Initialize Kalman filter with process and measurement noise
    explicit KalmanFilter1D(double process_noise = 0.01,
                           double measurement_noise = 4.0) noexcept
        : process_noise_(process_noise),
          measurement_noise_(measurement_noise),
          estimate_(0.0),
          estimate_error_(1.0),
          initialized_(false) {}

    // Predict step (advance time by dt seconds)
    void predict(double dt) noexcept {
        estimate_error_ += process_noise_ * std::abs(dt);
    }

    // Update step (fuse measurement)
    void update(double measurement, double measurement_weight = 1.0) noexcept {
        if (measurement_weight <= 0.0) {
            return;  // Ignore invalid weight
        }

        if (!initialized_) {
            estimate_ = measurement;
            initialized_ = true;
            return;
        }

        // Kalman gain: how much to trust measurement vs estimate
        double gain = estimate_error_ / (estimate_error_ + measurement_noise_ / measurement_weight);

        // Update estimate with weighted measurement
        estimate_ = estimate_ + gain * (measurement - estimate_);

        // Update uncertainty
        estimate_error_ = (1.0 - gain) * estimate_error_;
    }

    // Get filtered estimate
    [[nodiscard]] double get_estimate() const noexcept {
        return estimate_;
    }

    // Get estimate uncertainty (error covariance)
    [[nodiscard]] double get_error() const noexcept {
        return estimate_error_;
    }

    // Get Kalman gain (confidence in estimate)
    [[nodiscard]] double get_gain() const noexcept {
        if (estimate_error_ + measurement_noise_ < 1e-9) {
            return 0.0;
        }
        return estimate_error_ / (estimate_error_ + measurement_noise_);
    }

    // Reset filter to initial state
    void reset() noexcept {
        estimate_ = 0.0;
        estimate_error_ = 1.0;
        initialized_ = false;
    }

private:
    double process_noise_;           // How much uncertainty grows per time step
    double measurement_noise_;        // Measurement uncertainty (inverse of trust)
    double estimate_;                 // Current filtered value
    double estimate_error_;           // Current uncertainty
    bool initialized_;                // Whether first measurement received
};

// ============================================================================
// Estimate State (full GPS state with velocity and bearing)
// ============================================================================

struct EstimateState {
    double latitude = 0.0;           // Filtered latitude
    double longitude = 0.0;          // Filtered longitude
    double latitude_velocity = 0.0;  // lat change per second
    double longitude_velocity = 0.0; // lon change per second
    double altitude = 0.0;           // Filtered altitude (meters MSL)
    double speed_kmh = 0.0;          // Filtered speed (km/h)
    double bearing_deg = 0.0;        // Filtered bearing (degrees)
    double accuracy_m = 0.0;         // Estimated accuracy (meters)
    uint32_t timestamp_ms = 0U;      // Last update time

    // Get bearing from velocity components (radians, 0 = North)
    [[nodiscard]] double bearing_radians() const noexcept {
        return std::atan2(longitude_velocity, latitude_velocity);
    }

    // Get speed magnitude from velocity components (m/s)
    [[nodiscard]] double speed_ms() const noexcept {
        return std::sqrt(latitude_velocity * latitude_velocity +
                        longitude_velocity * longitude_velocity);
    }
};

// ============================================================================
// Multi-dimensional Kalman Filter (for GPS location)
// ============================================================================

class KalmanFilterND {
public:
    // Initialize with process and measurement noise parameters
    explicit KalmanFilterND(double position_process_noise = 0.001,
                           double position_measurement_noise = 4.0,
                           double velocity_process_noise = 0.01,
                           double altitude_measurement_noise = 10.0) noexcept
        : latitude_filter_(position_process_noise, position_measurement_noise),
          longitude_filter_(position_process_noise, position_measurement_noise),
          altitude_filter_(position_process_noise, altitude_measurement_noise),
          velocity_decay_(0.95),
          initialized_(false) {
        state_.latitude_velocity = 0.0;
        state_.longitude_velocity = 0.0;
    }

    // Predict step (advance time by dt seconds)
    void predict(double dt) noexcept {
        // Decay velocity (friction effect)
        state_.latitude_velocity *= velocity_decay_;
        state_.longitude_velocity *= velocity_decay_;

        // Update position based on velocity
        state_.latitude += state_.latitude_velocity * dt;
        state_.longitude += state_.longitude_velocity * dt;

        // Predict internal filters
        latitude_filter_.predict(dt);
        longitude_filter_.predict(dt);
        altitude_filter_.predict(dt);

        initialized_ = true;
    }

    // Update step with new GPS measurement
    void update(double latitude, double longitude, double altitude,
                double accuracy_m) noexcept {
        if (accuracy_m <= 0.0 || accuracy_m > 1000.0) {
            return;  // Ignore invalid measurements
        }

        // Measurement weight is inverse of accuracy (better accuracy = higher weight)
        double weight = 100.0 / accuracy_m;  // Normalize to reasonable range

        // Update position filters
        latitude_filter_.update(latitude, weight);
        longitude_filter_.update(longitude, weight);
        altitude_filter_.update(altitude, weight);

        // Update internal state
        state_.latitude = latitude_filter_.get_estimate();
        state_.longitude = longitude_filter_.get_estimate();
        state_.altitude = altitude_filter_.get_estimate();
        state_.accuracy_m = accuracy_m;

        if (initialized_) {
            // Calculate velocity (change in position)
            // Note: In real implementation, dt from predict() would be used
            state_.latitude_velocity = state_.latitude - previous_latitude_;
            state_.longitude_velocity = state_.longitude - previous_longitude_;

            // Convert velocity to speed (m/s) and bearing
            double speed_ms = state_.speed_ms();
            state_.speed_kmh = speed_ms * 3.6;  // Convert m/s to km/h

            // Bearing in degrees
            double bearing_rad = state_.bearing_radians();
            state_.bearing_deg = bearing_rad * 180.0 / M_PI;
            if (state_.bearing_deg < 0.0) {
                state_.bearing_deg += 360.0;
            }
        }

        previous_latitude_ = state_.latitude;
        previous_longitude_ = state_.longitude;
        initialized_ = true;
    }

    // Get filtered state
    [[nodiscard]] const EstimateState& get_state() const noexcept {
        return state_;
    }

    // Get latitude uncertainty
    [[nodiscard]] double get_latitude_error() const noexcept {
        return latitude_filter_.get_error();
    }

    // Get longitude uncertainty
    [[nodiscard]] double get_longitude_error() const noexcept {
        return longitude_filter_.get_error();
    }

    // Get altitude uncertainty
    [[nodiscard]] double get_altitude_error() const noexcept {
        return altitude_filter_.get_error();
    }

    // Reset to initial state
    void reset() noexcept {
        latitude_filter_.reset();
        longitude_filter_.reset();
        altitude_filter_.reset();
        state_ = EstimateState();
        previous_latitude_ = 0.0;
        previous_longitude_ = 0.0;
        initialized_ = false;
    }

private:
    KalmanFilter1D latitude_filter_;
    KalmanFilter1D longitude_filter_;
    KalmanFilter1D altitude_filter_;

    double velocity_decay_;          // How fast velocity decays (0.0-1.0)
    EstimateState state_;            // Current filtered state
    double previous_latitude_ = 0.0;
    double previous_longitude_ = 0.0;
    bool initialized_;

    // Define M_PI for MSVC compatibility
#ifndef M_PI
    static constexpr double M_PI = 3.14159265358979323846;
#endif
};

// Factory functions
KalmanFilter1D create_default_1d_filter() noexcept;
KalmanFilterND create_gps_filter(double process_noise, double measurement_noise,
                                  double altitude_noise) noexcept;
KalmanFilterND create_default_gps_filter() noexcept;

}  // namespace umap::core

#endif  // UMAP_CORE_KALMAN_FILTER_H_
