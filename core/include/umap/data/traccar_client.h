// SPDX-License-Identifier: Apache-2.0
// Traccar client: constructs URLs/payloads and sends via HTTP

#ifndef UMAP_DATA_TRACCAR_CLIENT_H_
#define UMAP_DATA_TRACCAR_CLIENT_H_

#include <cstdint>
#include <string>

#include "umap/core/geo_types.h"
#include "umap/data/http_client.h"
#include "umap/utils/result.h"

namespace umap::data {

class TraccarClient {
public:
    TraccarClient(IHttpClient& http_client,
                   const char* server_url,
                   const char* device_id) noexcept;

    TraccarClient(const TraccarClient&) = delete;
    TraccarClient& operator=(const TraccarClient&) = delete;
    TraccarClient(TraccarClient&&) = delete;
    TraccarClient& operator=(TraccarClient&&) = delete;

    [[nodiscard]] std::string build_url(const core::GpsPoint& point) const noexcept;

    [[nodiscard]] std::string build_json_payload(const core::GpsPoint& point) const noexcept;

    utils::Result<void> send(const core::GpsPoint& point);

    [[nodiscard]] const char* server_url() const noexcept { return server_url_; }
    [[nodiscard]] const char* device_id() const noexcept { return device_id_; }

private:
    IHttpClient& http_;
    const char* server_url_;
    const char* device_id_;
};

}  // namespace umap::data

#endif  // UMAP_DATA_TRACCAR_CLIENT_H_