// SPDX-License-Identifier: Apache-2.0
// State Machine utility implementations

#include "umap/core/state_machine.h"

namespace umap::core {

const char* event_to_string(StateEvent event) noexcept {
    switch (event) {
        case StateEvent::StartTracking:     return "StartTracking";
        case StateEvent::StopTracking:      return "StopTracking";
        case StateEvent::LocationAcquired:  return "LocationAcquired";
        case StateEvent::LocationLost:      return "LocationLost";
        case StateEvent::UploadStarted:     return "UploadStarted";
        case StateEvent::UploadCompleted:   return "UploadCompleted";
        case StateEvent::UploadFailed:      return "UploadFailed";
        case StateEvent::PauseTracking:     return "PauseTracking";
        case StateEvent::ResumeTracking:    return "ResumeTracking";
        case StateEvent::ErrorOccurred:     return "ErrorOccurred";
        case StateEvent::ErrorRecovered:    return "ErrorRecovered";
        default:                            return "Unknown";
    }
}

}  // namespace umap::core