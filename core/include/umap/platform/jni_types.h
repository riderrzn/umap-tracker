// SPDX-License-Identifier: Apache-2.0
// JNI type definitions

#ifndef UMAP_PLATFORM_JNI_TYPES_H_
#define UMAP_PLATFORM_JNI_TYPES_H_

#include <jni.h>

namespace umap::platform {

// Callback types for JNI
using LocationCallback = void (*)(double lat, double lon, float accuracy, uint32_t timestamp_ms);
using BatteryCallback = void (*)(uint32_t battery_percent, bool is_charging);
using ErrorCallback = void (*)(int error_code, const char* message);

}  // namespace umap::platform

#endif  // UMAP_PLATFORM_JNI_TYPES_H_
