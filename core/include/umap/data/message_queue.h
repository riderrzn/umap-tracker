// SPDX-License-Identifier: Apache-2.0
// MessageQueue: SQLite-backed offline queue for GPS data

#ifndef UMAP_DATA_MESSAGE_QUEUE_H_
#define UMAP_DATA_MESSAGE_QUEUE_H_

#include <cstdint>
#include <string>

#include "umap/utils/result.h"

namespace umap::data {

struct QueuedMessage {
    uint64_t id = 0U;
    std::string payload;
    uint32_t retry_count = 0U;
    int64_t created_at = 0;
    int64_t next_retry_at = 0;
};

class MessageQueue {
public:
    explicit MessageQueue(const char* db_path);
    ~MessageQueue() noexcept;

    MessageQueue(const MessageQueue&) = delete;
    MessageQueue& operator=(const MessageQueue&) = delete;
    MessageQueue(MessageQueue&&) = delete;
    MessageQueue& operator=(MessageQueue&&) = delete;

    utils::Result<void> enqueue(const char* payload);

    utils::Result<QueuedMessage> dequeue();

    utils::Result<void> mark_sent(uint64_t message_id);

    utils::Result<void> mark_failed(uint64_t message_id);

    [[nodiscard]] uint32_t pending_count() const noexcept;

    [[nodiscard]] uint32_t total_count() const noexcept;

    uint32_t purge_expired(uint32_t ttl_hours);

    void reset();

    [[nodiscard]] utils::Result<uint32_t> get_retry_count(uint64_t message_id) const noexcept;

private:
    void create_tables();
    int64_t now_seconds() const noexcept;
    int64_t calculate_backoff(uint32_t retry_count) const noexcept;
    void finalize() noexcept;

    struct Impl;
    Impl* impl_;
};

}  // namespace umap::data

#endif  // UMAP_DATA_MESSAGE_QUEUE_H_