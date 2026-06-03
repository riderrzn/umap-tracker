// SPDX-License-Identifier: Apache-2.0
// Kalman Filter benchmark: verifies < 10ms per update

#include <gtest/gtest.h>
#include <chrono>

#include "umap/core/kalman_filter.h"
#include "umap/core/state_machine.h"

using namespace umap::core;

class KalmanBenchmark : public ::testing::Test {
protected:
    KalmanFilterND filter{0.001, 4.0, 0.01, 10.0};

    static constexpr int BENCHMARK_ITERATIONS = 10000;
};

TEST_F(KalmanBenchmark, UpdateLatencyUnder10ms) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        double lat = 55.0 + static_cast<double>(i % 100) * 0.001;
        double lon = 37.0 + static_cast<double>(i % 100) * 0.001;
        filter.update(lat, lon, 100.0, 5.0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start).count();

    double avg_us = static_cast<double>(duration_us) / BENCHMARK_ITERATIONS;

    // Average update time must be under 10ms (10000us)
    EXPECT_LT(avg_us, 10000.0);
}

TEST_F(KalmanBenchmark, PredictUpdateCycleLatency) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        filter.predict(0.1);
        double lat = 55.0 + static_cast<double>(i % 100) * 0.001;
        double lon = 37.0 + static_cast<double>(i % 100) * 0.001;
        filter.update(lat, lon, 100.0, 5.0);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start).count();

    double avg_us = static_cast<double>(duration_us) / BENCHMARK_ITERATIONS;

    // Average predict+update cycle must be under 10ms (10000us)
    EXPECT_LT(avg_us, 10000.0);
}

TEST_F(KalmanBenchmark, ThroughputUpdatesPerSecond) {
    KalmanFilter1D filter_1d{0.01, 4.0};

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        filter_1d.predict(0.1);
        filter_1d.update(static_cast<double>(i % 100));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count() / 1000.0;

    double updates_per_sec = BENCHMARK_ITERATIONS / duration_s;

    // Minimum 1000 updates/second for 1D filter
    EXPECT_GT(updates_per_sec, 1000.0);
}

TEST_F(KalmanBenchmark, NDThroughputUpdatesPerSecond) {
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        filter.predict(0.1);
        double lat = 55.0 + static_cast<double>(i % 100) * 0.001;
        double lon = 37.0 + static_cast<double>(i % 100) * 0.001;
        filter.update(lat, lon, 100.0, 5.0);
        (void)filter.get_state();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count() / 1000.0;

    double cycles_per_sec = BENCHMARK_ITERATIONS / duration_s;

    // Minimum 500 complete cycles/second for 3D filter
    EXPECT_GT(cycles_per_sec, 500.0);
}

TEST_F(KalmanBenchmark, StateMachineTransitionLatency) {
    LocationStateMachine sm;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BENCHMARK_ITERATIONS; ++i) {
        sm.reset();
        sm.process_event(StateEvent::StartTracking);
        sm.process_event(StateEvent::LocationAcquired);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(
        end - start).count();

    double avg_us = static_cast<double>(duration_us) / (BENCHMARK_ITERATIONS * 3);

    // State machine transition under 1ms
    EXPECT_LT(avg_us, 1000.0);
}
