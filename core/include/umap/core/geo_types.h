// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Geographic data types

#ifndef UMAP_CORE_GEO_TYPES_H_
#define UMAP_CORE_GEO_TYPES_H_

#include <cstdint>

namespace umap::core {

// WGS84 coordinate system
struct Coordinate {
    double latitude = 0.0;
    double longitude = 0.0;

    Coordinate() noexcept = default;
    Coordinate(double lat, double lon) noexcept : latitude(lat), longitude(lon) {}
};

// GPS point with metadata
struct GpsPoint {
    Coordinate location{};
    double speed_kmh = 0.0;  // km/h
    double bearing_deg = 0.0;  // degrees (0-360)
    double altitude_m = 0.0;  // meters
    double accuracy_m = 0.0;  // meters (horizontal accuracy)
    double hdop = 0.0;  // Horizontal Dilution of Precision
    uint32_t timestamp_ms = 0U;  // milliseconds since epoch
    uint32_t battery_percent = 0U;  // 0-100
    bool is_charging = false;

    GpsPoint() noexcept = default;
};

}  // namespace umap::core

#endif  // UMAP_CORE_GEO_TYPES_H_
