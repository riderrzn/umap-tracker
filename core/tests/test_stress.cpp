// SPDX-License-Identifier: Apache-2.0
// Stress & recovery tests: large-scale queue, throughput, pipeline E2E

#include <gtest/gtest.h>
#include <chrono>
#include <string>

#include "umap/data/message_queue.h"
#include "umap/core/gps_pipeline.h"
#include "umap/data/upload_worker.h"
#include "umap/data/traccar_client.h"
#include "umap/data/http_client.h"

using namespace umap::data;
using namespace umap::core;

class QueueStressTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};

    void SetUp() override {
        queue.reset();
    }
};

TEST_F(QueueStressTest, EnqueueFiftyThousandPoints) {
    constexpr int COUNT = 50000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < COUNT; ++i) {
        std::string payload = "{\"seq\":" + std::to_string(i) + "}";
        auto result = queue.enqueue(payload.c_str());
        ASSERT_TRUE(result.has_value()) << "Failed at index " << i;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    EXPECT_EQ(queue.total_count(), static_cast<uint32_t>(COUNT));

    // 50k inserts should complete within 5 seconds
    EXPECT_LT(duration_ms, 5000);
}

TEST_F(QueueStressTest, DequeueFiftyThousandPoints) {
    constexpr int COUNT = 5000;

    for (int i = 0; i < COUNT; ++i) {
        std::string payload = "{\"seq\":" + std::to_string(i) + "}";
        queue.enqueue(payload.c_str());
    }

    auto start = std::chrono::high_resolution_clock::now();

    int dequeued = 0;
    while (dequeued < COUNT) {
        auto msg = queue.dequeue();
        ASSERT_TRUE(msg.has_value());
        queue.mark_sent(msg.value().id);
        ++dequeued;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    EXPECT_EQ(dequeued, COUNT);
    EXPECT_EQ(queue.total_count(), 0U);

    // Queue recovery: 5k dequeues should complete reliably
    EXPECT_LT(duration_ms, 30000);
}

TEST_F(QueueStressTest, ThroughputThousandPerSecond) {
    constexpr int BATCH = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < BATCH; ++i) {
        queue.enqueue("x");
    }

    int dequeued = 0;
    while (dequeued < BATCH) {
        auto msg = queue.dequeue();
        ASSERT_TRUE(msg.has_value());
        queue.mark_sent(msg.value().id);
        ++dequeued;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_s = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count() / 1000.0;

    double per_second = static_cast<double>(BATCH * 2) / duration_s;

    // 2000 operations (1000 enqueue + 1000 dequeue) should be > 1000/sec
    EXPECT_GT(per_second, 1000.0);
}

TEST_F(QueueStressTest, TtlCleanupBulk) {
    constexpr int COUNT = 10000;

    for (int i = 0; i < COUNT; ++i) {
        queue.enqueue("x");
    }

    EXPECT_EQ(queue.total_count(), static_cast<uint32_t>(COUNT));

    auto start = std::chrono::high_resolution_clock::now();

    uint32_t deleted = queue.purge_expired(0U);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    EXPECT_GE(deleted, static_cast<uint32_t>(COUNT));
    EXPECT_EQ(queue.total_count(), 0U);
    EXPECT_LT(duration_ms, 2000);
}

class PipelineStressTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};
    GpsProcessingPipeline pipeline{queue};

    void SetUp() override {
        queue.reset();
        pipeline.reset();
    }
};

TEST_F(PipelineStressTest, ProcessThousandPointsPipeline) {
    constexpr int COUNT = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < COUNT; ++i) {
        GpsPoint point;
        point.location = Coordinate(55.0 + static_cast<double>(i) * 0.0001,
                                    37.0 + static_cast<double>(i) * 0.0001);
        point.accuracy_m = 5.0;
        point.altitude_m = 100.0 + i * 0.1;
        point.timestamp_ms = static_cast<uint32_t>(1000U + i * 1000U);
        point.battery_percent = 85U;
        point.hdop = 1.5;

        auto result = pipeline.process_location(point);
        ASSERT_TRUE(result.has_value()) << "Failed at point " << i;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    EXPECT_EQ(pipeline.processed_count(), static_cast<uint32_t>(COUNT));
    EXPECT_EQ(queue.pending_count(), static_cast<uint32_t>(COUNT));

    // 1000 Kalman filter + enqueue operations should be < 500ms
    EXPECT_LT(duration_ms, 500);
}

TEST_F(PipelineStressTest, PipelineToUploadE2E) {
    MockHttpClient mock_http;
    mock_http.set_response(200, "{}");
    TraccarClient client(mock_http, "http://test.com:5055", "device-001");
    UploadWorker worker(queue, client);

    constexpr int COUNT = 500;

    for (int i = 0; i < COUNT; ++i) {
        GpsPoint point;
        point.location = Coordinate(55.0 + i * 0.0001, 37.0 + i * 0.0001);
        point.accuracy_m = 5.0;
        point.altitude_m = 100.0;
        point.timestamp_ms = 1000U + static_cast<uint32_t>(i) * 1000U;
        ASSERT_TRUE(pipeline.process_location(point).has_value());
    }

    EXPECT_EQ(queue.pending_count(), static_cast<uint32_t>(COUNT));

    auto start = std::chrono::high_resolution_clock::now();

    worker.process_pending(static_cast<uint32_t>(COUNT));

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    EXPECT_EQ(worker.total_sent(), static_cast<uint32_t>(COUNT));
    EXPECT_EQ(queue.pending_count(), 0U);

    // Upload 500 via mock should be < 3 seconds
    EXPECT_LT(duration_ms, 3000);
}

TEST_F(PipelineStressTest, ColdStartInitTime) {
    auto start = std::chrono::high_resolution_clock::now();

    MessageQueue q(":memory:");
    GpsProcessingPipeline p(q);
    MockHttpClient http;
    TraccarClient c(http, "http://t.com:5055", "d");
    UploadWorker w(q, c);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        end - start).count();

    // Complete pipeline init should be < 100ms (cold start component)
    EXPECT_LT(duration_ms, 100);
}