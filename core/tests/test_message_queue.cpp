// SPDX-License-Identifier: Apache-2.0
// MessageQueue tests: enqueue/dequeue, retry, TTL, overflow

#include <gtest/gtest.h>
#include <string>

#include "umap/data/message_queue.h"

using namespace umap::data;

class MessageQueueTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};

    void SetUp() override {
        queue.reset();
    }
};

TEST_F(MessageQueueTest, EnqueueSingleMessage) {
    auto result = queue.enqueue("{\"test\":1}");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(queue.total_count(), 1U);
    EXPECT_EQ(queue.pending_count(), 1U);
}

TEST_F(MessageQueueTest, EnqueueNullPayload) {
    auto result = queue.enqueue(nullptr);
    EXPECT_FALSE(result.has_value());
}

TEST_F(MessageQueueTest, DequeueEmptyQueue) {
    auto result = queue.dequeue();
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::NotFound);
}

TEST_F(MessageQueueTest, DequeueReturnsOldest) {
    queue.enqueue("{\"seq\":1}");
    queue.enqueue("{\"seq\":2}");
    queue.enqueue("{\"seq\":3}");

    auto msg1 = queue.dequeue();
    ASSERT_TRUE(msg1.has_value());
    EXPECT_EQ(msg1.value().payload, "{\"seq\":1}");

    auto msg2 = queue.dequeue();
    ASSERT_TRUE(msg2.has_value());
    EXPECT_EQ(msg2.value().payload, "{\"seq\":2}");
}

TEST_F(MessageQueueTest, MarkSentRemovesMessage) {
    queue.enqueue("{\"test\":1}");
    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());

    auto result = queue.mark_sent(msg.value().id);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(queue.total_count(), 0U);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(MessageQueueTest, MarkFailedIncrementsRetry) {
    queue.enqueue("{\"test\":1}");
    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());

    auto result = queue.mark_failed(msg.value().id);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(queue.total_count(), 1U);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(MessageQueueTest, DequeueSkipsNotReadyMessages) {
    queue.enqueue("{\"test\":1}");
    auto msg1 = queue.dequeue();
    ASSERT_TRUE(msg1.has_value());

    queue.mark_failed(msg1.value().id);

    // Message should not be available immediately (backoff)
    auto result = queue.dequeue();
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(MessageQueueTest, TotalCountIncludesAllMessages) {
    queue.enqueue("a");
    queue.enqueue("b");
    queue.enqueue("c");

    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());
    queue.mark_failed(msg.value().id);

    // 3 total: 2 ready + 1 waiting retry
    EXPECT_EQ(queue.total_count(), 3U);
    EXPECT_EQ(queue.pending_count(), 2U);
}

TEST_F(MessageQueueTest, TtlCleanupRemovesOldMessages) {
    queue.enqueue("{\"old\":1}");
    EXPECT_EQ(queue.total_count(), 1U);

    // Purge with 0 hours TTL — removes everything
    uint32_t deleted = queue.purge_expired(0U);
    EXPECT_GE(deleted, 1U);
    EXPECT_EQ(queue.total_count(), 0U);
}

TEST_F(MessageQueueTest, TtlCleanupDoesNotRemoveNewMessages) {
    queue.enqueue("{\"new\":1}");
    EXPECT_EQ(queue.total_count(), 1U);

    // 168 hours (7 days) — should not delete just-created messages
    uint32_t deleted = queue.purge_expired(168U);
    EXPECT_EQ(deleted, 0U);
    EXPECT_EQ(queue.total_count(), 1U);
}

TEST_F(MessageQueueTest, ResetClearsAllMessages) {
    queue.enqueue("a");
    queue.enqueue("b");
    queue.enqueue("c");
    EXPECT_EQ(queue.total_count(), 3U);

    queue.reset();
    EXPECT_EQ(queue.total_count(), 0U);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(MessageQueueTest, EnqueueDequeueManyMessages) {
    constexpr int COUNT = 500;

    for (int i = 0; i < COUNT; ++i) {
        std::string payload = "{\"seq\":" + std::to_string(i) + "}";
        auto result = queue.enqueue(payload.c_str());
        EXPECT_TRUE(result.has_value());
    }

    EXPECT_EQ(queue.total_count(), static_cast<uint32_t>(COUNT));
    EXPECT_EQ(queue.pending_count(), static_cast<uint32_t>(COUNT));

    int dequeued = 0;
    while (true) {
        auto msg = queue.dequeue();
        if (!msg.has_value()) {
            break;
        }
        queue.mark_sent(msg.value().id);
        ++dequeued;
    }

    EXPECT_EQ(dequeued, COUNT);
    EXPECT_EQ(queue.total_count(), 0U);
}

TEST_F(MessageQueueTest, RetryCountTracking) {
    queue.enqueue("{\"test\":1}");
    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());
    uint64_t msg_id = msg.value().id;

    auto retry0 = queue.get_retry_count(msg_id);
    ASSERT_TRUE(retry0.has_value());
    EXPECT_EQ(retry0.value(), 0U);

    queue.mark_failed(msg_id);
    auto retry1 = queue.get_retry_count(msg_id);
    ASSERT_TRUE(retry1.has_value());
    EXPECT_EQ(retry1.value(), 1U);

    queue.mark_failed(msg_id);
    auto retry2 = queue.get_retry_count(msg_id);
    ASSERT_TRUE(retry2.has_value());
    EXPECT_EQ(retry2.value(), 2U);
}
