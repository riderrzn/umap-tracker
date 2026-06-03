// SPDX-License-Identifier: Apache-2.0
// TraccarClient tests: URL construction, JSON payload, HTTP dispatch

#include <gtest/gtest.h>

#include "umap/data/traccar_client.h"
#include "umap/data/http_client.h"
#include "umap/core/geo_types.h"

using namespace umap::data;
using namespace umap::core;

class TraccarClientTest : public ::testing::Test {
protected:
    MockHttpClient mock_http;
    TraccarClient client{mock_http, "http://test.example.com:5055", "test-device-001"};

    void SetUp() override {
        mock_http.reset();
        mock_http.set_response(200, "{}");
    }
};

TEST_F(TraccarClientTest, BuildUrlContainsRequiredParams) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(55.7558, 37.6173);
    point.speed_kmh = 45.0;
    point.bearing_deg = 180.0;
    point.altitude_m = 150.0;
    point.accuracy_m = 5.0;
    point.timestamp_ms = 1717500000U;

    std::string url = client.build_url(point);

    EXPECT_NE(url.find("id=test-device-001"), std::string::npos);
    EXPECT_NE(url.find("lat=55.755800"), std::string::npos);
    EXPECT_NE(url.find("lon=37.617300"), std::string::npos);
    EXPECT_NE(url.find("speed=45.000000"), std::string::npos);
    EXPECT_NE(url.find("bearing=180.000000"), std::string::npos);
    EXPECT_NE(url.find("altitude=150.000000"), std::string::npos);
    EXPECT_NE(url.find("accuracy=5.000000"), std::string::npos);
    EXPECT_NE(url.find("timestamp=1717500000"), std::string::npos);
}

TEST_F(TraccarClientTest, BuildJsonPayloadValid) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(59.9311, 30.3609);
    point.speed_kmh = 0.0;
    point.bearing_deg = 0.0;
    point.altitude_m = 10.0;
    point.accuracy_m = 3.5;
    point.timestamp_ms = 1717500000U;

    std::string json = client.build_json_payload(point);

    EXPECT_NE(json.find("test-device-001"), std::string::npos);
    EXPECT_NE(json.find("59.931100"), std::string::npos);
    EXPECT_NE(json.find("30.360900"), std::string::npos);
    EXPECT_NE(json.find("\"altitude\":10.0"), std::string::npos);
    EXPECT_NE(json.find("\"speed\":0.0"), std::string::npos);
    EXPECT_NE(json.find("\"bearing\":0.0"), std::string::npos);
}

TEST_F(TraccarClientTest, SendSuccessfulRequest) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(55.7558, 37.6173);
    point.speed_kmh = 0.0;
    point.accuracy_m = 5.0;
    point.timestamp_ms = 1717500000U;

    mock_http.set_response(200, "{}");
    auto result = client.send(point);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(mock_http.request_count(), 1U);
}

TEST_F(TraccarClientTest, SendNetworkError) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(55.0, 37.0);
    point.accuracy_m = 5.0;
    point.timestamp_ms = 1717500000U;

    mock_http.set_should_fail(true);
    auto result = client.send(point);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::NetworkError);
}

TEST_F(TraccarClientTest, SendServerErrorResponse) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(55.0, 37.0);
    point.accuracy_m = 5.0;
    point.timestamp_ms = 1717500000U;

    mock_http.set_response(500, "Internal Server Error");
    auto result = client.send(point);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::NetworkError);
}

TEST_F(TraccarClientTest, HttpClientMockRecordsRequests) {
    mock_http.set_response(200, "{}");

    auto result = mock_http.post("http://test.com/api", "application/json", "{\"data\":1}");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().status_code, 200);
    EXPECT_EQ(result.value().body, "{}");

    ASSERT_EQ(mock_http.request_count(), 1U);
    EXPECT_EQ(mock_http.requests()[0].method, "POST");
    EXPECT_EQ(mock_http.requests()[0].url, "http://test.com/api");
    EXPECT_EQ(mock_http.requests()[0].content_type, "application/json");
    EXPECT_EQ(mock_http.requests()[0].body, "{\"data\":1}");
}

TEST_F(TraccarClientTest, BuildUrlZeroCoordinates) {
    umap::core::GpsPoint point;
    point.location = umap::core::Coordinate(0.0, 0.0);
    point.timestamp_ms = 0U;

    std::string url = client.build_url(point);
    EXPECT_NE(url.find("lat=0.000000"), std::string::npos);
    EXPECT_NE(url.find("lon=0.000000"), std::string::npos);
}
