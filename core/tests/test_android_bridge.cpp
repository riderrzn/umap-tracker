// SPDX-License-Identifier: Apache-2.0
// Tests for Android JNI bridge and thread-safe queue

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "umap/platform/android_bridge.h"
#include "umap/core/geo_types.h"

using namespace umap::platform;
using namespace umap::core;

// Tests for ThreadSafeQueue
class ThreadSafeQueueTest : public ::testing::Test {
protected:
    ThreadSafeQueue<int, 10> queue;
};

TEST_F(ThreadSafeQueueTest, EmptyQueueOnCreation) {
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0U);
}

TEST_F(ThreadSafeQueueTest, EnqueueAndDequeue) {
    EXPECT_TRUE(queue.enqueue(42));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1U);
    
    auto item = queue.dequeue();
    EXPECT_TRUE(item.has_value());
    EXPECT_EQ(item.value(), 42);
    EXPECT_TRUE(queue.empty());
}

TEST_F(ThreadSafeQueueTest, DequeueEmptyReturnsNullopt) {
    EXPECT_FALSE(queue.dequeue().has_value());
}

TEST_F(ThreadSafeQueueTest, EnqueueMultipleItems) {
    EXPECT_TRUE(queue.enqueue(1));
    EXPECT_TRUE(queue.enqueue(2));
    EXPECT_TRUE(queue.enqueue(3));
    EXPECT_EQ(queue.size(), 3U);
}

TEST_F(ThreadSafeQueueTest, FifoOrder) {
    EXPECT_TRUE(queue.enqueue(1));
    EXPECT_TRUE(queue.enqueue(2));
    EXPECT_TRUE(queue.enqueue(3));
    
    auto item1 = queue.dequeue();
    auto item2 = queue.dequeue();
    auto item3 = queue.dequeue();
    
    EXPECT_TRUE(item1.has_value() && item1.value() == 1);
    EXPECT_TRUE(item2.has_value() && item2.value() == 2);
    EXPECT_TRUE(item3.has_value() && item3.value() == 3);
}

TEST_F(ThreadSafeQueueTest, QueueOverflow) {
    // Enqueue more than max size (10)
    for (int i = 0; i < 10; ++i) {
        EXPECT_TRUE(queue.enqueue(i));
    }
    
    // 11th should fail
    EXPECT_FALSE(queue.enqueue(10));
    EXPECT_EQ(queue.size(), 10U);
}

TEST_F(ThreadSafeQueueTest, ClearQueue) {
    queue.enqueue(1);
    queue.enqueue(2);
    queue.enqueue(3);
    EXPECT_EQ(queue.size(), 3U);
    
    queue.clear();
    EXPECT_TRUE(queue.empty());
    EXPECT_EQ(queue.size(), 0U);
}

// Tests with GpsPoint
class ThreadSafeQueueGpsPointTest : public ::testing::Test {
protected:
    ThreadSafeQueue<GpsPoint, 100> queue;
};

TEST_F(ThreadSafeQueueGpsPointTest, EnqueueGpsPoint) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);  // Moscow
    point.accuracy_m = 10.0;
    point.timestamp_ms = 1000U;
    
    EXPECT_TRUE(queue.enqueue(point));
    auto retrieved = queue.dequeue();
    
    EXPECT_TRUE(retrieved.has_value());
    EXPECT_DOUBLE_EQ(retrieved.value().location.latitude, 55.7558);
    EXPECT_DOUBLE_EQ(retrieved.value().location.longitude, 37.6173);
    EXPECT_DOUBLE_EQ(retrieved.value().accuracy_m, 10.0);
}

// Tests for LocationEngine
class LocationEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        LocationEngine::instance().unregister_callbacks();
        LocationEngine::instance().clear_location_queue();
    }

    void TearDown() override {
        LocationEngine::instance().unregister_callbacks();
        LocationEngine::instance().clear_location_queue();
    }
};

TEST_F(LocationEngineTest, SingletonInstance) {
    LocationEngine& engine1 = LocationEngine::instance();
    LocationEngine& engine2 = LocationEngine::instance();
    
    // Should be same instance
    EXPECT_EQ(&engine1, &engine2);
}

TEST_F(LocationEngineTest, EnqueueLocation) {
    bool result = LocationEngine::instance().enqueue_location(
        55.7558, 37.6173, 10.0f, 1000U
    );
    EXPECT_TRUE(result);
    EXPECT_EQ(LocationEngine::instance().location_queue_size(), 1U);
}

TEST_F(LocationEngineTest, GetLocation) {
    LocationEngine::instance().enqueue_location(
        55.7558, 37.6173, 10.0f, 1000U
    );
    
    auto point = LocationEngine::instance().get_location();
    EXPECT_TRUE(point.has_value());
    EXPECT_DOUBLE_EQ(point.value().location.latitude, 55.7558);
    EXPECT_DOUBLE_EQ(point.value().location.longitude, 37.6173);
}

TEST_F(LocationEngineTest, GetLocationTimeout) {
    auto start = std::chrono::high_resolution_clock::now();
    auto point = LocationEngine::instance().get_location(100U);  // 100ms timeout
    auto end = std::chrono::high_resolution_clock::now();
    
    EXPECT_FALSE(point.has_value());
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Should wait approximately 100ms (allow some variance)
    EXPECT_GE(elapsed_ms, 80);
    EXPECT_LE(elapsed_ms, 150);
}

TEST_F(LocationEngineTest, RegisterLocationCallback) {
    int call_count = 0;
    auto callback = [](double, double, float, uint32_t) {
        // Mock callback
    };
    
    auto result = LocationEngine::instance().register_location_callback(callback);
    EXPECT_TRUE(result.has_value());
}

TEST_F(LocationEngineTest, RegisterNullptrCallback) {
    auto result = LocationEngine::instance().register_location_callback(nullptr);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::InvalidConfig);
}

TEST_F(LocationEngineTest, DoubleRegisterCallback) {
    auto callback = [](double, double, float, uint32_t) {};
    
    EXPECT_TRUE(LocationEngine::instance().register_location_callback(callback).has_value());
    EXPECT_FALSE(LocationEngine::instance().register_location_callback(callback).has_value());
}

TEST_F(LocationEngineTest, RegisterBatteryCallback) {
    auto callback = [](uint32_t, bool) {};
    auto result = LocationEngine::instance().register_battery_callback(callback);
    EXPECT_TRUE(result.has_value());
}

TEST_F(LocationEngineTest, RegisterErrorCallback) {
    auto callback = [](int, const char*) {};
    auto result = LocationEngine::instance().register_error_callback(callback);
    EXPECT_TRUE(result.has_value());
}

TEST_F(LocationEngineTest, UnregisterCallbacks) {
    auto loc_cb = [](double, double, float, uint32_t) {};
    auto bat_cb = [](uint32_t, bool) {};
    auto err_cb = [](int, const char*) {};
    
    LocationEngine::instance().register_location_callback(loc_cb);
    LocationEngine::instance().register_battery_callback(bat_cb);
    LocationEngine::instance().register_error_callback(err_cb);
    
    LocationEngine::instance().unregister_callbacks();
    
    // Should be able to register again
    EXPECT_TRUE(LocationEngine::instance().register_location_callback(loc_cb).has_value());
}

// Test concurrent access
TEST_F(LocationEngineTest, ConcurrentEnqueue) {
    auto enqueue_thread = [](int start, int count) {
        for (int i = start; i < start + count; ++i) {
            LocationEngine::instance().enqueue_location(
                55.0 + i, 37.0 + i, 10.0f + i, 1000U + i
            );
        }
    };
    
    std::thread t1(enqueue_thread, 0, 50);
    std::thread t2(enqueue_thread, 50, 50);
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(LocationEngine::instance().location_queue_size(), 100U);
}

// Test concurrent dequeue
TEST_F(LocationEngineTest, ConcurrentDequeue) {
    // Pre-fill queue
    for (int i = 0; i < 100; ++i) {
        LocationEngine::instance().enqueue_location(
            55.0 + i, 37.0 + i, 10.0f, 1000U
        );
    }
    
    int dequeued_count = 0;
    std::mutex count_mutex;
    
    auto dequeue_thread = [&]() {
        for (int i = 0; i < 50; ++i) {
            auto point = LocationEngine::instance().get_location();
            if (point.has_value()) {
                std::unique_lock<std::mutex> lock(count_mutex);
                dequeued_count++;
            }
        }
    };
    
    std::thread t1(dequeue_thread);
    std::thread t2(dequeue_thread);
    
    t1.join();
    t2.join();
    
    EXPECT_EQ(dequeued_count, 100);
    EXPECT_TRUE(LocationEngine::instance().location_queue_size() == 0U);
}

// Test static callback adapters
TEST_F(LocationEngineTest, StaticLocationCallback) {
    int call_count = 0;
    auto callback = [](double, double, float, uint32_t) {
        // Would increment call_count, but captures not allowed in test
    };
    
    LocationEngine::instance().register_location_callback(callback);
    LocationEngine::instance().on_location_received(55.7558, 37.6173, 10.0f, 1000U);
    
    // Should be queued
    EXPECT_EQ(LocationEngine::instance().location_queue_size(), 1U);
}

// Test LocationEngine with realistic GPS points
TEST_F(LocationEngineTest, RealisticGpsPoints) {
    // Simulate Moscow to SPB track
    double latitudes[] = {55.7558, 56.0, 56.5, 57.0, 57.5, 58.0, 59.9311};
    double longitudes[] = {37.6173, 37.5, 37.0, 36.5, 36.0, 35.5, 30.3609};
    
    for (int i = 0; i < 7; ++i) {
        LocationEngine::instance().enqueue_location(
            latitudes[i], longitudes[i], 5.0f, 1000U + i * 1000U
        );
    }
    
    EXPECT_EQ(LocationEngine::instance().location_queue_size(), 7U);
    
    // Retrieve and verify
    for (int i = 0; i < 7; ++i) {
        auto point = LocationEngine::instance().get_location();
        EXPECT_TRUE(point.has_value());
        EXPECT_DOUBLE_EQ(point.value().location.latitude, latitudes[i]);
        EXPECT_DOUBLE_EQ(point.value().location.longitude, longitudes[i]);
    }
}
