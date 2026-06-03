#include <gtest/gtest.h>
#include <umap/utils/result.h>

namespace umap::utils {

TEST(ResultTest, SuccessCase) {
    using Result = tl::expected<int, int>;
    Result res = 42;
    EXPECT_TRUE(res.has_value());
    EXPECT_EQ(res.value(), 42);
}

TEST(ResultTest, ErrorCase) {
    using Result = tl::expected<int, std::string>;
    Result res = tl::unexpected(std::string("Error"));
    EXPECT_FALSE(res.has_value());
    EXPECT_EQ(res.error(), "Error");
}

TEST(ResultTest, MoveSemantics) {
    using Result = tl::expected<std::string, int>;
    Result res = std::string("hello");
    Result moved = std::move(res);
    EXPECT_TRUE(moved.has_value());
}

}  // namespace umap::utils
