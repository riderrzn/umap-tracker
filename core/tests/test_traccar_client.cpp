// SPDX-License-Identifier: Apache-2.0
// TraccarClient tests: URL construction, JSON payload, HTTP dispatch
// Protocol: Traccar / OsmAnd (port 5055)

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
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.speed_kmh = 45.0;
    point.bearing_deg = 180.0;
    point.altitude_m = 150.0;
    point.accuracy_m = 5.0;
    point.timestamp_ms = 5000000U;  // 5M ms → 5000 sec
    point.battery_percent = 85U;
    point.hdop = 1.5;

    std::string url = client.build_url(point);

    EXPECT_NE(url.find("id=test-device-001"), std::string::npos);
    EXPECT_NE(url.find("lat=55.755800"), std::string::npos);
    EXPECT_NE(url.find("lon=37.617300"), std::string::npos);
    EXPECT_NE(url.find("speed=45.000000"), std::string::npos);
    EXPECT_NE(url.find("bearing=180.000000"), std::string::npos);
    EXPECT_NE(url.find("altitude=150.000000"), std::string::npos);
    EXPECT_NE(url.find("accuracy=5.000000"), std::string::npos);
    EXPECT_NE(url.find("timestamp=5000"), std::string::npos) << "URL: " << url;
    EXPECT_NE(url.find("batt=85"), std::string::npos);
    EXPECT_NE(url.find("hdop=1.500000"), std::string::npos);
}

TEST_F(TraccarClientTest, BuildJsonPayloadValid) {
    GpsPoint point;
    point.location = Coordinate(59.9311, 30.3609);
    point.speed_kmh = 0.0;
    point.bearing_deg = 0.0;
    point.altitude_m = 10.0;
    point.accuracy_m = 3.5;
    point.timestamp_ms = 5000000U;  // 5M ms → 5000 sec
    point.battery_percent = 90U;
    point.hdop = 1.2;

    std::string json = client.build_json_payload(point);

    EXPECT_NE(json.find("test-device-001"), std::string::npos);
    EXPECT_NE(json.find("\"lat\":59.931100"), std::string::npos);
    EXPECT_NE(json.find("\"lon\":30.360900"), std::string::npos);
    EXPECT_NE(json.find("\"timestamp\":5000"), std::string::npos) << "JSON: " << json;
    EXPECT_NE(json.find("\"altitude\":10.0"), std::string::npos);
    EXPECT_NE(json.find("\"speed\":0.0"), std::string::npos);
    EXPECT_NE(json.find("\"bearing\":0.0"), std::string::npos);
    EXPECT_NE(json.find("\"accuracy\":3.5"), std::string::npos);
    EXPECT_NE(json.find("\"batt\":90"), std::string::npos);
    EXPECT_NE(json.find("\"hdop\":1.2"), std::string::npos);
}

TEST_F(TraccarClientTest, SendUsesGetMethod) {
    GpsPoint point;
    point.location = Coordinate(55.7558, 37.6173);
    point.speed_kmh = 0.0;
    point.accuracy_m = 5.0;
    point.timestamp_ms = 5000000U;

    mock_http.set_response(200, "{}");
    auto result = client.send(point);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(mock_http.request_count(), 1U);
    EXPECT_EQ(mock_http.requests()[0].method, "GET");
}

TEST_F(TraccarClientTest, SendNetworkError) {
    GpsPoint point;
    point.location = Coordinate(55.0, 37.0);
    point.accuracy_m = 5.0;
    point.timestamp_ms = 5000000U;

    mock_http.set_should_fail(true);
    auto result = client.send(point);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::NetworkError);
}

TEST_F(TraccarClientTest, SendServerErrorResponse) {
    GpsPoint point;
    point.location = Coordinate(55.0, 37.0);
    point.accuracy_m = 5.0;
    point.timestamp_ms = 5000000U;

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
    GpsPoint point;
    point.location = Coordinate(0.0, 0.0);
    point.timestamp_ms = 0U;

    std::string url = client.build_url(point);
    EXPECT_NE(url.find("lat=0.000000"), std::string::npos);
    EXPECT_NE(url.find("lon=0.000000"), std::string::npos);
    EXPECT_NE(url.find("timestamp=0"), std::string::npos);
}

TEST_F(TraccarClientTest, TimestampConversionMsToSec) {
    GpsPoint point;
    point.location = Coordinate(55.0, 37.0);
    point.timestamp_ms = 3000000U;  // 3 million ms → 3000 sec

    std::string url = client.build_url(point);
    EXPECT_NE(url.find("timestamp=3000"), std::string::npos) << "URL: " << url;

    std::string json = client.build_json_payload(point);
    EXPECT_NE(json.find("\"timestamp\":3000"), std::string::npos) << "JSON: " << json;
}

// ============================================================================
// MockHttpClient Standalone Tests
// ============================================================================

TEST(MockHttpClientTest, DefaultSuccessResponse) {
    MockHttpClient mock;
    auto result = mock.get("http://test.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().status_code, 200);
    EXPECT_EQ(result.value().body, "{}");
}

TEST(MockHttpClientTest, CustomResponse) {
    MockHttpClient mock;
    mock.set_response(201, "created");
    auto result = mock.post("http://test.com", "text/plain", "hello");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().status_code, 201);
    EXPECT_EQ(result.value().body, "created");
}

TEST(MockHttpClientTest, FailureMode) {
    MockHttpClient mock;
    mock.set_should_fail(true);
    auto result = mock.get("http://test.com");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::NetworkError);
}

TEST(MockHttpClientTest, ResetClearsState) {
    MockHttpClient mock;
    mock.set_response(500, "error");
    mock.set_should_fail(true);
    mock.get("http://test.com");

    mock.reset();

    auto result = mock.get("http://test.com");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value().status_code, 200);
    EXPECT_EQ(result.value().body, "{}");
    EXPECT_EQ(mock.request_count(), 1U);
}

TEST(MockHttpClientTest, RecordsMultipleRequests) {
    MockHttpClient mock;
    mock.get("http://a.com");
    mock.post("http://b.com", "json", "{}");
    mock.get("http://c.com");

    ASSERT_EQ(mock.request_count(), 3U);
    EXPECT_EQ(mock.requests()[0].method, "GET");
    EXPECT_EQ(mock.requests()[0].url, "http://a.com");
    EXPECT_EQ(mock.requests()[1].method, "POST");
    EXPECT_EQ(mock.requests()[1].url, "http://b.com");
    EXPECT_EQ(mock.requests()[1].content_type, "json");
    EXPECT_EQ(mock.requests()[1].body, "{}");
    EXPECT_EQ(mock.requests()[2].method, "GET");
    EXPECT_EQ(mock.requests()[2].url, "http://c.com");
}

// ============================================================================
// ErrorCode Enum Tests
// ============================================================================

TEST(ErrorCodeTest, AllErrorsHaveStrings) {
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::Ok), "OK");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::NetworkError), "Network error");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::GpsTimeout), "GPS timeout");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::ParseError), "Parse error");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::StorageFull), "Storage full");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::InvalidConfig), "Invalid config");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::ThreadError), "Thread error");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::NotInitialized), "Not initialized");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::OutOfRange), "Out of range");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::NotFound), "Not found");
    EXPECT_STREQ(error_to_string(umap::utils::ErrorCode::InternalError), "Internal error");
}