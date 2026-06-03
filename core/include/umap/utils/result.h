// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Result type (tl::expected wrapper for error handling)

#ifndef UMAP_UTILS_RESULT_H_
#define UMAP_UTILS_RESULT_H_

#include <cstdint>

// TODO: Replace with actual tl::expected when dependency is available
// For Sprint 0, this is a placeholder that will be filled in Sprint 1

namespace umap::utils {

// Placeholder Result type
// In production, use: #include <tl/expected.hpp>
// and use: template<typename T> using Result = tl::expected<T, ErrorCode>;

enum class ErrorCode : uint8_t {
    Ok = 0,
    NetworkError = 1,
    GpsTimeout = 2,
    ParseError = 3,
    StorageFull = 4,
    InvalidConfig = 5,
    ThreadError = 6,
};

// TODO: Implement proper Result<T> type in Sprint 1

}  // namespace umap::utils

#endif  // UMAP_UTILS_RESULT_H_
