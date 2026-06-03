#include <gtest/gtest.h>
#include <umap/utils/config.h>

namespace umap::utils {

// ============================================================================
// Configuration Constants Tests
// ============================================================================

TEST(ConfigTest, QueueSizeValid) {
    // Ensure queue size is reasonable
    EXPECT_GT(MAX_QUEUE_SIZE, 0U);
    EXPECT_LE(MAX_QUEUE_SIZE, 100000U);
}

TEST(ConfigTest, MessageTTLValid) {
    // 7 days in hours
    EXPECT_EQ(MESSAGE_TTL_HOURS, 168U);
}

TEST(ConfigTest, TrackRetentionValid) {
    // 90 days
    EXPECT_EQ(TRACK_RETENTION_DAYS, 90U);
}

TEST(ConfigTest, MaxTrackPointsValid) {
    // Reasonable upper bound for track points
    EXPECT_GT(MAX_TRACK_POINTS, 0U);
}

TEST(ConfigTest, GPSIntervalValid) {
    // GPS interval should be reasonable (1-60 seconds)
    EXPECT_GE(DEFAULT_GPS_INTERVAL_MS, 1000U);
    EXPECT_LE(DEFAULT_GPS_INTERVAL_MS, 60000U);
}

TEST(ConfigTest, GPSAccuracyValid) {
    // Minimum accuracy should be reasonable
    EXPECT_GT(MIN_GPS_ACCURACY_M, 0U);
    EXPECT_LE(MIN_GPS_ACCURACY_M, 50U);
}

TEST(ConfigTest, GPSTimeoutValid) {
    // GPS timeout should be > interval
    EXPECT_GT(GPS_TIMEOUT_MS, DEFAULT_GPS_INTERVAL_MS);
}

TEST(ConfigTest, PerformanceTargetsValid) {
    // GPS to UI latency should be < 500ms
    EXPECT_LT(GPS_UI_LATENCY_MAX_MS, 500U);
    
    // Upload timeout should be reasonable
    EXPECT_LT(UPLOAD_TIMEOUT_MS, 5000U);
    
    // Cold start should be < 5 seconds
    EXPECT_LT(COLD_START_MAX_MS, 5000U);
}

TEST(ConfigTest, BatteryDrainReasonable) {
    // Battery drain should be 1-10% per hour in balanced mode
    EXPECT_GT(BATTERY_DRAIN_MAX_PERCENT_PER_HOUR, 1.0f);
    EXPECT_LT(BATTERY_DRAIN_MAX_PERCENT_PER_HOUR, 10.0f);
}

}  // namespace umap::utils
