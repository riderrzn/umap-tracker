// Placeholder stub implementations for Sprint 0

#include <umap/utils/logger.h>
#include <umap/utils/config.h>
#include <umap/core/geo_types.h>
#include <umap/platform/jni_types.h>

// These will be properly implemented in Sprint 1+

namespace umap::utils {
void log_impl(LogLevel level, const char* tag, const char* msg) noexcept {
    // TODO: Implement Android NDK logging
    (void)level;
    (void)tag;
    (void)msg;
}
}  // namespace umap::utils

namespace umap::data {
// Placeholder for MessageQueue
}

namespace umap::platform {
// Placeholder for Android bridge
}
