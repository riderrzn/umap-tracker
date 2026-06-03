# Tests for umap::core (Google Test)

include(FetchContent)

# Fetch Google Test
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# ============================================================================
# Test executables
# ============================================================================

add_executable(test_utils)

target_sources(test_utils
    PRIVATE
        test_result.cpp
        test_logger.cpp
        test_ring_buffer.cpp
        test_geo_types.cpp
        test_config.cpp
        test_android_bridge.cpp
        test_kalman_filter.cpp
        test_state_machine.cpp
        test_message_queue.cpp
        test_traccar_client.cpp
        test_gps_pipeline.cpp
        test_upload_worker.cpp
        test_stress.cpp
        integration_kalman_state.cpp
        integration_queue_traccar.cpp
        bench_kalman.cpp
)

target_link_libraries(test_utils
    PRIVATE
        umap-core
        gtest
        gtest_main
)

target_compile_features(test_utils PRIVATE cxx_std_17)

# Apply MISRA flags to tests
if(MSVC)
    target_compile_options(test_utils PRIVATE /W4)
else()
    target_compile_options(test_utils PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# ============================================================================
# Register tests
# ============================================================================

add_test(NAME UtilsTests COMMAND test_utils)

message(STATUS "Tests configured")
