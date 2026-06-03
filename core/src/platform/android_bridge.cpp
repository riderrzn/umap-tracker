// SPDX-License-Identifier: Apache-2.0
// Android JNI bridge implementation

#include "umap/platform/android_bridge.h"
#include "umap/utils/logger.h"

namespace umap::platform {

// Explicit template instantiation for ThreadSafeQueue<GpsPoint>
// This ensures the template is compiled in the translation unit
template class ThreadSafeQueue<core::GpsPoint, 1000U>;

}  // namespace umap::platform
