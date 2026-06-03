// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Result type: expected<T, E> for error handling without exceptions

#ifndef UMAP_UTILS_RESULT_H_
#define UMAP_UTILS_RESULT_H_

#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

namespace umap::utils {

// Error codes for Result type
enum class ErrorCode : uint8_t {
    Ok = 0U,
    NetworkError = 1U,
    GpsTimeout = 2U,
    ParseError = 3U,
    StorageFull = 4U,
    InvalidConfig = 5U,
    ThreadError = 6U,
    NotInitialized = 7U,
    OutOfRange = 8U,
    NotFound = 9U,
    InternalError = 10U,
};

// Helper to format error messages
inline const char* error_to_string(ErrorCode err) noexcept {
    switch (err) {
        case ErrorCode::Ok:
            return "OK";
        case ErrorCode::NetworkError:
            return "Network error";
        case ErrorCode::GpsTimeout:
            return "GPS timeout";
        case ErrorCode::ParseError:
            return "Parse error";
        case ErrorCode::StorageFull:
            return "Storage full";
        case ErrorCode::InvalidConfig:
            return "Invalid config";
        case ErrorCode::ThreadError:
            return "Thread error";
        case ErrorCode::NotInitialized:
            return "Not initialized";
        case ErrorCode::OutOfRange:
            return "Out of range";
        case ErrorCode::NotFound:
            return "Not found";
        case ErrorCode::InternalError:
            return "Internal error";
        default:
            return "Unknown error";
    }
}

// Sentinel value for error state
struct Unexpected {
    ErrorCode error;
    
    explicit Unexpected(ErrorCode err) noexcept : error(err) {}
};

inline Unexpected unexpected(ErrorCode err) noexcept {
    return Unexpected(err);
}

// Result<T> — a type that can hold either a value T or an error
// MISRA C++:2023 compliant — no exceptions, no dynamic allocation
template<typename T>
class Result {
 public:
    // Constructors
    Result(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : value_(value), has_value_(true) {}

    Result(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : value_(std::move(value)), has_value_(true) {}

    Result(Unexpected err) noexcept : error_(err.error), has_value_(false) {}

    Result() noexcept : error_(ErrorCode::Ok), has_value_(false) {}

    // Copy
    Result(const Result& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (other.has_value_) {
            new (&value_) T(other.value_);
        } else {
            error_ = other.error_;
        }
    }

    Result& operator=(const Result& other) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                value_ = other.value_;
            } else if (has_value_ && !other.has_value_) {
                value_.~T();
                error_ = other.error_;
                has_value_ = false;
            } else if (!has_value_ && other.has_value_) {
                new (&value_) T(other.value_);
                has_value_ = true;
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }

    // Move
    Result(Result&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : has_value_(other.has_value_) {
        if (other.has_value_) {
            new (&value_) T(std::move(other.value_));
        } else {
            error_ = other.error_;
        }
    }

    Result& operator=(Result&& other) noexcept(std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            if (has_value_ && other.has_value_) {
                value_ = std::move(other.value_);
            } else if (has_value_ && !other.has_value_) {
                value_.~T();
                error_ = other.error_;
                has_value_ = false;
            } else if (!has_value_ && other.has_value_) {
                new (&value_) T(std::move(other.value_));
                has_value_ = true;
            } else {
                error_ = other.error_;
            }
        }
        return *this;
    }

    // Destructor
    ~Result() noexcept {
        if (has_value_) {
            value_.~T();
        }
    }

    // Observers
    [[nodiscard]] bool has_value() const noexcept { return has_value_; }

    [[nodiscard]] bool is_error() const noexcept { return !has_value_; }

    [[nodiscard]] const T& value() const noexcept {
        // Caller must check has_value() first (MISRA: no undefined behavior)
        return value_;
    }

    [[nodiscard]] T& value() noexcept {
        return value_;
    }

    [[nodiscard]] ErrorCode error() const noexcept { return error_; }

    [[nodiscard]] explicit operator bool() const noexcept { return has_value_; }

 private:
    union {
        T value_;
        ErrorCode error_;
    };
    bool has_value_;
};

// Specialization for void (Result<void> for operations with no return value)
template<>
class Result<void> {
 public:
    Result() noexcept : error_(ErrorCode::Ok), has_value_(true) {}

    Result(Unexpected err) noexcept : error_(err.error), has_value_(false) {}

    [[nodiscard]] bool has_value() const noexcept { return has_value_; }

    [[nodiscard]] bool is_error() const noexcept { return !has_value_; }

    [[nodiscard]] ErrorCode error() const noexcept { return error_; }

    [[nodiscard]] explicit operator bool() const noexcept { return has_value_; }

 private:
    ErrorCode error_;
    bool has_value_;
};

}  // namespace umap::utils

#endif  // UMAP_UTILS_RESULT_H_
