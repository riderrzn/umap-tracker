// SPDX-License-Identifier: Apache-2.0
// Tests for Kalman filter (1D and ND)

#include <gtest/gtest.h>
#include <cmath>

#include "umap/core/kalman_filter.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace umap::core;

// ============================================================================
// 1D Kalman Filter Tests
// ============================================================================

class KalmanFilter1DTest : public ::testing::Test {
protected:
    KalmanFilter1D filter{0.01, 4.0};
};

TEST_F(KalmanFilter1DTest, InitialState) {
    EXPECT_DOUBLE_EQ(filter.get_estimate(), 0.0);
    EXPECT_GT(filter.get_error(), 0.0);
}

TEST_F(KalmanFilter1DTest, FirstMeasurementInitializes) {
    filter.update(10.0);
    EXPECT_DOUBLE_EQ(filter.get_estimate(), 10.0);
}

TEST_F(KalmanFilter1DTest, PredictIncreasesUncertainty) {
    filter.update(10.0);
    double error_before = filter.get_error();

    filter.predict(1.0);
    double error_after = filter.get_error();

    EXPECT_GT(error_after, error_before);
}

TEST_F(KalmanFilter1DTest, UpdateReducesUncertainty) {
    filter.update(10.0);
    filter.predict(1.0);
    double error_before = filter.get_error();

    filter.update(10.5);
    double error_after = filter.get_error();

    EXPECT_LT(error_after, error_before);
}

TEST_F(KalmanFilter1DTest, MultipleUpdatesConverge) {
    double true_value = 42.0;

    for (int i = 0; i < 10; ++i) {
        filter.update(true_value);
        filter.predict(0.1);
    }

    double estimate = filter.get_estimate();
    EXPECT_NEAR(estimate, true_value, 1.0);
}

TEST_F(KalmanFilter1DTest, NoiseRejection) {
    double true_value = 100.0;

    // First update with true value
    filter.update(true_value);

    // Add noise (should be mostly rejected)
    for (int i = 0; i < 5; ++i) {
        filter.update(true_value + (i % 2 == 0 ? 5.0 : -5.0));
        filter.predict(0.1);
    }

    double estimate = filter.get_estimate();
    EXPECT_NEAR(estimate, true_value, 5.0);
}

TEST_F(KalmanFilter1DTest, MeasurementWeighting) {
    filter.update(10.0);

    // High weight measurement
    filter.update(20.0, 10.0);
    double estimate_high_weight = filter.get_estimate();

    // Reset and try again with low weight
    filter.reset();
    filter.update(10.0);
    filter.update(20.0, 0.1);
    double estimate_low_weight = filter.get_estimate();

    // High weight should move estimate more towards 20
    EXPECT_GT(estimate_high_weight, estimate_low_weight);
}

TEST_F(KalmanFilter1DTest, InvalidWeightIgnored) {
    filter.update(10.0);
    filter.update(50.0, 1.0);
    double estimate_before = filter.get_estimate();

    // Zero or negative weight should be ignored
    filter.update(100.0, 0.0);
    double estimate_after = filter.get_estimate();

    EXPECT_DOUBLE_EQ(estimate_before, estimate_after);
}

TEST_F(KalmanFilter1DTest, ResetState) {
    filter.update(10.0);
    filter.update(20.0);
    filter.reset();

    EXPECT_DOUBLE_EQ(filter.get_estimate(), 0.0);
}

TEST_F(KalmanFilter1DTest, GainCalibration) {
    filter.update(10.0);
    double gain = filter.get_gain();

    EXPECT_GE(gain, 0.0);
    EXPECT_LE(gain, 1.0);
}

// ============================================================================
// EstimateState Tests
// ============================================================================

TEST(EstimateStateTest, VelocityToSpeed) {
    EstimateState state;
    state.latitude_velocity = 3.0;    // m/s
    state.longitude_velocity = 4.0;   // m/s

    // Pythagorean: 3-4-5 triangle
    double speed = state.speed_ms();
    EXPECT_NEAR(speed, 5.0, 0.01);
}

TEST(EstimateStateTest, VelocityToBearing) {
    EstimateState state;
    state.latitude_velocity = 0.0;
    state.longitude_velocity = 1.0;

    double bearing_rad = state.bearing_radians();
    // Moving east: longitude velocity > 0, latitude velocity = 0 → bearing ≈ 90°
    EXPECT_NEAR(bearing_rad, M_PI / 2.0, 0.1);
}

// ============================================================================
// Multi-dimensional Kalman Filter Tests
// ============================================================================

class KalmanFilterNDTest : public ::testing::Test {
protected:
    KalmanFilterND filter{0.001, 4.0, 0.01, 10.0};
};

TEST_F(KalmanFilterNDTest, InitialState) {
    auto state = filter.get_state();
    EXPECT_DOUBLE_EQ(state.latitude, 0.0);
    EXPECT_DOUBLE_EQ(state.longitude, 0.0);
    EXPECT_DOUBLE_EQ(state.latitude_velocity, 0.0);
    EXPECT_DOUBLE_EQ(state.longitude_velocity, 0.0);
}

TEST_F(KalmanFilterNDTest, FirstUpdateInitializes) {
    filter.update(55.7558, 37.6173, 100.0, 10.0);

    auto state = filter.get_state();
    EXPECT_NEAR(state.latitude, 55.7558, 0.001);
    EXPECT_NEAR(state.longitude, 37.6173, 0.001);
}

TEST_F(KalmanFilterNDTest, InvalidAccuracyIgnored) {
    filter.update(55.7558, 37.6173, 100.0, 10.0);
    auto state_before = filter.get_state();

    // Invalid accuracy (zero)
    filter.update(56.0, 38.0, 100.0, 0.0);
    auto state_after = filter.get_state();

    EXPECT_DOUBLE_EQ(state_before.latitude, state_after.latitude);
}

TEST_F(KalmanFilterNDTest, SequentialUpdatesSmooth) {
    // Simulate GPS track with step pattern
    double lat = 55.0;
    double lon = 37.0;

    filter.update(lat, lon, 100.0, 5.0);

    for (int i = 0; i < 5; ++i) {
        lat += 0.01;  // 0.01 degree ≈ 1.1 km
        filter.predict(0.1);  // Time passes between measurements
        filter.update(lat, lon, 100.0, 5.0);
    }

    auto state = filter.get_state();
    // Filter correctly smooths step changes; tolerance reflects Kalman lag
    EXPECT_NEAR(state.latitude, lat, 0.03);
}

TEST_F(KalmanFilterNDTest, PredictDecaysVelocity) {
    // Create velocity
    filter.update(55.0, 37.0, 100.0, 5.0);
    filter.predict(0.1);
    filter.update(55.01, 37.01, 100.0, 5.0);

    double velocity_before = filter.get_state().speed_ms();

    // Predict without new measurement (velocity decays)
    for (int i = 0; i < 5; ++i) {
        filter.predict(0.1);
    }

    double velocity_after = filter.get_state().speed_ms();
    EXPECT_LT(velocity_after, velocity_before);
}

TEST_F(KalmanFilterNDTest, AltitudeFiltering) {
    filter.update(55.0, 37.0, 100.0, 5.0);
    filter.update(55.001, 37.001, 105.0, 5.0);
    filter.update(55.002, 37.002, 98.0, 5.0);

    auto state = filter.get_state();
    EXPECT_NEAR(state.altitude, 101.0, 5.0);  // Smoothed altitude
}

TEST_F(KalmanFilterNDTest, AccuracyWeighting) {
    // High accuracy measurement
    filter.update(55.0, 37.0, 100.0, 2.0);
    filter.predict(0.1);
    filter.update(55.01, 37.01, 100.0, 2.0);

    auto state1 = filter.get_state();
    double error1 = filter.get_latitude_error();

    // Reset and try with low accuracy
    filter.reset();
    filter.update(55.0, 37.0, 100.0, 50.0);  // Poor accuracy
    filter.predict(0.1);
    filter.update(55.01, 37.01, 100.0, 50.0);

    auto state2 = filter.get_state();
    double error2 = filter.get_latitude_error();

    // Better accuracy should result in lower error
    EXPECT_LT(error1, error2);
}

TEST_F(KalmanFilterNDTest, SpeedCalculation) {
    filter.update(55.0, 37.0, 100.0, 5.0);
    filter.predict(0.1);
    // Create movement
    filter.update(55.001, 37.001, 100.0, 5.0);

    auto state = filter.get_state();
    EXPECT_GT(state.speed_kmh, 0.0);
}

TEST_F(KalmanFilterNDTest, BearingCalculation) {
    filter.update(55.0, 37.0, 100.0, 5.0);
    filter.predict(0.1);
    filter.update(55.001, 37.001, 100.0, 5.0);

    auto state = filter.get_state();
    EXPECT_GE(state.bearing_deg, 0.0);
    EXPECT_LE(state.bearing_deg, 360.0);
}

TEST_F(KalmanFilterNDTest, MoscowToSpb) {
    // Moscow to St Petersburg track (simplified)
    double lat = 55.7558;  // Moscow
    double lon = 37.6173;

    filter.update(lat, lon, 100.0, 10.0);

    // Move north towards SPB
    for (int i = 0; i < 100; ++i) {
        lat += 0.001;  // Move north
        filter.predict(0.01);
        filter.update(lat, lon, 100.0, 10.0);
    }

    auto state = filter.get_state();

    // Should have moved north
    EXPECT_GT(state.latitude, 55.7558);
    EXPECT_NEAR(state.longitude, 37.6173, 0.01);

    // Should have north bearing
    EXPECT_LT(state.bearing_deg, 45.0);  // Roughly north
}

TEST_F(KalmanFilterNDTest, RealWorldAccuracy) {
    // Simulate real GPS with 5-10m accuracy
    double lat = 59.9311;   // SPB
    double lon = 30.3609;
    double accuracy = 7.0;

    for (int i = 0; i < 20; ++i) {
        // Add small variations
        double dlat = (i % 2 == 0 ? 0.0001 : -0.00005);
        filter.update(lat + dlat, lon, 100.0, accuracy);
        filter.predict(0.05);
    }

    auto state = filter.get_state();
    EXPECT_NEAR(state.latitude, lat, 0.001);
}

TEST_F(KalmanFilterNDTest, ResetState) {
    filter.update(55.7558, 37.6173, 100.0, 10.0);
    filter.reset();

    auto state = filter.get_state();
    EXPECT_DOUBLE_EQ(state.latitude, 0.0);
    EXPECT_DOUBLE_EQ(state.longitude, 0.0);
}

TEST_F(KalmanFilterNDTest, ErrorTracking) {
    filter.update(55.0, 37.0, 100.0, 5.0);

    double lat_error = filter.get_latitude_error();
    double lon_error = filter.get_longitude_error();
    double alt_error = filter.get_altitude_error();

    EXPECT_GT(lat_error, 0.0);
    EXPECT_GT(lon_error, 0.0);
    EXPECT_GT(alt_error, 0.0);
}

// ============================================================================
// Outlier Rejection Tests (S2-3)
// ============================================================================

TEST_F(KalmanFilterNDTest, OutlierRejectionSpeedSpike) {
    // Simulate normal tracking at ~36 km/h
    filter.update(55.0, 37.0, 100.0, 5.0);

    for (int i = 0; i < 10; ++i) {
        filter.predict(1.0);
        filter.update(55.0 + static_cast<double>(i) * 0.0001,
                      37.0 + static_cast<double>(i) * 0.0001,
                      100.0, 5.0);
    }

    auto state_before = filter.get_state();

    // Inject a 100 km/h spike (≈ 0.00025 degrees per second at this latitude)
    filter.predict(1.0);
    filter.update(55.01, 37.01, 100.0, 5.0);  // Large jump

    auto state_after = filter.get_state();

    // Filter should reject most of the spike, not jump to the outlier position
    double lat_diff = state_after.latitude - state_before.latitude;
    EXPECT_LT(lat_diff, 0.005);  // Less than 0.005 degree per second
}

TEST_F(KalmanFilterNDTest, OutlierRejectionAccuracyDegradation) {
    // Track with 5m accuracy
    filter.update(55.7558, 37.6173, 100.0, 5.0);
    filter.predict(1.0);
    filter.update(55.7559, 37.6174, 100.0, 5.0);

    auto state_before = filter.get_state();

    // Sudden accuracy drop to 500m — filter should mostly reject this measurement
    filter.predict(1.0);
    filter.update(55.8, 37.7, 100.0, 500.0);

    auto state_after = filter.get_state();

    // With very poor accuracy (weight ~0.2), filter relies on its estimate
    // State should remain close to previous, not jump to outlier
    EXPECT_NEAR(state_after.latitude, state_before.latitude, 0.002);
    EXPECT_NEAR(state_after.longitude, state_before.longitude, 0.002);
}

TEST_F(KalmanFilterNDTest, OutlierRejectionConsecutiveSpikes) {
    filter.update(55.0, 37.0, 100.0, 5.0);

    // Normal tracking builds up confidence
    for (int i = 0; i < 5; ++i) {
        filter.predict(0.1);
        filter.update(55.0 + static_cast<double>(i) * 0.00001,
                      37.0, 100.0, 5.0);
    }

    auto initial_state = filter.get_state();

    // Inject 3 consecutive outlier points
    for (int i = 0; i < 3; ++i) {
        filter.predict(0.1);
        filter.update(55.0 + static_cast<double>(i) * 0.01,
                      38.0, 100.0, 5.0);  // Jump to different longitude
    }

    auto final_state = filter.get_state();

    // Longitude should not fully follow 3 consecutive outlier points
    EXPECT_NEAR(final_state.longitude, initial_state.longitude, 0.5);
}

// ============================================================================
// Jitter Smoothing Tests (S2-4)
// ============================================================================

TEST_F(KalmanFilterNDTest, JitterSmoothingZeroMeanNoise) {
    double base_lat = 55.7558;
    double base_lon = 37.6173;
    filter.update(base_lat, base_lon, 100.0, 5.0);

    // Apply zero-mean jitter (± 0.0001 degrees ≈ ±11m)
    double noise_pattern[] = {0.0001, -0.0001, 0.00008, -0.00009, 0.00011,
                              -0.0001, 0.00007, -0.00011, 0.00009, -0.00008};

    for (double noise : noise_pattern) {
        filter.predict(0.1);
        filter.update(base_lat + noise, base_lon + noise, 100.0, 5.0);
    }

    auto state = filter.get_state();
    // With zero-mean noise, filter should stay near base values
    EXPECT_NEAR(state.latitude, base_lat, 0.0001);
    EXPECT_NEAR(state.longitude, base_lon, 0.0001);
}

TEST_F(KalmanFilterNDTest, JitterSmoothingGPSNoisePattern) {
    double lat = 59.9311;   // SPB
    double lon = 30.3609;

    filter.update(lat, lon, 100.0, 5.0);

    // Simulate realistic GPS jitter over 50 updates
    double max_deviation = 0.0;
    for (int i = 0; i < 50; ++i) {
        // Generate pseudo-random jitter based on iteration
        double jitter_lat = ((i % 7) - 3) * 0.00002;
        double jitter_lon = ((i % 5) - 2) * 0.00002;

        filter.predict(0.1);
        filter.update(lat + jitter_lat, lon + jitter_lon, 100.0, 5.0);

        auto state = filter.get_state();
        double dev_lat = std::abs(state.latitude - lat);
        if (dev_lat > max_deviation) {
            max_deviation = dev_lat;
        }
    }

    auto state = filter.get_state();
    // After convergence, filter should stay within bounds
    EXPECT_NEAR(state.latitude, lat, 0.0001);
    EXPECT_NEAR(state.longitude, lon, 0.0001);

    // Maximum deviation during jitter should be less than raw jitter amplitude
    EXPECT_LT(max_deviation, 0.00015);
}

TEST_F(KalmanFilterNDTest, JitterVsOutlierDistinction) {
    double lat = 55.0;
    double lon = 37.0;
    filter.update(lat, lon, 100.0, 5.0);

    // Jitter: small deviations (±3m)
    for (int i = 0; i < 5; ++i) {
        filter.predict(0.1);
        filter.update(lat + ((i % 2 == 0) ? 0.00003 : -0.00003),
                      lon, 100.0, 5.0);
    }
    auto state_after_jitter = filter.get_state();
    double jitter_response = std::abs(state_after_jitter.latitude - lat);

    // Reset
    filter.reset();
    filter.update(lat, lon, 100.0, 5.0);

    // Outlier: large deviation
    filter.predict(0.1);
    filter.update(lat + 0.01, lon, 100.0, 5.0);  // 1 km jump
    auto state_after_outlier = filter.get_state();
    double outlier_response = std::abs(state_after_outlier.latitude - lat);

    // Jitter response should be significantly smaller than outlier response
    EXPECT_LT(jitter_response, outlier_response);
}
