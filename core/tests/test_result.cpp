#include <gtest/gtest.h>
#include <umap/utils/result.h>

namespace umap::utils {

// ============================================================================
// Result<int> Tests
// ============================================================================

TEST(ResultTest, ConstructorWithValue) {
    Result<int> res(42);
    EXPECT_TRUE(res.has_value());
    EXPECT_FALSE(res.is_error());
    EXPECT_EQ(res.value(), 42);
}

TEST(ResultTest, ConstructorWithError) {
    Result<int> res = unexpected(ErrorCode::NetworkError);
    EXPECT_FALSE(res.has_value());
    EXPECT_TRUE(res.is_error());
    EXPECT_EQ(res.error(), ErrorCode::NetworkError);
}

TEST(ResultTest, DefaultConstructor) {
    Result<int> res;
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::Ok);
}

TEST(ResultTest, CopyConstructor) {
    Result<int> res1(42);
    Result<int> res2(res1);
    EXPECT_TRUE(res2.has_value());
    EXPECT_EQ(res2.value(), 42);
}

TEST(ResultTest, CopyConstructorError) {
    Result<int> res1 = unexpected(ErrorCode::ParseError);
    Result<int> res2(res1);
    EXPECT_FALSE(res2.has_value());
    EXPECT_EQ(res2.error(), ErrorCode::ParseError);
}

TEST(ResultTest, MoveConstructor) {
    Result<int> res1(42);
    Result<int> res2(std::move(res1));
    EXPECT_TRUE(res2.has_value());
    EXPECT_EQ(res2.value(), 42);
}

TEST(ResultTest, CopyAssignment) {
    Result<int> res1(42);
    Result<int> res2(0);
    res2 = res1;
    EXPECT_TRUE(res2.has_value());
    EXPECT_EQ(res2.value(), 42);
}

TEST(ResultTest, MoveAssignment) {
    Result<int> res1(42);
    Result<int> res2(0);
    res2 = std::move(res1);
    EXPECT_TRUE(res2.has_value());
    EXPECT_EQ(res2.value(), 42);
}

TEST(ResultTest, BoolOperator) {
    Result<int> res_ok(42);
    Result<int> res_err = unexpected(ErrorCode::GpsTimeout);
    
    EXPECT_TRUE(static_cast<bool>(res_ok));
    EXPECT_FALSE(static_cast<bool>(res_err));
}

// ============================================================================
// Result<std::string> Tests (more complex type)
// ============================================================================

TEST(ResultTest, StringValue) {
    Result<std::string> res(std::string("hello"));
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "hello");
}

TEST(ResultTest, StringMove) {
    std::string str("world");
    Result<std::string> res(std::move(str));
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), "world");
}

TEST(ResultTest, StringCopy) {
    Result<std::string> res1(std::string("test"));
    Result<std::string> res2 = res1;
    EXPECT_TRUE(res2.has_value());
    EXPECT_EQ(res2.value(), "test");
}

// ============================================================================
// Result<void> Tests
// ============================================================================

TEST(ResultVoidTest, SuccessVoid) {
    Result<void> res;
    EXPECT_TRUE(res.has_value());
    EXPECT_FALSE(res.is_error());
}

TEST(ResultVoidTest, ErrorVoid) {
    Result<void> res = unexpected(ErrorCode::StorageFull);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::StorageFull);
}

// ============================================================================
// Error Code String Conversion
// ============================================================================

TEST(ErrorCodeTest, ToStringOk) {
    EXPECT_STREQ(error_to_string(ErrorCode::Ok), "OK");
}

TEST(ErrorCodeTest, ToStringNetworkError) {
    EXPECT_STREQ(error_to_string(ErrorCode::NetworkError), "Network error");
}

TEST(ErrorCodeTest, ToStringGpsTimeout) {
    EXPECT_STREQ(error_to_string(ErrorCode::GpsTimeout), "GPS timeout");
}

TEST(ErrorCodeTest, ToStringParseError) {
    EXPECT_STREQ(error_to_string(ErrorCode::ParseError), "Parse error");
}

// ============================================================================
// Pattern Tests (error handling patterns)
// ============================================================================

Result<int> divide(int a, int b) noexcept {
    if (b == 0) {
        return unexpected(ErrorCode::InvalidConfig);
    }
    return a / b;
}

TEST(ResultTest, PatternErrorHandling) {
    auto res = divide(10, 0);
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), ErrorCode::InvalidConfig);
}

TEST(ResultTest, PatternSuccessHandling) {
    auto res = divide(10, 2);
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 5);
}

}  // namespace umap::utils
