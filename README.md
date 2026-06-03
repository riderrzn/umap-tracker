# umap-tracker: Android GPS Tracker

![Build Status](https://github.com/your-org/umap-tracker/actions/workflows/build-and-test.yml/badge.svg)

A high-performance GPS tracker for Android 8.0+ with offline capabilities, Kalman filtering, and MISRA C++:2023 compliance.

## Features

✅ **Real-time GPS tracking** — FusedLocationProvider with <200ms UI latency  
✅ **Kalman filter** — Smooths GPS jitter, predicts positions during signal loss  
✅ **Offline queue** — SQLite-backed message queue with exponential backoff  
✅ **Traccar integration** — Sends data to `http://map.bd62.ru:5055`  
✅ **Foreground service** — Works reliably in background  
✅ **Modern UI** — Jetpack Compose with glassmorphism design  
✅ **MISRA C++:2023** — All C++ code fully compliant (with documented deviations)  
✅ **CI/CD** — GitHub Actions with static analysis (clang-tidy, cppcheck)  

## Architecture

```
┌─────────────────────────────────────┐
│  Presentation (Kotlin/Compose)      │
│  MapScreen | StatsScreen | Settings  │
└────────────────┬────────────────────┘
                 │ JNI
┌────────────────▼────────────────────┐
│  C++ Core (MISRA C++:2023)          │
│  Kalman | State Machine | Network   │
└────────────────┬────────────────────┘
                 │
┌────────────────▼────────────────────┐
│  Data Layer (SQLite, libcurl)       │
│  Track DB | Message Queue           │
└─────────────────────────────────────┘
```

## Technology Stack

| Component | Technology |
|-----------|-----------|
| UI | Kotlin + Jetpack Compose |
| C++ Core | C++17 (MISRA C++:2023) |
| GPS | FusedLocationProvider (JNI) |
| Maps | MapLibre GL Native |
| Network | libcurl (HTTP/1.1) |
| Database | SQLite |
| Build | CMake + Gradle |

## Quick Start

### Prerequisites

- Android NDK r27+
- CMake 3.22+
- Gradle 8.0+
- clang-tidy 19+
- cppcheck 2.15+

### Clone & Setup

```bash
git clone https://github.com/your-org/umap-tracker.git
cd umap-tracker

# Initialize git hooks
git config core.hooksPath .githooks
chmod +x .githooks/*

# Create build directory
mkdir -p build
cd build
```

### Desktop Build (Linux/macOS)

```bash
cmake -GNinja -DBUILD_TESTING=ON -DENABLE_CLANG_TIDY=ON ..
cmake --build .
ctest --verbose
```

### Android Build

```bash
cd android
./gradlew assembleDebug

# Output: android/app/build/outputs/apk/debug/app-debug.apk
```

## Project Structure

```
tracker/
├── android/                    # Kotlin UI + Service
│   ├── ui/screens/            # MapScreen, StatsScreen, SettingsScreen
│   ├── ui/components/         # GlassCard, Speedometer, Compass
│   ├── viewmodel/             # MVVM ViewModels
│   ├── bridge/                # JNI bridge (Kotlin side)
│   └── service/               # ForegroundService
│
├── core/                       # C++ Core (MISRA C++:2023)
│   ├── include/umap/
│   │   ├── core/             # Kalman, State Machine, Geo algorithms
│   │   ├── data/             # Network, Message Queue, Database
│   │   ├── platform/         # Android JNI bridge
│   │   └── utils/            # Logger, Config, Result type
│   ├── src/                  # Implementation
│   ├── jni/                  # JNI bridge (C++ side)
│   ├── tests/                # Google Test unit tests
│   └── CMakeLists.txt
│
├── .github/workflows/         # CI/CD
├── .clang-tidy               # MISRA config
├── MISRA_DEVIATIONS.md       # Approved deviations
├── DEVELOPMENT_PLAN.md       # Sprint roadmap
└── README.md                 # This file
```

## MISRA C++:2023 Compliance

All C++ code follows MISRA C++:2023 with these tools:

- **clang-tidy** — enforces cppcoreguidelines (Mandatory rules)
- **cppcheck** — MISRA addon (Required rules)
- **CI/CD gate** — build fails on any violation (except documented deviations)

See [MISRA_DEVIATIONS.md](MISRA_DEVIATIONS.md) for approved exceptions.

```bash
# Check compliance locally
cd build
cmake -DENABLE_CLANG_TIDY=ON ..
cmake --build . 2>&1 | grep -i "warning\|error"
```

## Development Workflow

### Pre-commit Checks

```bash
git add .
git commit  # Runs .githooks/pre-commit (clang-tidy, tests)
```

### CI/CD Pipeline

On every `push` or `pull_request`:

1. ✅ **clang-tidy** (MISRA Mandatory rules)
2. ✅ **cppcheck** (MISRA Required rules)
3. ✅ **Unit tests** (Google Test)
4. ✅ **Android build** (Gradle)

## Performance Targets

| Metric | Target |
|--------|--------|
| GPS → UI latency | <200ms |
| HTTP upload | <500ms |
| Memory (idle) | <40MB |
| Memory (tracking) | <80MB |
| Cold start | <3s |
| Battery drain (balanced) | <5%/hour |

## Testing

```bash
cd build

# Run all unit tests
ctest --verbose

# Run specific test
ctest -R test_kalman --verbose

# With memory check
ctest --verbose -D ExperimentalMemCheck
```

## Configuration

Edit `core/include/umap/utils/config.h`:

```cpp
constexpr uint32_t MAX_QUEUE_SIZE = 50000;
constexpr uint32_t MESSAGE_TTL_HOURS = 168;
constexpr uint32_t TRACK_RETENTION_DAYS = 90;
```

## Contributing

1. Create feature branch: `git checkout -b feature/xyz`
2. Make changes (code must pass clang-tidy locally)
3. Push & create PR
4. CI/CD must pass
5. Code review + merge

## License

[Your License Here]

## Support

- 📧 Email: dev@example.com
- 🐛 Issues: [GitHub Issues](https://github.com/your-org/umap-tracker/issues)
- 📖 Docs: [Wiki](https://github.com/your-org/umap-tracker/wiki)

## Changelog

See [CHANGELOG.md](CHANGELOG.md)

---

**Last updated:** 2026-06-03  
**Version:** 1.0.0-dev
