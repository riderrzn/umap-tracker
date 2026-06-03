// SPDX-License-Identifier: Apache-2.0
// Tests for location state machine

#include <gtest/gtest.h>

#include "umap/core/state_machine.h"

using namespace umap::core;

// ============================================================================
// State Conversion Tests
// ============================================================================

TEST(StateConversionTest, StateToString) {
    EXPECT_STREQ(state_to_string(LocationState::Idle), "Idle");
    EXPECT_STREQ(state_to_string(LocationState::Acquiring), "Acquiring");
    EXPECT_STREQ(state_to_string(LocationState::Tracking), "Tracking");
    EXPECT_STREQ(state_to_string(LocationState::Uploading), "Uploading");
    EXPECT_STREQ(state_to_string(LocationState::Paused), "Paused");
    EXPECT_STREQ(state_to_string(LocationState::Error), "Error");
}

// ============================================================================
// State Machine Tests
// ============================================================================

class LocationStateMachineTest : public ::testing::Test {
protected:
    LocationStateMachine machine;
};

TEST_F(LocationStateMachineTest, InitialState) {
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
    EXPECT_STREQ(machine.get_state_name(), "Idle");
}

TEST_F(LocationStateMachineTest, StartTracking) {
    auto result = machine.process_event(StateEvent::StartTracking);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);
}

TEST_F(LocationStateMachineTest, AcquireLocation) {
    machine.process_event(StateEvent::StartTracking);
    auto result = machine.process_event(StateEvent::LocationAcquired);

    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(LocationStateMachineTest, StopTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    auto result = machine.process_event(StateEvent::StopTracking);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(LocationStateMachineTest, InvalidTransitionRejected) {
    auto result = machine.process_event(StateEvent::LocationAcquired);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::InvalidConfig);
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(LocationStateMachineTest, AcquiringToTracking) {
    machine.process_event(StateEvent::StartTracking);
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);

    machine.process_event(StateEvent::LocationAcquired);
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(LocationStateMachineTest, TrackingToUploading) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    auto result = machine.process_event(StateEvent::UploadStarted);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Uploading);
}

TEST_F(LocationStateMachineTest, UploadingBackToTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::UploadStarted);

    auto result = machine.process_event(StateEvent::UploadCompleted);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(LocationStateMachineTest, UploadFailureBackToTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::UploadStarted);

    auto result = machine.process_event(StateEvent::UploadFailed);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(LocationStateMachineTest, PauseTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    auto result = machine.process_event(StateEvent::PauseTracking);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Paused);
}

TEST_F(LocationStateMachineTest, ResumeTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::PauseTracking);

    auto result = machine.process_event(StateEvent::ResumeTracking);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(LocationStateMachineTest, ErrorInTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    auto result = machine.process_event(StateEvent::ErrorOccurred);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Error);
}

TEST_F(LocationStateMachineTest, RecoverFromError) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::ErrorOccurred);

    auto result = machine.process_event(StateEvent::ErrorRecovered);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(LocationStateMachineTest, ErrorStopsTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::ErrorOccurred);

    auto result = machine.process_event(StateEvent::StopTracking);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(LocationStateMachineTest, LocationLostWhileTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    auto result = machine.process_event(StateEvent::LocationLost);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);
}

TEST_F(LocationStateMachineTest, CanTransitionCheck) {
    EXPECT_TRUE(machine.can_transition(LocationState::Idle, StateEvent::StartTracking));
    EXPECT_FALSE(machine.can_transition(LocationState::Idle, StateEvent::LocationAcquired));
}

TEST_F(LocationStateMachineTest, ResetToIdle) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    machine.reset();
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

// ============================================================================
// State Machine Callbacks
// ============================================================================

class StateMachineCallbackTest : public ::testing::Test {
protected:
    LocationStateMachine machine;

    int state_change_count = 0;
    LocationState last_from_state = LocationState::Idle;
    LocationState last_to_state = LocationState::Idle;
};

TEST_F(StateMachineCallbackTest, RegisterStateCallback) {
    auto callback = [this](LocationState from, LocationState to) {
        state_change_count++;
        last_from_state = from;
        last_to_state = to;
    };

    auto result = machine.register_state_callback(callback);
    EXPECT_TRUE(result.has_value());
}

TEST_F(StateMachineCallbackTest, RejectNullptrCallback) {
    auto result = machine.register_state_callback(nullptr);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), umap::utils::ErrorCode::InvalidConfig);
}

TEST_F(StateMachineCallbackTest, StateChangeTriggersCallback) {
    int change_count = 0;
    auto callback = [&change_count](LocationState, LocationState) {
        change_count++;
    };

    machine.register_state_callback(callback);
    machine.process_event(StateEvent::StartTracking);

    EXPECT_EQ(change_count, 1);
}

TEST_F(StateMachineCallbackTest, CallbackReceivesCorrectStates) {
    auto callback = [this](LocationState from, LocationState to) {
        last_from_state = from;
        last_to_state = to;
    };

    machine.register_state_callback(callback);
    machine.process_event(StateEvent::StartTracking);

    EXPECT_EQ(last_from_state, LocationState::Idle);
    EXPECT_EQ(last_to_state, LocationState::Acquiring);
}

TEST_F(StateMachineCallbackTest, MultipleStateChanges) {
    int change_count = 0;
    auto callback = [&change_count](LocationState, LocationState) {
        change_count++;
    };

    machine.register_state_callback(callback);

    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::PauseTracking);

    EXPECT_EQ(change_count, 3);
}

TEST_F(StateMachineCallbackTest, InvalidTransitionNoCallback) {
    int change_count = 0;
    auto callback = [&change_count](LocationState, LocationState) {
        change_count++;
    };

    machine.register_state_callback(callback);

    // Try invalid transition
    machine.process_event(StateEvent::LocationAcquired);

    EXPECT_EQ(change_count, 0);  // No callback for invalid transition
}

TEST_F(StateMachineCallbackTest, UnregisterCallbacks) {
    int change_count = 0;
    auto callback = [&change_count](LocationState, LocationState) {
        change_count++;
    };

    machine.register_state_callback(callback);
    machine.process_event(StateEvent::StartTracking);

    EXPECT_EQ(change_count, 1);

    machine.unregister_callbacks();

    machine.reset();
    machine.process_event(StateEvent::StartTracking);

    EXPECT_EQ(change_count, 1);  // No new callbacks
}

// ============================================================================
// Error Callback Tests
// ============================================================================

class ErrorCallbackTest : public ::testing::Test {
protected:
    LocationStateMachine machine;
};

TEST_F(ErrorCallbackTest, RegisterErrorCallback) {
    auto callback = [](int, const char*) {};
    auto result = machine.register_error_callback(callback);
    EXPECT_TRUE(result.has_value());
}

TEST_F(ErrorCallbackTest, RejectNullptrErrorCallback) {
    auto result = machine.register_error_callback(nullptr);
    EXPECT_FALSE(result.has_value());
}

// ============================================================================
// Complex Workflow Tests
// ============================================================================

class StateMachineWorkflowTest : public ::testing::Test {
protected:
    LocationStateMachine machine;
};

TEST_F(StateMachineWorkflowTest, TypicalTrackingSession) {
    // Start tracking
    EXPECT_TRUE(machine.process_event(StateEvent::StartTracking).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);

    // Acquire location
    EXPECT_TRUE(machine.process_event(StateEvent::LocationAcquired).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);

    // Upload data
    EXPECT_TRUE(machine.process_event(StateEvent::UploadStarted).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Uploading);

    // Upload completes
    EXPECT_TRUE(machine.process_event(StateEvent::UploadCompleted).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);

    // Stop tracking
    EXPECT_TRUE(machine.process_event(StateEvent::StopTracking).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(StateMachineWorkflowTest, PausedTracking) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    // Pause
    EXPECT_TRUE(machine.process_event(StateEvent::PauseTracking).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Paused);

    // Stop from pause
    EXPECT_TRUE(machine.process_event(StateEvent::StopTracking).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);
}

TEST_F(StateMachineWorkflowTest, ErrorRecovery) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    // Error occurs during tracking
    EXPECT_TRUE(machine.process_event(StateEvent::ErrorOccurred).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Error);

    // Recover and restart
    EXPECT_TRUE(machine.process_event(StateEvent::ErrorRecovered).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Idle);

    // Can restart tracking after error
    EXPECT_TRUE(machine.process_event(StateEvent::StartTracking).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);
}

TEST_F(StateMachineWorkflowTest, UploadFailureRetry) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);
    machine.process_event(StateEvent::UploadStarted);

    // Upload fails
    EXPECT_TRUE(machine.process_event(StateEvent::UploadFailed).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);

    // Can retry
    EXPECT_TRUE(machine.process_event(StateEvent::UploadStarted).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Uploading);
}

TEST_F(StateMachineWorkflowTest, LocationLostAndReacquired) {
    machine.process_event(StateEvent::StartTracking);
    machine.process_event(StateEvent::LocationAcquired);

    // Lose signal
    EXPECT_TRUE(machine.process_event(StateEvent::LocationLost).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Acquiring);

    // Reacquire
    EXPECT_TRUE(machine.process_event(StateEvent::LocationAcquired).has_value());
    EXPECT_EQ(machine.get_state(), LocationState::Tracking);
}

TEST_F(StateMachineWorkflowTest, StateNameConsistency) {
    machine.process_event(StateEvent::StartTracking);
    EXPECT_STREQ(machine.get_state_name(), "Acquiring");

    machine.process_event(StateEvent::LocationAcquired);
    EXPECT_STREQ(machine.get_state_name(), "Tracking");

    machine.process_event(StateEvent::PauseTracking);
    EXPECT_STREQ(machine.get_state_name(), "Paused");
}

// ============================================================================
// All State Transitions Matrix
// ============================================================================

TEST(StateTransitionMatrixTest, AllTransitionsCovered) {
    LocationStateMachine machine;

    // Test Idle transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Idle, StateEvent::StartTracking));
    EXPECT_FALSE(machine.can_transition(LocationState::Idle, StateEvent::StopTracking));

    // Test Acquiring transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Acquiring, StateEvent::LocationAcquired));
    EXPECT_TRUE(machine.can_transition(LocationState::Acquiring, StateEvent::StopTracking));

    // Test Tracking transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Tracking, StateEvent::LocationLost));
    EXPECT_TRUE(machine.can_transition(LocationState::Tracking, StateEvent::UploadStarted));
    EXPECT_TRUE(machine.can_transition(LocationState::Tracking, StateEvent::PauseTracking));

    // Test Uploading transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Uploading, StateEvent::UploadCompleted));
    EXPECT_TRUE(machine.can_transition(LocationState::Uploading, StateEvent::UploadFailed));

    // Test Paused transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Paused, StateEvent::ResumeTracking));
    EXPECT_TRUE(machine.can_transition(LocationState::Paused, StateEvent::StopTracking));

    // Test Error transitions
    EXPECT_TRUE(machine.can_transition(LocationState::Error, StateEvent::ErrorRecovered));
}

// Test event_to_string for all events
TEST(EventStringTest, AllEventsHaveString) {
    EXPECT_STREQ(event_to_string(StateEvent::StartTracking), "StartTracking");
    EXPECT_STREQ(event_to_string(StateEvent::StopTracking), "StopTracking");
    EXPECT_STREQ(event_to_string(StateEvent::LocationAcquired), "LocationAcquired");
    EXPECT_STREQ(event_to_string(StateEvent::LocationLost), "LocationLost");
    EXPECT_STREQ(event_to_string(StateEvent::UploadStarted), "UploadStarted");
    EXPECT_STREQ(event_to_string(StateEvent::UploadCompleted), "UploadCompleted");
    EXPECT_STREQ(event_to_string(StateEvent::UploadFailed), "UploadFailed");
    EXPECT_STREQ(event_to_string(StateEvent::PauseTracking), "PauseTracking");
    EXPECT_STREQ(event_to_string(StateEvent::ResumeTracking), "ResumeTracking");
    EXPECT_STREQ(event_to_string(StateEvent::ErrorOccurred), "ErrorOccurred");
    EXPECT_STREQ(event_to_string(StateEvent::ErrorRecovered), "ErrorRecovered");
}

TEST(EventStringTest, UnknownEvent) {
    auto unknown = static_cast<StateEvent>(255U);
    EXPECT_STREQ(event_to_string(unknown), "Unknown");
}
