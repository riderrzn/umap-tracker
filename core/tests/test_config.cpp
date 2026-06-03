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

// ============================================================================
// RuntimeConfig Tests
// ============================================================================

class RuntimeConfigTest : public ::testing::Test {
protected:
    void SetUp() override {
        RuntimeConfig::instance().reset_to_defaults();
    }

    void TearDown() override {
        RuntimeConfig::instance().reset_to_defaults();
    }
};

TEST_F(RuntimeConfigTest, DefaultsMatchConstexpr) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    EXPECT_EQ(cfg.max_queue_size(), MAX_QUEUE_SIZE);
    EXPECT_EQ(cfg.message_ttl_hours(), MESSAGE_TTL_HOURS);
    EXPECT_EQ(cfg.track_retention_days(), TRACK_RETENTION_DAYS);
    EXPECT_EQ(cfg.max_track_points(), MAX_TRACK_POINTS);
    EXPECT_EQ(cfg.gps_interval_ms(), DEFAULT_GPS_INTERVAL_MS);
    EXPECT_EQ(cfg.min_gps_accuracy_m(), MIN_GPS_ACCURACY_M);
    EXPECT_EQ(cfg.gps_timeout_ms(), GPS_TIMEOUT_MS);
    EXPECT_EQ(cfg.gps_ui_latency_max_ms(), GPS_UI_LATENCY_MAX_MS);
    EXPECT_EQ(cfg.upload_timeout_ms(), UPLOAD_TIMEOUT_MS);
    EXPECT_EQ(cfg.cold_start_max_ms(), COLD_START_MAX_MS);
    EXPECT_FLOAT_EQ(cfg.battery_drain_max_percent_per_hour(),
                    BATTERY_DRAIN_MAX_PERCENT_PER_HOUR);
}

TEST_F(RuntimeConfigTest, OverrideMaxQueueSize) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_max_queue_size(100U);
    EXPECT_EQ(cfg.max_queue_size(), 100U);
}

TEST_F(RuntimeConfigTest, OverrideGpsInterval) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_gps_interval_ms(10000U);
    EXPECT_EQ(cfg.gps_interval_ms(), 10000U);
}

TEST_F(RuntimeConfigTest, InvalidQueueSizeRejected) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_max_queue_size(0U);
    EXPECT_EQ(cfg.max_queue_size(), MAX_QUEUE_SIZE);
}

TEST_F(RuntimeConfigTest, InvalidGpsIntervalRejected) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_gps_interval_ms(500U);
    EXPECT_EQ(cfg.gps_interval_ms(), DEFAULT_GPS_INTERVAL_MS);
}

TEST_F(RuntimeConfigTest, ResetToDefaultsWorks) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_max_queue_size(100U);
    cfg.set_gps_interval_ms(999U);
    cfg.set_message_ttl_hours(24U);
    cfg.reset_to_defaults();
    EXPECT_EQ(cfg.max_queue_size(), MAX_QUEUE_SIZE);
    EXPECT_EQ(cfg.gps_interval_ms(), DEFAULT_GPS_INTERVAL_MS);
    EXPECT_EQ(cfg.message_ttl_hours(), MESSAGE_TTL_HOURS);
}

TEST_F(RuntimeConfigTest, SingletonReturnsSameInstance) {
    RuntimeConfig& a = RuntimeConfig::instance();
    RuntimeConfig& b = RuntimeConfig::instance();
    EXPECT_EQ(&a, &b);
}

TEST_F(RuntimeConfigTest, OverrideBatteryDrain) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_battery_drain_max_percent_per_hour(3.0f);
    EXPECT_FLOAT_EQ(cfg.battery_drain_max_percent_per_hour(), 3.0f);
}

TEST_F(RuntimeConfigTest, InvalidBatteryDrainRejected) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_battery_drain_max_percent_per_hour(0.0f);
    EXPECT_FLOAT_EQ(cfg.battery_drain_max_percent_per_hour(),
                    BATTERY_DRAIN_MAX_PERCENT_PER_HOUR);
}

TEST_F(RuntimeConfigTest, InvalidGpsTimeoutRejected) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_gps_timeout_ms(100U);
    EXPECT_EQ(cfg.gps_timeout_ms(), GPS_TIMEOUT_MS);
}

TEST_F(RuntimeConfigTest, OverrideUploadTimeout) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_upload_timeout_ms(1000U);
    EXPECT_EQ(cfg.upload_timeout_ms(), 1000U);
}

TEST_F(RuntimeConfigTest, MultipleOverridesAndReset) {
    RuntimeConfig& cfg = RuntimeConfig::instance();
    cfg.set_max_queue_size(25000U);
    cfg.set_message_ttl_hours(72U);
    cfg.set_track_retention_days(30U);
    EXPECT_EQ(cfg.max_queue_size(), 25000U);
    EXPECT_EQ(cfg.message_ttl_hours(), 72U);
    EXPECT_EQ(cfg.track_retention_days(), 30U);
    cfg.reset_to_defaults();
    EXPECT_EQ(cfg.max_queue_size(), MAX_QUEUE_SIZE);
}

}  // namespace umap::utils
