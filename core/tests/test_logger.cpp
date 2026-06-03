#include <gtest/gtest.h>
#include <umap/utils/logger.h>

namespace umap::utils {

class LoggerTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Initialize logger for tests
    }
};

TEST_F(LoggerTest, LogDebug) {
    // This test verifies that debug logging compiles
    // In Android, logs go to logcat
    LOG_DEBUG("Test debug message");
    SUCCEED();
}

TEST_F(LoggerTest, LogInfo) {
    LOG_INFO("Test info message");
    SUCCEED();
}

TEST_F(LoggerTest, LogWarn) {
    LOG_WARN("Test warning message");
    SUCCEED();
}

TEST_F(LoggerTest, LogError) {
    LOG_ERROR("Test error message");
    SUCCEED();
}

}  // namespace umap::utils
