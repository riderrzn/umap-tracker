// SPDX-License-Identifier: Apache-2.0
// Traccar client implementation: URL construction and HTTP dispatch
// Protocol: Traccar / OsmAnd (port 5055) — GET and JSON POST

#include "umap/data/traccar_client.h"

#include <cstdio>
#include <cmath>

namespace umap::data {

namespace {

constexpr const char* CONTENT_TYPE_JSON = "application/json";
constexpr int BUFFER_SIZE = 1024;

int encode_double(char* buf, int buf_size, double value) noexcept {
    return std::snprintf(buf, static_cast<size_t>(buf_size), "%.6f", value);
}

int encode_int(char* buf, int buf_size, int64_t value) noexcept {
    return std::snprintf(buf, static_cast<size_t>(buf_size), "%lld",
                         static_cast<long long>(value));
}

int encode_uint(char* buf, int buf_size, uint32_t value) noexcept {
    return std::snprintf(buf, static_cast<size_t>(buf_size), "%lu",
                         static_cast<unsigned long>(value));
}

int64_t to_traccar_timestamp(uint32_t timestamp_ms) noexcept {
    return static_cast<int64_t>(timestamp_ms) / 1000;
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
    char batt_buf[16];
    char hdop_buf[16];

    encode_double(lat_buf, sizeof(lat_buf), point.location.latitude);
    encode_double(lon_buf, sizeof(lon_buf), point.location.longitude);
    encode_double(alt_buf, sizeof(alt_buf), point.altitude_m);
    encode_double(speed_buf, sizeof(speed_buf), point.speed_kmh);
    encode_double(bearing_buf, sizeof(bearing_buf), point.bearing_deg);
    encode_double(accuracy_buf, sizeof(accuracy_buf), point.accuracy_m);
    encode_int(time_buf, sizeof(time_buf), to_traccar_timestamp(point.timestamp_ms));
    encode_uint(batt_buf, sizeof(batt_buf), point.battery_percent);
    encode_double(hdop_buf, sizeof(hdop_buf), point.hdop);

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
    url += "&batt=";
    url += batt_buf;
    url += "&hdop=";
    url += hdop_buf;

    return url;
}

std::string TraccarClient::build_json_payload(const core::GpsPoint& point) const noexcept {
    char buf[BUFFER_SIZE];

    int64_t ts = to_traccar_timestamp(point.timestamp_ms);

    int len = std::snprintf(buf, sizeof(buf),
        "{\"id\":\"%s\","
        "\"timestamp\":%lld,"
        "\"lat\":%.6f,"
        "\"lon\":%.6f,"
        "\"altitude\":%.1f,"
        "\"speed\":%.1f,"
        "\"bearing\":%.1f,"
        "\"accuracy\":%.1f,"
        "\"batt\":%lu,"
        "\"hdop\":%.1f}",
        device_id_,
        static_cast<long long>(ts),
        point.location.latitude,
        point.location.longitude,
        point.altitude_m,
        point.speed_kmh,
        point.bearing_deg,
        point.accuracy_m,
        static_cast<unsigned long>(point.battery_percent),
        point.hdop);

    if (len < 0 || len >= BUFFER_SIZE) {
        return "{}";
    }

    return std::string(buf, static_cast<size_t>(len));
}

utils::Result<void> TraccarClient::send(const core::GpsPoint& point) {
    // Traccar accepts both GET (OsmAnd) and POST (JSON) on port 5055.
    // Use GET as primary — simpler, no content-type negotiation.
    std::string url = build_url(point);
    auto result = http_.get(url.c_str());
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
