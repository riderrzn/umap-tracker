// SPDX-License-Identifier: Apache-2.0
// Traccar client implementation: URL construction and HTTP dispatch

#include "umap/data/traccar_client.h"

#include <cstdio>
#include <cmath>

namespace umap::data {

namespace {

constexpr const char* CONTENT_TYPE_JSON = "application/json";
constexpr int BUFFER_SIZE = 1024;

int url_encode_double(char* buf, int buf_size, double value) noexcept {
    return std::snprintf(buf, static_cast<size_t>(buf_size), "%.6f", value);
}

int url_encode_int(char* buf, int buf_size, int64_t value) noexcept {
    return std::snprintf(buf, static_cast<size_t>(buf_size), "%lld",
                         static_cast<long long>(value));
}

}  // namespace

TraccarClient::TraccarClient(IHttpClient& http_client,
                               const char* server_url,
                               const char* device_id) noexcept
    : http_(http_client), server_url_(server_url), device_id_(device_id) {}

std::string TraccarClient::build_url(const core::GpsPoint& point) const noexcept {
    char lat_buf[32];
    char lon_buf[32];
    char alt_buf[32];
    char speed_buf[32];
    char bearing_buf[32];
    char accuracy_buf[32];
    char time_buf[32];

    url_encode_double(lat_buf, sizeof(lat_buf), point.location.latitude);
    url_encode_double(lon_buf, sizeof(lon_buf), point.location.longitude);
    url_encode_double(alt_buf, sizeof(alt_buf), point.altitude_m);
    url_encode_double(speed_buf, sizeof(speed_buf), point.speed_kmh);
    url_encode_double(bearing_buf, sizeof(bearing_buf), point.bearing_deg);
    url_encode_double(accuracy_buf, sizeof(accuracy_buf), point.accuracy_m);
    url_encode_int(time_buf, sizeof(time_buf),
                   static_cast<int64_t>(point.timestamp_ms));

    std::string url;
    url.reserve(512);
    url += server_url_;
    url += "?id=";
    url += device_id_;
    url += "&lat=";
    url += lat_buf;
    url += "&lon=";
    url += lon_buf;
    url += "&altitude=";
    url += alt_buf;
    url += "&speed=";
    url += speed_buf;
    url += "&bearing=";
    url += bearing_buf;
    url += "&accuracy=";
    url += accuracy_buf;
    url += "&timestamp=";
    url += time_buf;

    return url;
}

std::string TraccarClient::build_json_payload(const core::GpsPoint& point) const noexcept {
    char buf[BUFFER_SIZE];

    int len = std::snprintf(buf, sizeof(buf),
        "{\"id\":\"%s\","
        "\"latitude\":%.6f,"
        "\"longitude\":%.6f,"
        "\"altitude\":%.1f,"
        "\"speed\":%.1f,"
        "\"bearing\":%.1f,"
        "\"accuracy\":%.1f,"
        "\"timestamp\":%lu}",
        device_id_,
        point.location.latitude,
        point.location.longitude,
        point.altitude_m,
        point.speed_kmh,
        point.bearing_deg,
        point.accuracy_m,
        static_cast<unsigned long>(point.timestamp_ms));

    if (len < 0 || len >= BUFFER_SIZE) {
        return "{}";
    }

    return std::string(buf, static_cast<size_t>(len));
}

utils::Result<void> TraccarClient::send(const core::GpsPoint& point) {
    std::string json = build_json_payload(point);
    auto result = http_.post(server_url_, CONTENT_TYPE_JSON, json.c_str());
    if (!result.has_value()) {
        return utils::Unexpected(result.error());
    }
    const HttpResponse& resp = result.value();
    if (resp.status_code < 200 || resp.status_code >= 300) {
        return utils::Unexpected(utils::ErrorCode::NetworkError);
    }
    return utils::Result<void>();
}

}  // namespace umap::data
