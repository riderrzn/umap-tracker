// SPDX-License-Identifier: Apache-2.0
// Upload worker tests: MessageQueue → TraccarClient → HTTP

#include <gtest/gtest.h>

#include "umap/data/upload_worker.h"
#include "umap/data/message_queue.h"
#include "umap/data/traccar_client.h"
#include "umap/data/http_client.h"

using namespace umap::data;

class UploadWorkerTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};
    MockHttpClient mock_http;
    TraccarClient client{mock_http, "http://test.com:5055", "device-001"};
    UploadWorker worker{queue, client};

    void SetUp() override {
        queue.reset();
        mock_http.reset();
        mock_http.set_response(200, "{}");
    }
};

TEST_F(UploadWorkerTest, ProcessEmptyQueue) {
    uint32_t sent = worker.process_pending(10U);
    EXPECT_EQ(sent, 0U);
    EXPECT_EQ(worker.total_sent(), 0U);
}

TEST_F(UploadWorkerTest, ProcessSingleMessage) {
    queue.enqueue("{\"lat\":55.0,\"lon\":37.0}");
    EXPECT_EQ(queue.pending_count(), 1U);

    uint32_t sent = worker.process_pending(10U);
    EXPECT_EQ(sent, 1U);
    EXPECT_EQ(worker.total_sent(), 1U);
    EXPECT_EQ(queue.pending_count(), 0U);
}

TEST_F(UploadWorkerTest, ProcessBatch) {
    for (int i = 0; i < 5; ++i) {
        queue.enqueue("{\"seq\":1}");
    }

    uint32_t sent = worker.process_pending(5U);
    EXPECT_EQ(sent, 5U);
    EXPECT_EQ(worker.total_sent(), 5U);
}

TEST_F(UploadWorkerTest, BatchLimit) {
    for (int i = 0; i < 10; ++i) {
        queue.enqueue("{\"seq\":1}");
    }

    uint32_t sent = worker.process_pending(3U);
    EXPECT_EQ(sent, 3U);
    EXPECT_GE(queue.pending_count(), 5U);
}

TEST_F(UploadWorkerTest, NetworkFailure) {
    mock_http.set_should_fail(true);
    queue.enqueue("{\"lat\":55.0,\"lon\":37.0}");

    uint32_t sent = worker.process_pending(10U);
    EXPECT_EQ(sent, 0U);
    EXPECT_EQ(worker.total_failed(), 1U);
    EXPECT_EQ(queue.total_count(), 1U);
}

TEST_F(UploadWorkerTest, MixedSuccessAndFailure) {
    queue.enqueue("a");
    queue.enqueue("b");
    queue.enqueue("c");

    mock_http.set_should_fail(false);
    worker.process_pending(1U);
    EXPECT_EQ(worker.total_sent(), 1U);

    mock_http.set_should_fail(true);
    worker.process_pending(1U);
    EXPECT_EQ(worker.total_failed(), 1U);

    mock_http.set_should_fail(false);
    worker.process_pending(1U);
    EXPECT_EQ(worker.total_sent(), 2U);
}