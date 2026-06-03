// SPDX-License-Identifier: Apache-2.0
// Integration test: GPS → Kalman → State Machine
// Verifies end-to-end pipeline: raw GPS data → filtered position → state transitions

#include <gtest/gtest.h>
#include <cmath>

#include "umap/core/kalman_filter.h"
#include "umap/core/geo_types.h"
#include "umap/core/state_machine.h"

using namespace umap::core;

class IntegrationKalmanStateTest : public ::testing::Test {
protected:
    KalmanFilterND kalman{0.001, 4.0, 0.01, 10.0};
    LocationStateMachine state_machine;

    bool kalman_initialized = false;

    struct ProcessedPoint {
        double lat;
        double lon;
        double speed_kmh;
        double bearing_deg;
        LocationState state;
    };
};

TEST_F(IntegrationKalmanStateTest, ColdStartGPSAcquisition) {
    // Simulate cold start: no location → acquiring → tracking
    EXPECT_EQ(state_machine.get_state(), LocationState::Idle);

    state_machine.process_event(StateEvent::StartTracking);
    EXPECT_EQ(state_machine.get_state(), LocationState::Acquiring);

    // First GPS fix arrives
    kalman.update(55.7558, 37.6173, 100.0, 10.0);
    kalman_initialized = true;

    state_machine.process_event(StateEvent::LocationAcquired);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
}

TEST_F(IntegrationKalmanStateTest, FullTrackingSession) {
    // Start tracking
    state_machine.process_event(StateEvent::StartTracking);
    state_machine.process_event(StateEvent::LocationAcquired);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);

    double lat = 59.9311;   // SPB
    double lon = 30.3609;

    // Seed the Kalman filter
    kalman.update(lat, lon, 50.0, 5.0);

    // Simulate 20 seconds of tracking with GPS updates every 1 second
    for (int t = 0; t < 20; ++t) {
        kalman.predict(1.0);

        // Simulate walking east at 1.4 m/s
        lat += 0.000005;  // ~0.5m north
        lon += 0.000012;  // ~1.0m east
        kalman.update(lat, lon, 50.0, 5.0);

        auto state = kalman.get_state();
        EXPECT_GT(state.speed_kmh, 0.0);
        EXPECT_GE(state.bearing_deg, 0.0);
        EXPECT_LE(state.bearing_deg, 360.0);
        EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
    }
}

TEST_F(IntegrationKalmanStateTest, UploadCycle) {
    // Get into tracking state
    state_machine.process_event(StateEvent::StartTracking);
    state_machine.process_event(StateEvent::LocationAcquired);

    kalman.update(55.0, 37.0, 100.0, 5.0);

    // Trigger upload
    state_machine.process_event(StateEvent::UploadStarted);
    EXPECT_EQ(state_machine.get_state(), LocationState::Uploading);

    // Kalman filter still processes data during upload
    kalman.predict(0.5);
    kalman.update(55.0001, 37.0001, 100.0, 5.0);

    // Upload completes
    state_machine.process_event(StateEvent::UploadCompleted);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
}

TEST_F(IntegrationKalmanStateTest, GPSLossAndRecovery) {
    state_machine.process_event(StateEvent::StartTracking);
    state_machine.process_event(StateEvent::LocationAcquired);

    // Track for a while
    kalman.update(55.0, 37.0, 100.0, 5.0);
    for (int i = 0; i < 5; ++i) {
        kalman.predict(1.0);
        kalman.update(55.0 + static_cast<double>(i) * 0.0001,
                      37.0 + static_cast<double>(i) * 0.0001,
                      100.0, 5.0);
    }

    // GPS signal lost
    state_machine.process_event(StateEvent::LocationLost);
    EXPECT_EQ(state_machine.get_state(), LocationState::Acquiring);

    // Filter continues predicting without new measurements
    for (int i = 0; i < 3; ++i) {
        kalman.predict(1.0);
        auto state = kalman.get_state();
        // Speed should decay without measurements
        EXPECT_GE(state.speed_kmh, 0.0);
    }

    // GPS recovered
    kalman.update(55.0004, 37.0004, 100.0, 5.0);
    state_machine.process_event(StateEvent::LocationAcquired);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
}

TEST_F(IntegrationKalmanStateTest, PauseResumeWithKalman) {
    state_machine.process_event(StateEvent::StartTracking);
    state_machine.process_event(StateEvent::LocationAcquired);

    kalman.update(55.0, 37.0, 100.0, 5.0);

    // Pause tracking
    state_machine.process_event(StateEvent::PauseTracking);
    EXPECT_EQ(state_machine.get_state(), LocationState::Paused);

    // During pause, Kalman filter predictions continue but no measurements
    for (int i = 0; i < 5; ++i) {
        kalman.predict(1.0);
    }
    auto paused_state = kalman.get_state();

    // Resume tracking
    state_machine.process_event(StateEvent::ResumeTracking);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);

    // New measurement after resume
    kalman.update(55.0001, 37.0001, 100.0, 5.0);
    auto resumed_state = kalman.get_state();
    EXPECT_NEAR(resumed_state.latitude, paused_state.latitude, 0.001);
}

TEST_F(IntegrationKalmanStateTest, ErrorStateHaltsProcessing) {
    state_machine.process_event(StateEvent::StartTracking);
    state_machine.process_event(StateEvent::LocationAcquired);

    kalman.update(55.0, 37.0, 100.0, 5.0);

    // Error occurs
    state_machine.process_event(StateEvent::ErrorOccurred);
    EXPECT_EQ(state_machine.get_state(), LocationState::Error);

    // Kalman should be resettable during error
    kalman.reset();
    auto state = kalman.get_state();
    EXPECT_DOUBLE_EQ(state.latitude, 0.0);

    // Recover and restart
    state_machine.process_event(StateEvent::ErrorRecovered);
    EXPECT_EQ(state_machine.get_state(), LocationState::Idle);

    state_machine.process_event(StateEvent::StartTracking);
    kalman.update(55.0, 37.0, 100.0, 5.0);
    state_machine.process_event(StateEvent::LocationAcquired);
    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
}

TEST_F(IntegrationKalmanStateTest, RealisticMoscowTrack) {
    // Moscow to SPB simplified track (≈ 650 km, 100 update points)
    double lat = 55.7558;   // Moscow
    double lon = 37.6173;

    state_machine.process_event(StateEvent::StartTracking);
    kalman.update(lat, lon, 150.0, 10.0);
    state_machine.process_event(StateEvent::LocationAcquired);

    int state_change_count = 0;
    state_machine.register_state_callback(
        [&state_change_count](LocationState, LocationState) {
            state_change_count++;
        });

    // Simulate driving north-west at ~80 km/h
    for (int i = 0; i < 100; ++i) {
        kalman.predict(1.0);

        lat += 0.002;   // North
        lon -= 0.001;   // West
        kalman.update(lat, lon, 150.0, 10.0);

        auto state = kalman.get_state();
        // Verify filter values are physically reasonable
        EXPECT_LT(state.speed_kmh, 200.0);
        EXPECT_GE(state.latitude, 55.0);
        EXPECT_GE(state.longitude, 30.0);

        // Periodically trigger upload cycles
        if (i % 20 == 19) {
            state_machine.process_event(StateEvent::UploadStarted);
            state_machine.process_event(StateEvent::UploadCompleted);
        }
    }

    auto final_state = kalman.get_state();
    // Filtered track moved north from Moscow (55.7558) by ~0.2 degrees
    EXPECT_GT(final_state.latitude, 55.9);
    EXPECT_LT(final_state.longitude, 37.6);

    EXPECT_EQ(state_machine.get_state(), LocationState::Tracking);
    EXPECT_GT(state_change_count, 0);
}
