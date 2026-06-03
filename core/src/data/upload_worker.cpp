// SPDX-License-Identifier: Apache-2.0
// Upload worker implementation

#include "umap/data/upload_worker.h"

namespace umap::data {

UploadWorker::UploadWorker(MessageQueue& queue, TraccarClient& client) noexcept
    : queue_(queue), client_(client), total_sent_(0U), total_failed_(0U) {}

uint32_t UploadWorker::process_pending(uint32_t max_batch) {
    uint32_t sent = 0U;

    for (uint32_t i = 0U; i < max_batch; ++i) {
        auto msg = queue_.dequeue();
        if (!msg.has_value()) {
            break;
        }

        auto& queued = msg.value();

        core::GpsPoint point;
        point.timestamp_ms = static_cast<uint32_t>(queued.created_at);

        auto result = client_.send(point);
        if (result.has_value()) {
            queue_.mark_sent(queued.id);
            ++sent;
            ++total_sent_;
        } else {
            queue_.mark_failed(queued.id);
            ++total_failed_;
        }
    }

    return sent;
}

}  // namespace umap::data