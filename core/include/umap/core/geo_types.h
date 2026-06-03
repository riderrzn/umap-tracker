// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Geographic data types (WGS84, GPS points)

#ifndef UMAP_CORE_GEO_TYPES_H_
#define UMAP_CORE_GEO_TYPES_H_

#include <cstdint>
#include <cmath>

namespace umap::core {

// WGS84 coordinate system
// Latitude range: [-90.0, 90.0]
// Longitude range: [-180.0, 180.0]
struct Coordinate {
    double latitude = 0.0;   // degrees
    double longitude = 0.0;  // degrees

    Coordinate() noexcept = default;
    
    Coordinate(double lat, double lon) noexcept 
        : latitude(lat), longitude(lon) {}

    // Check if coordinate is valid (WGS84 bounds)
    [[nodiscard]] bool is_valid() const noexcept {
        // NOLINTNEXTLINE(cert-fp30-c) — floating point comparison acceptable for bounds
        return latitude >= -90.0 && latitude <= 90.0 &&
               longitude >= -180.0 && longitude <= 180.0;
    }

    // Calculate approximate distance to another coordinate (Haversine formula)
    // Returns distance in meters
    [[nodiscard]] double distance_to(const Coordinate& other) const noexcept {
        // Earth radius in meters
        constexpr double EARTH_RADIUS_M = 6371000.0;
        
        // Convert to radians
        double lat1_rad = latitude * M_PI / 180.0;
        double lon1_rad = longitude * M_PI / 180.0;
        double lat2_rad = other.latitude * M_PI / 180.0;
        double lon2_rad = other.longitude * M_PI / 180.0;
        
        double dlat = lat2_rad - lat1_rad;
        double dlon = lon2_rad - lon1_rad;
        
        double a = std::sin(dlat / 2.0) * std::sin(dlat / 2.0) +
                   std::cos(lat1_rad) * std::cos(lat2_rad) *
                   std::sin(dlon / 2.0) * std::sin(dlon / 2.0);
        
        double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
        
        return EARTH_RADIUS_M * c;
    }
};

// GPS point with metadata (from location provider)
struct GpsPoint {
    Coordinate location{};          // WGS84 coordinates
    double speed_kmh = 0.0;         // Speed in km/h (from velocity or GPS field)
    double bearing_deg = 0.0;       // Direction in degrees [0, 360)
    double altitude_m = 0.0;        // Altitude in meters (MSL)
    double accuracy_m = 0.0;        // Horizontal accuracy in meters
    double hdop = 0.0;              // Horizontal Dilution of Precision
    uint32_t timestamp_ms = 0U;     // Milliseconds since epoch
    uint32_t battery_percent = 0U;  // Battery level [0, 100]
    bool is_charging = false;       // Is device charging

    GpsPoint() noexcept = default;

    // Check if point has valid location data
    [[nodiscard]] bool is_valid() const noexcept {
        return location.is_valid() && accuracy_m > 0.0 && accuracy_m < 1000.0;
    }

    // Get location quality indicator (0.0 = bad, 1.0 = excellent)
    // Based on accuracy, hdop, age
    [[nodiscard]] float quality_score() const noexcept {
        float score = 1.0f;
        
        // Penalize poor accuracy
        if (accuracy_m > 100.0) {
            score *= 0.5f;
        } else if (accuracy_m > 50.0) {
            score *= 0.7f;
        } else if (accuracy_m > 20.0) {
            score *= 0.9f;
        }
        
        // Penalize high HDOP
        if (hdop > 5.0) {
            score *= 0.5f;
        } else if (hdop > 2.0) {
            score *= 0.7f;
        }
        
        return score;
    }
};

}  // namespace umap::core

#endif  // UMAP_CORE_GEO_TYPES_H_
