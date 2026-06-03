# Changelog — umap-tracker

## [1.0.0-dev] — 2026-06-04

### Added — Sprint 0–5 (Core + Android UI)

**Core (C++17, MISRA-compliant)**
- Result<T> error handling without exceptions/RTTI
- Logger (Android NDK `__android_log` + stderr fallback)
- RingBuffer<T,N> — fixed-size zero-alloc FIFO
- Configuration — constexpr defaults + RuntimeConfig with env-var overrides
- Geo types — WGS84 Coordinate (Haversine distance), GpsPoint (quality score)
- Kalman filter — 1D + 3D (lat/lon/alt), velocity, bearing, outlier rejection, jitter
- State machine — 6 states (Idle/Acquiring/Tracking/Uploading/Paused/Error), 11 events
- LocationEngine — Thread-safe GPS queue, battery state tracking, JNI callbacks
- MessageQueue — SQLite-backed, exponential retry backoff, TTL cleanup
- TraccarClient — OsmAnd protocol (GET), JSON payload, batt/hdop support
- GPS Processing Pipeline — raw→Kalman→filtered→MessageQueue
- Upload Worker — MessageQueue→TraccarClient→HTTP dispatch

**Android (Kotlin, Jetpack Compose, MapLibre)**
- Theme — dark scheme with glassmorphism
- MapScreen — MapLibre GL with OpenFreeMap Liberty + OSM raster tiles
- StatsScreen — speedometer, compass, altimeter, trip details
- SettingsScreen — server URL, device ID, map style, GPS interval, upload toggles
- Components — GlassCard, Speedometer (arc 270°), Compass (rotating needle), Altimeter (vertical scale), MiniStatsBar
- Navigation — BottomBar (Map/Stats/Settings)
- ViewModels — MapViewModel, StatsViewModel, SettingsViewModel (StateFlow)
- Foreground Service — Handler-based 1 Hz processing loop
- Notification — persistent with speed/distance/status
- TrackerBridge — JNI functions for GPS/battery/error/pipeline

**Testing — 249 tests, 34 suites, 100% pass**
- Unit tests for all core components
- Integration tests (GPS→Kalman→State, Queue→Traccar)
- Stress tests (50k enqueue, 5k dequeue, 1000 pipeline points)
- Performance benchmarks (Kalman <10ms, pipeline cold start <100ms, throughput >1000 ops/sec)

**Infrastructure**
- CMake 3.22+ build (core + tests)
- GitHub Actions CI/CD
- clang-tidy + cppcheck MISRA configuration
- Pre-commit hooks
- ProGuard/R8 rules
- Gradle 8.0+ with AGP 8.2, Kotlin 1.9, Compose BOM 2023.10
- Target: Android 8.0+ (API 26), compile SDK 34

### Fixed
- Kalman filter bearing (atan2 longitude/latitude argument swap)
- Kalman filter predict() scaling by dt (was ignoring dt)
- Traccar protocol format (ms→sec timestamp, lat/lon field names, batt/hdop)
- JNI bridge altitude/hdop/battery data flow gaps
- SQLite dequeue performance (RETURNING→SELECT+UPDATE)

### Deviations (MISRA C++:2023)
- D001: JNI C-style casts (permanent)
- D002: reinterpret_cast in JNI_OnLoad and EGL (permanent)
- D003: JNI callback structs without in-class initializers (permanent)
- D004: Google Test macros (permanent)
- D005: JNI pipeline global variables (permanent)
- D006: snprintf C-style format strings (permanent)

---

## [0.0.0-scaffold] — 2026-06-03

- Initial project structure
- CMake + Gradle scaffold
- CI/CD pipeline
- MISRA deviations document
- Placeholder headers
