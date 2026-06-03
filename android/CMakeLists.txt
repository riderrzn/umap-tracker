# Android NDK CMakeLists for JNI

cmake_minimum_required(VERSION 3.22)
project(umap-jni LANGUAGES CXX)

# Include main core library
include_directories(${CMAKE_SOURCE_DIR}/../core/include)

# Add core library
add_subdirectory(${CMAKE_SOURCE_DIR}/../core core)

# JNI Library
add_library(umap-jni SHARED)

target_sources(umap-jni
    PRIVATE
        ${CMAKE_SOURCE_DIR}/../core/jni/jni_bridge.cpp
)

target_link_libraries(umap-jni
    PRIVATE
        umap-core
        android
        log
)

target_compile_features(umap-jni PRIVATE cxx_std_17)
