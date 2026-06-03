// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Configuration constants

#ifndef UMAP_UTILS_CONFIG_H_
#define UMAP_UTILS_CONFIG_H_

#include <cstdint>

namespace umap::utils {

// Queue management
constexpr uint32_t MAX_QUEUE_SIZE = 50000U;
constexpr uint32_t MESSAGE_TTL_HOURS = 168U;

// Track storage
constexpr uint32_t TRACK_RETENTION_DAYS = 90U;
constexpr uint32_t MAX_TRACK_POINTS = 1000000U;

// GPS parameters
constexpr uint32_t DEFAULT_GPS_INTERVAL_MS = 5000U;  // 5 seconds
constexpr uint32_t MIN_GPS_ACCURACY_M = 10U;  // 10 meters
constexpr uint32_t GPS_TIMEOUT_MS = 60000U;  // 60 seconds

// Performance targets
constexpr uint32_t GPS_UI_LATENCY_MAX_MS = 200U;
constexpr uint32_t UPLOAD_TIMEOUT_MS = 500U;
constexpr uint32_t COLD_START_MAX_MS = 3000U;

// Battery drain target (percentage per hour in balanced mode)
constexpr float BATTERY_DRAIN_MAX_PERCENT_PER_HOUR = 5.0f;

}  // namespace umap::utils

#endif  // UMAP_UTILS_CONFIG_H_
