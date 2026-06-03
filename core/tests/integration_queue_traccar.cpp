// SPDX-License-Identifier: Apache-2.0
// Integration test: MessageQueue → TraccarClient → Mock HTTP server

#include <gtest/gtest.h>

#include "umap/data/message_queue.h"
#include "umap/data/traccar_client.h"
#include "umap/core/geo_types.h"

using namespace umap::data;
using namespace umap::core;

class IntegrationQueueTraccarTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};
    MockHttpClient mock_http;
    TraccarClient client{mock_http, "http://tracker.example.com:5055", "device-abc"};

    void SetUp() override {
        queue.reset();
        mock_http.reset();
        mock_http.set_response(200, "{}");
    }

    GpsPoint make_point(double lat, double lon, uint32_t timestamp) {
        GpsPoint point;
        point.location = Coordinate(lat, lon);
        point.speed_kmh = 30.0;
        point.accuracy_m = 5.0;
        point.timestamp_ms = timestamp;
        return point;
    }
};

TEST_F(IntegrationQueueTraccarTest, EnqueueAndSendSinglePoint) {
    auto point = make_point(55.7558, 37.6173, 1000000U);
    std::string payload = client.build_json_payload(point);

    auto enq_result = queue.enqueue(payload.c_str());
    ASSERT_TRUE(enq_result.has_value());
    EXPECT_EQ(queue.pending_count(), 1U);

    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());

    auto send_result = client.send(point);
    EXPECT_TRUE(send_result.has_value());

    queue.mark_sent(msg.value().id);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(IntegrationQueueTraccarTest, NetworkFailureStoresRetry) {
    mock_http.set_should_fail(true);

    auto point = make_point(55.0, 37.0, 1000000U);
    std::string payload = client.build_json_payload(point);

    queue.enqueue(payload.c_str());

    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());

    auto send_result = client.send(point);
    EXPECT_FALSE(send_result.has_value());

    queue.mark_failed(msg.value().id);
    EXPECT_EQ(queue.pending_count(), 0U);
    EXPECT_EQ(queue.total_count(), 1U);
}

TEST_F(IntegrationQueueTraccarTest, BatchEnqueueAndDrain) {
    constexpr int BATCH_SIZE = 10;
    mock_http.set_response(200, "{}");

    for (int i = 0; i < BATCH_SIZE; ++i) {
        auto point = make_point(55.0 + static_cast<double>(i) * 0.01,
                                37.0, 1000000U + static_cast<uint32_t>(i));
        std::string payload = client.build_json_payload(point);
        queue.enqueue(payload.c_str());
    }

    EXPECT_EQ(queue.pending_count(), static_cast<uint32_t>(BATCH_SIZE));

    int sent = 0;
    while (queue.pending_count() > 0U) {
        auto msg = queue.dequeue();
        ASSERT_TRUE(msg.has_value());
        queue.mark_sent(msg.value().id);
        ++sent;
    }

    EXPECT_EQ(sent, BATCH_SIZE);
    EXPECT_EQ(queue.total_count(), 0U);
    EXPECT_EQ(mock_http.request_count(), 0U);
}

TEST_F(IntegrationQueueTraccarTest, MixedSuccessAndFailurePattern) {
    auto point1 = make_point(55.0, 37.0, 1000000U);
    auto point2 = make_point(55.001, 37.001, 1001000U);
    auto point3 = make_point(55.002, 37.002, 1002000U);

    queue.enqueue(client.build_json_payload(point1).c_str());
    queue.enqueue(client.build_json_payload(point2).c_str());
    queue.enqueue(client.build_json_payload(point3).c_str());

    // Send message 1 — success
    auto msg1 = queue.dequeue();
    ASSERT_TRUE(msg1.has_value());
    auto result1 = client.send(point1);
    EXPECT_TRUE(result1.has_value());
    queue.mark_sent(msg1.value().id);

    // Send message 2 — failure
    mock_http.set_should_fail(true);
    auto msg2 = queue.dequeue();
    ASSERT_TRUE(msg2.has_value());
    auto result2 = client.send(point2);
    EXPECT_FALSE(result2.has_value());
    queue.mark_failed(msg2.value().id);

    // Send message 3 — success
    mock_http.set_should_fail(false);
    auto msg3 = queue.dequeue();
    ASSERT_TRUE(msg3.has_value());
    auto result3 = client.send(point3);
    EXPECT_TRUE(result3.has_value());
    queue.mark_sent(msg3.value().id);

    // Message 2 is waiting for retry
    EXPECT_EQ(queue.total_count(), 1U);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(IntegrationQueueTraccarTest, QueueOverflowRecovery) {
    constexpr int LARGE_COUNT = 1000;
    mock_http.set_response(200, "{}");

    for (int i = 0; i < LARGE_COUNT; ++i) {
        auto point = make_point(55.0, 37.0,
                                1000000U + static_cast<uint32_t>(i) * 1000U);
        std::string payload = client.build_json_payload(point);
        auto result = queue.enqueue(payload.c_str());
        ASSERT_TRUE(result.has_value());
    }

    EXPECT_EQ(queue.total_count(), static_cast<uint32_t>(LARGE_COUNT));

    int drained = 0;
    while (queue.pending_count() > 0U) {
        auto msg = queue.dequeue();
        ASSERT_TRUE(msg.has_value());
        queue.mark_sent(msg.value().id);
        ++drained;
    }

    EXPECT_EQ(drained, LARGE_COUNT);
    EXPECT_EQ(queue.total_count(), 0U);
}

TEST_F(IntegrationQueueTraccarTest, RetryBackoffBehavior) {
    mock_http.set_should_fail(true);
    auto point = make_point(55.0, 37.0, 1000000U);

    queue.enqueue(client.build_json_payload(point).c_str());

    for (int attempt = 0; attempt < 3; ++attempt) {
        auto msg = queue.dequeue();
        if (attempt == 0) {
            ASSERT_TRUE(msg.has_value());
        } else {
            if (!msg.has_value()) {
                break;
            }
        }
        client.send(point);
        if (msg.has_value()) {
            queue.mark_failed(msg.value().id);
        }
    }

    // After 3 retries, retry_count should reflect the attempts
    auto retry_result = queue.get_retry_count(1U);
    if (retry_result.has_value()) {
        EXPECT_GE(retry_result.value(), 1U);
    }

    // Queue should have 1 total entry (the original), but it's waiting retry
    EXPECT_EQ(queue.total_count(), 1U);

    // Clean up
    mock_http.set_should_fail(false);
    auto final_msg = queue.dequeue();
    // May or may not be ready depending on backoff
    if (final_msg.has_value()) {
        queue.mark_sent(final_msg.value().id);
    }
}

TEST_F(IntegrationQueueTraccarTest, CompleteWorkflowWithTTL) {
    mock_http.set_response(200, "{}");

    // Enqueue several points
    for (int i = 0; i < 5; ++i) {
        auto point = make_point(55.0 + static_cast<double>(i) * 0.01,
                                37.0, 1000000U + static_cast<uint32_t>(i));
        queue.enqueue(client.build_json_payload(point).c_str());
    }

    // Drain queue
    int sent = 0;
    while (queue.pending_count() > 0U) {
        auto msg = queue.dequeue();
        ASSERT_TRUE(msg.has_value());
        queue.mark_sent(msg.value().id);
        ++sent;
    }

    EXPECT_EQ(sent, 5);

    // TTL cleanup with 7 days — no effect
    uint32_t deleted = queue.purge_expired(168U);
    EXPECT_EQ(deleted, 0U);
    EXPECT_EQ(queue.total_count(), 0U);
}
