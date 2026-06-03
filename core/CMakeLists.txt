# umap::core library (MISRA C++:2023)

# ============================================================================
# Headers
# ============================================================================

set(UMAP_CORE_HEADERS
    include/umap/core/geo_types.h
    include/umap/data/message_queue.h
    include/umap/data/traccar_client.h
    include/umap/platform/android_bridge.h
    include/umap/platform/jni_types.h
    include/umap/utils/config.h
    include/umap/utils/logger.h
    include/umap/utils/result.h
    include/umap/utils/ring_buffer.h
)

# ============================================================================
# Core Library
# ============================================================================

add_library(umap-core STATIC)

target_sources(umap-core
    PUBLIC
        ${UMAP_CORE_HEADERS}
    PRIVATE
        src/core/state_machine.cpp
        src/core/kalman_filter.cpp
        src/data/message_queue.cpp
        src/data/traccar_client.cpp
        src/platform/android_bridge.cpp
        src/utils/logger.cpp
        src/utils/config.cpp
)

target_include_directories(umap-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# C++ standard
target_compile_features(umap-core PUBLIC cxx_std_17)

# Apply MISRA C++:2023 flags to our library
if(MSVC)
    target_compile_options(umap-core PRIVATE /W4)
else()
    target_compile_options(umap-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# ============================================================================
# Android NDK (if enabled)
# ============================================================================

if(ANDROID)
    message(STATUS "Building for Android...")
    
    # Find Android NDK
    if(NOT ANDROID_ABI)
        set(ANDROID_ABI arm64-v8a)
    endif()
    
    message(STATUS "Android ABI: ${ANDROID_ABI}")
    
    # Link Android libraries
    target_link_libraries(umap-core
        PRIVATE
            android
            log
    )
    
    # JNI library
    add_library(umap-jni SHARED)
    target_sources(umap-jni PRIVATE jni/jni_bridge.cpp)
    target_link_libraries(umap-jni PRIVATE umap-core)
    target_include_directories(umap-jni
        PRIVATE
            include
            ${ANDROID_NDK}/sources/android/native_app_glue
    )
endif()

# ============================================================================
# External dependencies (stub for now)
# ============================================================================

# TODO: Add libcurl, SQLite, etc.
# For now, headers-only approach for faster iteration

message(STATUS "umap-core library configured")
