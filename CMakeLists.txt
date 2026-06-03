cmake_minimum_required(VERSION 3.22)
project(umap-tracker
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "Android GPS Tracker with Kalman Filter (MISRA C++:2023)"
)

# ============================================================================
# Global settings
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MISRA C++:2023 compliance flags
set(MISRA_CXX_FLAGS
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual
    -Wno-c++98-compat -Wno-c++98-compat-pedantic
    -fno-exceptions -fno-rtti
    -fstack-protector-strong
)

add_compile_options(${MISRA_CXX_FLAGS})

# clang-tidy integration
if(CLANG_TIDY)
    set(CMAKE_CXX_CLANG_TIDY
        "clang-tidy"
        "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
        "--header-filter=${CMAKE_SOURCE_DIR}/core/include/.*"
    )
endif()

# ============================================================================
# Options
# ============================================================================

option(BUILD_TESTING "Enable tests" ON)
option(ENABLE_CLANG_TIDY "Run clang-tidy" OFF)
option(ANDROID_BUILD "Build for Android" OFF)

# ============================================================================
# Core library
# ============================================================================

add_subdirectory(core)

# ============================================================================
# Tests
# ============================================================================

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(core/tests)
endif()

# ============================================================================
# Print configuration
# ============================================================================

message(STATUS "=== umap-tracker Configuration ===")
message(STATUS "CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "BUILD_TESTING: ${BUILD_TESTING}")
message(STATUS "ANDROID_BUILD: ${ANDROID_BUILD}")
message(STATUS "====================================")
