#include <gtest/gtest.h>
#include <umap/utils/logger.h>
#include <cstring>
#include <string>

namespace umap::utils {

class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Logger is stateless — no setup needed
    }
};

TEST_F(LoggerTest, LogDebugDoesNotCrash) {
    LOG_DEBUG("Test debug message");
    SUCCEED();
}

TEST_F(LoggerTest, LogInfoDoesNotCrash) {
    LOG_INFO("Test info message");
    SUCCEED();
}

TEST_F(LoggerTest, LogWarnDoesNotCrash) {
    LOG_WARN("Test warning message");
    SUCCEED();
}

TEST_F(LoggerTest, LogErrorDoesNotCrash) {
    LOG_ERROR("Test error message");
    SUCCEED();
}

TEST_F(LoggerTest, LogWithSpecialCharacters) {
    LOG_INFO("Special chars: !@#$%^&*()_+-=[]{}|;':\",./<>?");
    LOG_DEBUG("Newlines\nTabs\tBackslash\\");
    SUCCEED();
}

TEST_F(LoggerTest, LogEmptyString) {
    LOG_INFO("");
    SUCCEED();
}

TEST_F(LoggerTest, LogLongMessage) {
    std::string long_msg(1000, 'x');
    LOG_INFO(long_msg.c_str());
    SUCCEED();
}

TEST_F(LoggerTest, LogUnicodeMessage) {
    LOG_INFO("Unicode test: Москва Санкт-Петербург GPS трекер");
    SUCCEED();
}

TEST_F(LoggerTest, DirectLogImplNullTag) {
    log_impl(LogLevel::Info, nullptr, "should not crash");
    SUCCEED();
}

TEST_F(LoggerTest, DirectLogImplNullMsg) {
    log_impl(LogLevel::Info, "test", nullptr);
    SUCCEED();
}

TEST_F(LoggerTest, DirectLogImplBothNull) {
    log_impl(LogLevel::Error, nullptr, nullptr);
    SUCCEED();
}

TEST_F(LoggerTest, AllLogLevelsDirectCall) {
    log_impl(LogLevel::Debug, "test", "debug direct");
    log_impl(LogLevel::Info, "test", "info direct");
    log_impl(LogLevel::Warning, "test", "warning direct");
    log_impl(LogLevel::Error, "test", "error direct");
    SUCCEED();
}

TEST_F(LoggerTest, TaggedLoggingMacros) {
    LOG_TAG_DEBUG("GpsModule", "acquiring satellites");
    LOG_TAG_INFO("NetModule", "connecting to server");
    LOG_TAG_WARN("QueueModule", "queue 80% full");
    LOG_TAG_ERROR("UploadModule", "connection timeout");
    SUCCEED();
}

TEST_F(LoggerTest, RapidConsecutiveLogging) {
    for (int i = 0; i < 100; ++i) {
        LOG_DEBUG("Rapid logging iteration");
    }
    SUCCEED();
}

TEST_F(LoggerTest, LogLevelEnumValues) {
    EXPECT_NE(static_cast<uint8_t>(LogLevel::Debug),
              static_cast<uint8_t>(LogLevel::Info));
    EXPECT_NE(static_cast<uint8_t>(LogLevel::Info),
              static_cast<uint8_t>(LogLevel::Warning));
    EXPECT_NE(static_cast<uint8_t>(LogLevel::Warning),
              static_cast<uint8_t>(LogLevel::Error));
}

TEST_F(LoggerTest, DebugLogDoesNotAffectSubsequentCalls) {
    LOG_ERROR("error1");
    LOG_DEBUG("debug");
    LOG_ERROR("error2");
    SUCCEED();
}

}  // namespace umap::utils
