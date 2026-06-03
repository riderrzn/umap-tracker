#include <gtest/gtest.h>
#include <umap/utils/ring_buffer.h>

namespace umap::utils {

class RingBufferTest : public ::testing::Test {
 protected:
    RingBuffer<int, 10> buffer_;
};

// ============================================================================
// Basic Operations
// ============================================================================

TEST_F(RingBufferTest, InitiallyEmpty) {
    EXPECT_TRUE(buffer_.empty());
    EXPECT_FALSE(buffer_.full());
    EXPECT_EQ(buffer_.size(), 0U);
}

TEST_F(RingBufferTest, PushSingleElement) {
    buffer_.push(42);
    EXPECT_FALSE(buffer_.empty());
    EXPECT_FALSE(buffer_.full());
    EXPECT_EQ(buffer_.size(), 1U);
}

TEST_F(RingBufferTest, PushAndPop) {
    buffer_.push(42);
    EXPECT_FALSE(buffer_.empty());
    
    auto val = buffer_.pop();
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 42);
    EXPECT_TRUE(buffer_.empty());
}

TEST_F(RingBufferTest, PopFromEmpty) {
    auto val = buffer_.pop();
    EXPECT_FALSE(val.has_value());
}

// ============================================================================
// Fill and Overflow
// ============================================================================

TEST_F(RingBufferTest, FillBufferCompletely) {
    for (int i = 0; i < 10; ++i) {
        buffer_.push(i);
    }
    EXPECT_TRUE(buffer_.full());
    EXPECT_EQ(buffer_.size(), 10U);
}

TEST_F(RingBufferTest, OverflowDiscardOldest) {
    // Fill buffer
    for (int i = 0; i < 10; ++i) {
        buffer_.push(i);
    }
    EXPECT_TRUE(buffer_.full());
    
    // Push one more (should discard 0)
    buffer_.push(10);
    EXPECT_TRUE(buffer_.full());
    EXPECT_EQ(buffer_.size(), 10U);
    
    // First element should be 1 (not 0)
    auto val = buffer_.pop();
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 1);
}

// ============================================================================
// FIFO Behavior
// ============================================================================

TEST_F(RingBufferTest, FIFOOrder) {
    for (int i = 0; i < 5; ++i) {
        buffer_.push(i);
    }
    
    for (int i = 0; i < 5; ++i) {
        auto val = buffer_.pop();
        EXPECT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i);
    }
    EXPECT_TRUE(buffer_.empty());
}

// ============================================================================
// Wrap-Around Behavior
// ============================================================================

TEST_F(RingBufferTest, WrapAroundSimple) {
    // Fill buffer (0-9)
    for (int i = 0; i < 10; ++i) {
        buffer_.push(i);
    }
    
    // Pop 5 elements (0-4)
    for (int i = 0; i < 5; ++i) {
        buffer_.pop();
    }
    EXPECT_EQ(buffer_.size(), 5U);
    
    // Push 5 more (10-14)
    for (int i = 10; i < 15; ++i) {
        buffer_.push(i);
    }
    EXPECT_EQ(buffer_.size(), 10U);
    
    // Should pop 5-9, then 10-14
    for (int i = 5; i < 15; ++i) {
        auto val = buffer_.pop();
        EXPECT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), i);
    }
}

TEST_F(RingBufferTest, WrapAroundMultipleTimes) {
    // Push and pop multiple times to cause wrap-around
    for (int cycle = 0; cycle < 3; ++cycle) {
        for (int i = 0; i < 10; ++i) {
            buffer_.push(cycle * 100 + i);
        }
        EXPECT_TRUE(buffer_.full());
        
        for (int i = 0; i < 10; ++i) {
            auto val = buffer_.pop();
            EXPECT_TRUE(val.has_value());
            EXPECT_EQ(val.value(), cycle * 100 + i);
        }
        EXPECT_TRUE(buffer_.empty());
    }
}

// ============================================================================
// Size Limits
// ============================================================================

TEST_F(RingBufferTest, SizeTrackingAccurate) {
    EXPECT_EQ(buffer_.size(), 0U);
    
    buffer_.push(1);
    EXPECT_EQ(buffer_.size(), 1U);
    
    buffer_.push(2);
    EXPECT_EQ(buffer_.size(), 2U);
    
    buffer_.pop();
    EXPECT_EQ(buffer_.size(), 1U);
    
    buffer_.pop();
    EXPECT_EQ(buffer_.size(), 0U);
}

// ============================================================================
// Different Element Types
// ============================================================================

TEST(RingBufferTypesTest, DoubleValues) {
    RingBuffer<double, 5> buf;
    buf.push(3.14);
    buf.push(2.71);
    
    auto val1 = buf.pop();
    EXPECT_TRUE(val1.has_value());
    EXPECT_DOUBLE_EQ(val1.value(), 3.14);
    
    auto val2 = buf.pop();
    EXPECT_TRUE(val2.has_value());
    EXPECT_DOUBLE_EQ(val2.value(), 2.71);
}

TEST(RingBufferTypesTest, StructValues) {
    struct Point {
        int x, y;
    };
    
    RingBuffer<Point, 3> buf;
    buf.push({1, 2});
    buf.push({3, 4});
    
    auto val = buf.pop();
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value().x, 1);
    EXPECT_EQ(val.value().y, 2);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(RingBufferTest, SingleElementBuffer) {
    // This would be static_assert'd at compile time, but test anyway
    RingBuffer<int, 1> tiny_buf;
    
    EXPECT_TRUE(tiny_buf.empty());
    tiny_buf.push(99);
    EXPECT_TRUE(tiny_buf.full());
    
    auto val = tiny_buf.pop();
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(val.value(), 99);
}

}  // namespace umap::utils
