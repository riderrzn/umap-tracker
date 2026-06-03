// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
/// @defgroup state_machine State Machine
/// @brief Location tracking state machine with 6 states and 11 events.
///
/// States: Idle → Acquiring → Tracking ⇄ Uploading ⇄ Tracking
///         Tracking → Paused → Tracking
///         Any → Error → Idle
/// @{

#ifndef UMAP_CORE_STATE_MACHINE_H_
#define UMAP_CORE_STATE_MACHINE_H_

#include <cstdint>
#include <functional>

#include "umap/utils/result.h"

namespace umap::core {

// ============================================================================
// Location States
// ============================================================================

enum class LocationState : uint8_t {
    Idle = 0U,         // Not tracking, GPS off
    Acquiring = 1U,    // GPS acquiring signal
    Tracking = 2U,     // Active GPS tracking
    Uploading = 3U,    // Uploading data to server
    Paused = 4U,       // Temporarily paused
    Error = 5U         // Error state
};

// Convert state to string for logging
[[nodiscard]] inline const char* state_to_string(LocationState state) noexcept {
    switch (state) {
        case LocationState::Idle:      return "Idle";
        case LocationState::Acquiring: return "Acquiring";
        case LocationState::Tracking:  return "Tracking";
        case LocationState::Uploading: return "Uploading";
        case LocationState::Paused:    return "Paused";
        case LocationState::Error:     return "Error";
        default:                       return "Unknown";
    }
}

// ============================================================================
// State Events
// ============================================================================

enum class StateEvent : uint8_t {
    StartTracking = 0U,
    StopTracking = 1U,
    LocationAcquired = 2U,
    LocationLost = 3U,
    UploadStarted = 4U,
    UploadCompleted = 5U,
    UploadFailed = 6U,
    PauseTracking = 7U,
    ResumeTracking = 8U,
    ErrorOccurred = 9U,
    ErrorRecovered = 10U
};

// ============================================================================
// State Transition
// ============================================================================

struct StateTransition {
    LocationState from_state;
    LocationState to_state;
    StateEvent trigger_event;
};

// Convert event to string for logging
[[nodiscard]] const char* event_to_string(StateEvent event) noexcept;

// ============================================================================
// State Machine Event Callbacks
// ============================================================================

using StateChangeCallback = std::function<void(LocationState, LocationState)>;
using ErrorCallback = std::function<void(int, const char*)>;

// ============================================================================
// Location State Machine
// ============================================================================

class LocationStateMachine {
public:
    LocationStateMachine() noexcept : current_state_(LocationState::Idle) {}

    // Delete copy operations
    LocationStateMachine(const LocationStateMachine&) = delete;
    LocationStateMachine& operator=(const LocationStateMachine&) = delete;

    // Delete move operations
    LocationStateMachine(LocationStateMachine&&) = delete;
    LocationStateMachine& operator=(LocationStateMachine&&) = delete;

    ~LocationStateMachine() noexcept = default;

    // Get current state
    [[nodiscard]] LocationState get_state() const noexcept {
        return current_state_;
    }

    // Get state name
    [[nodiscard]] const char* get_state_name() const noexcept {
        return state_to_string(current_state_);
    }

    // Process event and transition if valid
    [[nodiscard]] utils::Result<void> process_event(StateEvent event) noexcept {
        LocationState next_state = get_next_state(current_state_, event);

        // If no valid transition, return error
        if (next_state == current_state_) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        // Perform transition
        LocationState previous_state = current_state_;
        current_state_ = next_state;

        // Invoke callback if registered
        if (state_change_callback_ != nullptr) {
            // NOLINTNEXTLINE(cert-err58-cpp) — functional callback invocation
            state_change_callback_(previous_state, next_state);
        }

        return utils::Result<void>();
    }

    // Register state change callback
    [[nodiscard]] utils::Result<void> register_state_callback(
        StateChangeCallback callback) noexcept {
        if (callback == nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }
        state_change_callback_ = callback;
        return utils::Result<void>();
    }

    // Register error callback
    [[nodiscard]] utils::Result<void> register_error_callback(
        ErrorCallback callback) noexcept {
        if (callback == nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }
        error_callback_ = callback;
        return utils::Result<void>();
    }

    // Unregister callbacks
    void unregister_callbacks() noexcept {
        state_change_callback_ = nullptr;
        error_callback_ = nullptr;
    }

    // Reset to idle state
    void reset() noexcept {
        current_state_ = LocationState::Idle;
    }

    // Check if can transition from state with event
    [[nodiscard]] bool can_transition(LocationState state,
                                     StateEvent event) const noexcept {
        return get_next_state(state, event) != state;
    }

private:
    // Determine next state given current state and event
    [[nodiscard]] static LocationState get_next_state(
        LocationState current, StateEvent event) noexcept {
        // State transition table
        switch (current) {
            case LocationState::Idle:
                if (event == StateEvent::StartTracking) {
                    return LocationState::Acquiring;
                }
                break;

            case LocationState::Acquiring:
                if (event == StateEvent::LocationAcquired) {
                    return LocationState::Tracking;
                } else if (event == StateEvent::StopTracking) {
                    return LocationState::Idle;
                } else if (event == StateEvent::ErrorOccurred) {
                    return LocationState::Error;
                }
                break;

            case LocationState::Tracking:
                if (event == StateEvent::LocationLost) {
                    return LocationState::Acquiring;
                } else if (event == StateEvent::UploadStarted) {
                    return LocationState::Uploading;
                } else if (event == StateEvent::PauseTracking) {
                    return LocationState::Paused;
                } else if (event == StateEvent::StopTracking) {
                    return LocationState::Idle;
                } else if (event == StateEvent::ErrorOccurred) {
                    return LocationState::Error;
                }
                break;

            case LocationState::Uploading:
                if (event == StateEvent::UploadCompleted) {
                    return LocationState::Tracking;
                } else if (event == StateEvent::UploadFailed) {
                    return LocationState::Tracking;  // Retry tracking
                } else if (event == StateEvent::StopTracking) {
                    return LocationState::Idle;
                } else if (event == StateEvent::ErrorOccurred) {
                    return LocationState::Error;
                }
                break;

            case LocationState::Paused:
                if (event == StateEvent::ResumeTracking) {
                    return LocationState::Tracking;
                } else if (event == StateEvent::StopTracking) {
                    return LocationState::Idle;
                } else if (event == StateEvent::ErrorOccurred) {
                    return LocationState::Error;
                }
                break;

            case LocationState::Error:
                if (event == StateEvent::ErrorRecovered) {
                    return LocationState::Idle;
                } else if (event == StateEvent::StopTracking) {
                    return LocationState::Idle;
                }
                break;

            default:
                break;
        }

        // No valid transition found
        return current;
    }

    LocationState current_state_;
    StateChangeCallback state_change_callback_ = nullptr;
    ErrorCallback error_callback_ = nullptr;
};

}  // namespace umap::core

/// @}

#endif  // UMAP_CORE_STATE_MACHINE_H_
