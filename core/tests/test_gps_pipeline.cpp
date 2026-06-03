// SPDX-License-Identifier: Apache-2.0
// GPS processing pipeline tests: raw GPS → Kalman → MessageQueue

#include <gtest/gtest.h>

#include "umap/core/gps_pipeline.h"
#include "umap/data/message_queue.h"

using namespace umap::core;
using namespace umap::data;

class GpsPipelineTest : public ::testing::Test {
protected:
    MessageQueue queue{":memory:"};
    GpsProcessingPipeline pipeline{queue};

    void SetUp() override {
        queue.reset();
        pipeline.reset();
    }
};

TEST_F(GpsPipelineTest, ProcessSingleValidPoint) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.accuracy_m = 5.0;
    point.altitude_m = 150.0;
    point.timestamp_ms = 1000U;

    auto result = pipeline.process_location(point);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(pipeline.processed_count(), 1U);
    EXPECT_EQ(pipeline.queued_count(), 1U);
    EXPECT_EQ(queue.pending_count(), 1U);
}

TEST_F(GpsPipelineTest, ProcessInvalidPoint) {
    GpsPoint point;
    point.location = Coordinate(91.0, 0.0);
    point.accuracy_m = 0.0;

    auto result = pipeline.process_location(point);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(pipeline.processed_count(), 0U);
}

TEST_F(GpsPipelineTest, MultiplePointsSequential) {
    for (int i = 0; i < 10; ++i) {
        GpsPoint point;
        point.location = Coordinate(55.0 + i * 0.001, 37.0);
        point.accuracy_m = 5.0;
        point.altitude_m = 100.0;
        point.timestamp_ms = 1000U + i * 1000U;

        auto result = pipeline.process_location(point);
        EXPECT_TRUE(result.has_value());
    }

    EXPECT_EQ(pipeline.processed_count(), 10U);
    EXPECT_EQ(queue.pending_count(), 10U);
}

TEST_F(GpsPipelineTest, FilteredOutputHasSpeedAndBearing) {
    GpsPoint point1;
    point1.location = Coordinate(55.0, 37.0);
    point1.accuracy_m = 5.0;
    point1.timestamp_ms = 1000U;
    pipeline.process_location(point1);

    GpsPoint point2;
    point2.location = Coordinate(55.001, 37.001);
    point2.accuracy_m = 5.0;
    point2.timestamp_ms = 2000U;
    pipeline.process_location(point2);

    auto msg = queue.dequeue();
    ASSERT_TRUE(msg.has_value());

    auto msg2 = queue.dequeue();
    ASSERT_TRUE(msg2.has_value());

    EXPECT_NE(msg2.value().payload.find("speed"), std::string::npos);
    EXPECT_NE(msg2.value().payload.find("bearing"), std::string::npos);
}

TEST_F(GpsPipelineTest, ResetClearsState) {
    GpsPoint point;
    point.location = Coordinate(55.0, 37.0);
    point.accuracy_m = 5.0;
    point.timestamp_ms = 1000U;
    pipeline.process_location(point);

    pipeline.reset();
    EXPECT_EQ(pipeline.processed_count(), 0U);
    EXPECT_EQ(pipeline.queued_count(), 0U);
}