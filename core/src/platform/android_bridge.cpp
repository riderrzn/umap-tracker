// SPDX-License-Identifier: Apache-2.0
// Android JNI bridge implementation

#include "umap/platform/android_bridge.h"
#include "umap/utils/logger.h"

namespace umap::platform {

template class ThreadSafeQueue<core::GpsPoint, 1000U>;

bool LocationEngine::is_gps_available() noexcept {
    return instance().location_queue_size() > 0U;
}

void LocationEngine::initialize() noexcept {
    LOG_TAG_INFO("umap-bridge", "LocationEngine initialized");

    instance().unregister_callbacks();
    instance().clear_location_queue();
}

void LocationEngine::shutdown() noexcept {
    LOG_TAG_INFO("umap-bridge", "LocationEngine shutdown");

    instance().unregister_callbacks();
    instance().clear_location_queue();
}

}  // namespace umap::platform
