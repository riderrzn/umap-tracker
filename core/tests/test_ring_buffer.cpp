#include <gtest/gtest.h>
#include <umap/utils/ring_buffer.h>

namespace umap::utils {

class RingBufferTest : public ::testing::Test {
 protected:
    RingBuffer<int, 10> buffer_;
};

TEST_F(RingBufferTest, InitiallyEmpty) {
    EXPECT_TRUE(buffer_.empty());
    EXPECT_FALSE(buffer_.full());
}

TEST_F(RingBufferTest, PushAndPop) {
    buffer_.push(42);
    EXPECT_FALSE(buffer_.empty());
    
    auto val = buffer_.pop();
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 42);
    EXPECT_TRUE(buffer_.empty());
}

TEST_F(RingBufferTest, FillBuffer) {
    for (int i = 0; i < 10; ++i) {
        buffer_.push(i);
    }
    EXPECT_TRUE(buffer_.full());
}

TEST_F(RingBufferTest, WrapAround) {
    for (int i = 0; i < 15; ++i) {
        buffer_.push(i);
    }
    // Buffer wraps around, last 10 elements should remain
    int count = 0;
    while (!buffer_.empty()) {
        buffer_.pop();
        ++count;
    }
    EXPECT_EQ(count, 10);
}

}  // namespace umap::utils
