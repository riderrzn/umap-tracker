// SPDX-License-Identifier: Apache-2.0
// Upload worker: dequeues messages and sends to Traccar server

#ifndef UMAP_DATA_UPLOAD_WORKER_H_
#define UMAP_DATA_UPLOAD_WORKER_H_

#include <cstdint>

#include "umap/data/message_queue.h"
#include "umap/data/traccar_client.h"
#include "umap/utils/result.h"

namespace umap::data {

class UploadWorker {
public:
    UploadWorker(MessageQueue& queue, TraccarClient& client) noexcept;

    UploadWorker(const UploadWorker&) = delete;
    UploadWorker& operator=(const UploadWorker&) = delete;
    UploadWorker(UploadWorker&&) = delete;
    UploadWorker& operator=(UploadWorker&&) = delete;

    // Process up to max_batch pending messages
    // Returns number of successfully sent messages
    uint32_t process_pending(uint32_t max_batch = 10U);

    [[nodiscard]] uint32_t total_sent() const noexcept { return total_sent_; }
    [[nodiscard]] uint32_t total_failed() const noexcept { return total_failed_; }
    [[nodiscard]] uint32_t pending_count() const noexcept { return queue_.pending_count(); }

private:
    MessageQueue& queue_;
    TraccarClient& client_;
    uint32_t total_sent_;
    uint32_t total_failed_;
};

}  // namespace umap::data

#endif  // UMAP_DATA_UPLOAD_WORKER_H_