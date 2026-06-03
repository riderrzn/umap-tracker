#include <gtest/gtest.h>
#include <umap/core/geo_types.h>
#include <cmath>

namespace umap::core {

// ============================================================================
// Coordinate Tests
// ============================================================================

TEST(CoordinateTest, DefaultConstructor) {
    Coordinate coord;
    EXPECT_DOUBLE_EQ(coord.latitude, 0.0);
    EXPECT_DOUBLE_EQ(coord.longitude, 0.0);
}

TEST(CoordinateTest, ParameterizedConstructor) {
    Coordinate coord(55.7558, 37.6173);
    EXPECT_DOUBLE_EQ(coord.latitude, 55.7558);
    EXPECT_DOUBLE_EQ(coord.longitude, 37.6173);
}

TEST(CoordinateTest, IsValidOk) {
    Coordinate coord(55.7558, 37.6173);
    EXPECT_TRUE(coord.is_valid());
}

TEST(CoordinateTest, IsValidLatitudeTooHigh) {
    Coordinate coord(91.0, 0.0);
    EXPECT_FALSE(coord.is_valid());
}

TEST(CoordinateTest, IsValidLatitudeTooLow) {
    Coordinate coord(-91.0, 0.0);
    EXPECT_FALSE(coord.is_valid());
}

TEST(CoordinateTest, IsValidLongitudeTooHigh) {
    Coordinate coord(0.0, 181.0);
    EXPECT_FALSE(coord.is_valid());
}

TEST(CoordinateTest, IsValidLongitudeTooLow) {
    Coordinate coord(0.0, -181.0);
    EXPECT_FALSE(coord.is_valid());
}

TEST(CoordinateTest, DistanceSamePoint) {
    Coordinate coord(55.7558, 37.6173);
    double dist = coord.distance_to(coord);
    EXPECT_NEAR(dist, 0.0, 0.01);
}

TEST(CoordinateTest, DistanceKnownPair) {
    // Moscow and St. Petersburg (roughly 700 km apart)
    Coordinate moscow(55.7558, 37.6173);
    Coordinate spb(59.9311, 30.3609);
    double dist = moscow.distance_to(spb);
    // Should be approximately 700 km
    EXPECT_GT(dist, 600000.0);  // > 600 km
    EXPECT_LT(dist, 800000.0);  // < 800 km
}

// ============================================================================
// GpsPoint Tests
// ============================================================================

TEST(GpsPointTest, DefaultConstructor) {
    GpsPoint point;
    EXPECT_DOUBLE_EQ(point.speed_kmh, 0.0);
    EXPECT_DOUBLE_EQ(point.bearing_deg, 0.0);
    EXPECT_DOUBLE_EQ(point.altitude_m, 0.0);
    EXPECT_EQ(point.battery_percent, 0U);
    EXPECT_FALSE(point.is_charging);
}

TEST(GpsPointTest, IsValidGoodPoint) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.accuracy_m = 5.0;
    EXPECT_TRUE(point.is_valid());
}

TEST(GpsPointTest, IsValidInvalidLocation) {
    GpsPoint point;
    point.location = Coordinate(91.0, 0.0);  // Invalid latitude
    point.accuracy_m = 5.0;
    EXPECT_FALSE(point.is_valid());
}

TEST(GpsPointTest, IsValidZeroAccuracy) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.accuracy_m = 0.0;
    EXPECT_FALSE(point.is_valid());
}

TEST(GpsPointTest, IsValidTooHighAccuracy) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.accuracy_m = 1001.0;  // > 1000 m
    EXPECT_FALSE(point.is_valid());
}

TEST(GpsPointTest, QualityScorePerfect) {
    GpsPoint point;
    point.accuracy_m = 5.0;
    point.hdop = 1.0;
    float score = point.quality_score();
    EXPECT_GT(score, 0.9f);
    EXPECT_LE(score, 1.0f);
}

TEST(GpsPointTest, QualityScorePoor) {
    GpsPoint point;
    point.accuracy_m = 150.0;  // > 100 m
    point.hdop = 6.0;  // > 5.0
    float score = point.quality_score();
    EXPECT_LT(score, 0.5f);
}

TEST(GpsPointTest, QualityScoreMedium) {
    GpsPoint point;
    point.accuracy_m = 30.0;  // [20, 50]
    point.hdop = 2.5;  // [2, 5]
    float score = point.quality_score();
    EXPECT_GT(score, 0.5f);
    EXPECT_LT(score, 0.9f);
}

}  // namespace umap::core
