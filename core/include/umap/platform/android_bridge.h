// SPDX-License-Identifier: Apache-2.0
// MISRA C++:2023 compliant
// Android JNI bridge for GPS and battery events

#ifndef UMAP_PLATFORM_ANDROID_BRIDGE_H_
#define UMAP_PLATFORM_ANDROID_BRIDGE_H_

#include <mutex>
#include <condition_variable>
#include <optional>
#include <queue>
#include <cstdint>

#include "umap/core/geo_types.h"
#include "umap/platform/jni_types.h"
#include "umap/utils/result.h"

namespace umap::platform {

// Thread-safe queue for passing location data from JNI to C++
template<typename T, uint32_t MaxSize = 1000U>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() noexcept = default;

    // Delete copy operations (not thread-safe to copy)
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Move operations also deleted for simplicity
    ThreadSafeQueue(ThreadSafeQueue&&) = delete;
    ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

    ~ThreadSafeQueue() noexcept = default;

    // Add item to queue (thread-safe)
    // Returns false if queue is full
    [[nodiscard]] bool enqueue(const T& item) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (queue_.size() >= MaxSize) {
            return false;  // Queue full, overflow
        }
        
        queue_.push(item);
        cv_.notify_one();
        return true;
    }

    // Try to get item with timeout (thread-safe)
    // Returns nullopt if queue empty or timeout
    [[nodiscard]] std::optional<T> dequeue(uint32_t timeout_ms = 0U) noexcept {
        std::unique_lock<std::mutex> lock(mutex_);

        if (timeout_ms == 0U) {
            if (queue_.empty()) {
                return std::nullopt;
            }
        } else {
            // std::chrono required for condition_variable wait_for
            if (!cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                             [this]() { return !queue_.empty(); })) {
                return std::nullopt;
            }
        }

        if (queue_.empty()) {
            return std::nullopt;
        }

        T item = queue_.front();
        queue_.pop();
        return item;
    }

    // Check if queue is empty
    [[nodiscard]] bool empty() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Get queue size
    [[nodiscard]] uint32_t size() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return static_cast<uint32_t>(queue_.size());
    }

    // Clear queue
    void clear() noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
};

// LocationEngine: Manages GPS input from Android via JNI
class LocationEngine {
public:
    LocationEngine() noexcept = default;

    // Delete copy operations
    LocationEngine(const LocationEngine&) = delete;
    LocationEngine& operator=(const LocationEngine&) = delete;

    // Delete move operations (singleton pattern)
    LocationEngine(LocationEngine&&) = delete;
    LocationEngine& operator=(LocationEngine&&) = delete;

    ~LocationEngine() noexcept = default;

    // Get singleton instance
    static LocationEngine& instance() noexcept {
        static LocationEngine instance_obj;
        return instance_obj;
    }

    // Register location callback from JNI
    // Returns error if already registered
    [[nodiscard]] utils::Result<void> register_location_callback(
        LocationCallback callback) noexcept {
        if (callback == nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        std::unique_lock<std::mutex> lock(callbacks_mutex_);
        if (location_callback_ != nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        location_callback_ = callback;
        return utils::Result<void>();
    }

    // Register battery callback from JNI
    [[nodiscard]] utils::Result<void> register_battery_callback(
        BatteryCallback callback) noexcept {
        if (callback == nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        std::unique_lock<std::mutex> lock(callbacks_mutex_);
        if (battery_callback_ != nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        battery_callback_ = callback;
        return utils::Result<void>();
    }

    // Register error callback from JNI
    [[nodiscard]] utils::Result<void> register_error_callback(
        ErrorCallback callback) noexcept {
        if (callback == nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        std::unique_lock<std::mutex> lock(callbacks_mutex_);
        if (error_callback_ != nullptr) {
            return utils::Unexpected(utils::ErrorCode::InvalidConfig);
        }

        error_callback_ = callback;
        return utils::Result<void>();
    }

    // Unregister all callbacks
    void unregister_callbacks() noexcept {
        std::unique_lock<std::mutex> lock(callbacks_mutex_);
        location_callback_ = nullptr;
        battery_callback_ = nullptr;
        error_callback_ = nullptr;
    }

    // Add location point to queue (called from JNI thread)
    [[nodiscard]] bool enqueue_location(
        double latitude, double longitude, float accuracy_m,
        double altitude_m, double hdop, uint32_t timestamp_ms) noexcept {
        core::GpsPoint point;
        point.location = core::Coordinate(latitude, longitude);
        point.accuracy_m = static_cast<double>(accuracy_m);
        point.altitude_m = altitude_m;
        point.hdop = hdop;
        point.timestamp_ms = timestamp_ms;
        point.battery_percent = last_battery_percent_;
        point.is_charging = last_is_charging_;

        return location_queue_.enqueue(point);
    }

    // Get next location from queue with optional timeout
    [[nodiscard]] std::optional<core::GpsPoint> get_location(
        uint32_t timeout_ms = 0U) noexcept {
        return location_queue_.dequeue(timeout_ms);
    }

    // Get current queued location count
    [[nodiscard]] uint32_t location_queue_size() const noexcept {
        return location_queue_.size();
    }

    // Clear location queue
    void clear_location_queue() noexcept {
        location_queue_.clear();
    }

    // Static JNI callback adapters (forward to singleton)
    static void on_location_received(double lat, double lon, float accuracy,
                                     double altitude, double hdop,
                                     uint32_t timestamp_ms) noexcept {
        LocationEngine& engine = instance();
        
        // Store in queue for later processing
        // Intentional ignore of overflow result (not critical for queue overflow)
        engine.enqueue_location(lat, lon, accuracy, altitude, hdop, timestamp_ms);

        // Invoke registered callback if set
        std::unique_lock<std::mutex> lock(engine.callbacks_mutex_);
        if (engine.location_callback_ != nullptr) {
            engine.location_callback_(lat, lon, accuracy, timestamp_ms);
        }
    }

    static void on_battery_changed(uint32_t battery_percent,
                                    bool is_charging) noexcept {
        LocationEngine& engine = instance();

        std::unique_lock<std::mutex> lock(engine.callbacks_mutex_);
        engine.last_battery_percent_ = battery_percent;
        engine.last_is_charging_ = is_charging;
        if (engine.battery_callback_ != nullptr) {
            engine.battery_callback_(battery_percent, is_charging);
        }
    }

    static void on_error_occurred(int error_code, const char* message) noexcept {
        LocationEngine& engine = instance();
        
        std::unique_lock<std::mutex> lock(engine.callbacks_mutex_);
        if (engine.error_callback_ != nullptr) {
            engine.error_callback_(error_code, message);
        }
    }

    // Check if GPS data is available in the queue
    [[nodiscard]] static bool is_gps_available() noexcept;

    // Initialize the location engine (clear state, unregister callbacks)
    static void initialize() noexcept;

    // Shutdown the location engine (clean up)
    static void shutdown() noexcept;

private:
    // Callbacks registered by JNI layer
    LocationCallback location_callback_ = nullptr;
    BatteryCallback battery_callback_ = nullptr;
    ErrorCallback error_callback_ = nullptr;

    // Queue for storing GPS locations received from JNI
    ThreadSafeQueue<core::GpsPoint, 1000U> location_queue_;

    // Protect callback pointers and battery state
    mutable std::mutex callbacks_mutex_;

    // Last known battery state (injected into GpsPoint on enqueue)
    uint32_t last_battery_percent_ = 0U;
    bool last_is_charging_ = false;
};

}  // namespace umap::platform

#endif  // UMAP_PLATFORM_ANDROID_BRIDGE_H_
