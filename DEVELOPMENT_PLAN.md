# План разработки Android GPS Tracker

**Дата:** 2026-06-03  
**Версия:** 1.0  
**Статус:** В плане  

---

## 📋 Обзор проекта

- **Язык:** Kotlin (UI) + C++17 (Core, MISRA C++:2023)
- **Целевая платформа:** Android 8.0–14 (API 26–34)
- **Основные компоненты:** GPS трекинг, Kalman Filter, Traccar интеграция, Foreground Service, Offline Queue
- **Оценка:** ~10–12 недель (5 разработчиков: 2 C++, 2 Kotlin, 1 DevOps)

---

## 🚀 Sprint 0: Инициализация проекта (1 неделя)

### Цели
- Установить CI/CD pipeline
- Настроить MISRA compliance check
- Создать базовую структуру проекта
- Настроить local development environment

### Задачи

| # | Задача | Ответ. | Приоритет | Статус |
|---|--------|--------|-----------|--------|
| S0-1 | Инициализировать Git репозиторий + gitignore | DevOps | MUST | ❌ |
| S0-2 | Настроить CMakeLists.txt (корневой + core/) | C++ lead | MUST | ❌ |
| S0-3 | Создать `.clang-tidy` конфиг + pre-commit hook | C++ lead | MUST | ❌ |
| S0-4 | GitHub Actions workflow: build + test | DevOps | MUST | ❌ |
| S0-5 | Настроить Android NDK r27+, Gradle, Kotlin | Kotlin lead | MUST | ❌ |
| S0-6 | Создать MISRA_DEVIATIONS.md (шаблон) | C++ lead | MUST | ❌ |
| S0-7 | Scaffolding папок: android/, core/, tests/ | C++ lead | MUST | ❌ |
| S0-8 | README.md + setup инструкции | DevOps | SHOULD | ❌ |

### Deliverables
- ✅ Репозиторий готов
- ✅ CI/CD зелёный (пусть даже с пустым кодом)
- ✅ Pre-commit hook срабатывает
- ✅ Документация по setup

---

## 📦 Sprint 1: Основная инфраструктура (1.5 недели)

### Цели
- Создать базовые типы и утилиты (MISRA compliant)
- Настроить JNI bridge
- Настроить логирование

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S1-1 | Реализовать `result.h` (tl::expected обёртка) | `include/umap/utils/result.h` | C++ dev1 | 3h | ❌ |
| S1-2 | Реализовать logger (Android NDK __android_log) | `src/utils/logger.cpp` | C++ dev1 | 4h | ❌ |
| S1-3 | Создать geo_types.h (Coordinate, LatLon, GpsPoint) | `include/umap/core/geo_types.h` | C++ dev1 | 3h | ❌ |
| S1-4 | Реализовать RingBuffer<T, N> для GPS points | `include/umap/utils/ring_buffer.h` | C++ dev2 | 3h | ❌ |
| S1-5 | Создать config.h/cpp (MISRA-compliant settings) | `include/umap/utils/config.h` | C++ dev2 | 4h | ❌ |
| S1-6 | Настроить JNI bridge скелет (headers) | `include/umap/platform/jni_types.h` | C++ dev2 | 3h | ❌ |
| S1-7 | Реализовать jni_bridge.cpp (базовые callback'и) | `core/jni/jni_bridge.cpp` | C++ dev2 | 5h | ❌ |
| S1-8 | Unit tests для result, logger, ring_buffer | `core/tests/test_*.cpp` | C++ dev1 | 4h | ❌ |

### Deliverables
- ✅ All utils tested (coverage >90%)
- ✅ JNI calls работают (echo test)
- ✅ CI/CD берёт clang-tidy для нового кода
- ✅ Log файлы записываются в Android logcat

---

## 🧮 Sprint 2: Ядро — Kalman Filter & State Machine (2 недели)

### Цели
- Реализовать Kalman Filter для GPS данных
- Реализовать State Machine (IDLE → ACQUIRING → TRACKING → UPLOADING)
- Написать comprehensive тесты

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S2-1 | Реализовать KalmanFilter::init() + matrices | `src/core/kalman_filter.cpp` | C++ dev1 | 8h | ❌ |
| S2-2 | Реализовать KalmanFilter::update() (predict + correct) | `src/core/kalman_filter.cpp` | C++ dev1 | 6h | ❌ |
| S2-3 | Тест: outlier rejection (100 km/h spike) | `tests/test_kalman.cpp` | C++ dev1 | 3h | ❌ |
| S2-4 | Тест: smooth jitter (GPS noise) | `tests/test_kalman.cpp` | C++ dev1 | 3h | ❌ |
| S2-5 | Реализовать StateMachine (enum states + transitions) | `src/core/state_machine.cpp` | C++ dev2 | 5h | ❌ |
| S2-6 | Реализовать LocationEngine stub (GPS input) | `src/platform/android_bridge.cpp` | C++ dev2 | 4h | ❌ |
| S2-7 | Тесты State Machine (all transitions) | `tests/test_state_machine.cpp` | C++ dev2 | 4h | ❌ |
| S2-8 | Integration test: GPS → Kalman → State | `tests/integration_kalman_state.cpp` | C++ dev1 | 5h | ❌ |
| S2-9 | Benchmark Kalman (< 10ms per update) | `tests/bench_kalman.cpp` | C++ dev2 | 3h | ❌ |

### Deliverables
- ✅ KalmanFilter: 95%+ coverage, <10ms per point
- ✅ StateMachine: 100% coverage, все transitions протестированы
- ✅ CI/CD: тесты проходят на каждый коммит
- ✅ MISRA: clang-tidy 0 violations (除了 deviations)

---

## 🌐 Sprint 3: Network & Message Queue (1.5 недели)

### Цели
- Реализовать TraccarClient (HTTP GET + POST)
- Реализовать Message Queue (SQLite-backed)
- Retry policy: exponential backoff

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S3-1 | Настроить libcurl статический build (CMake) | `CMakeLists.txt` | C++ dev1 | 3h | ❌ |
| S3-2 | Реализовать TraccarClient::buildUrl() (GET) | `src/data/traccar_client.cpp` | C++ dev1 | 4h | ❌ |
| S3-3 | Реализовать TraccarClient::buildJsonPayload() (POST) | `src/data/traccar_client.cpp` | C++ dev1 | 4h | ❌ |
| S3-4 | Реализовать TraccarClient::send() (curl multi) | `src/data/traccar_client.cpp` | C++ dev1 | 6h | ❌ |
| S3-5 | Реализовать MessageQueue schema (SQLite) | `src/data/message_queue.cpp` | C++ dev2 | 4h | ❌ |
| S3-6 | Реализовать MessageQueue::enqueue() | `src/data/message_queue.cpp` | C++ dev2 | 3h | ❌ |
| S3-7 | Реализовать MessageQueue::dequeue() + retry logic | `src/data/message_queue.cpp` | C++ dev2 | 5h | ❌ |
| S3-8 | Реализовать TTL cleanup (7 дней) | `src/data/message_queue.cpp` | C++ dev2 | 3h | ❌ |
| S3-9 | Тест: MessageQueue overflow (> 50000) | `tests/test_message_queue.cpp` | C++ dev2 | 3h | ❌ |
| S3-10 | Тест: Retry exponential backoff | `tests/test_message_queue.cpp` | C++ dev2 | 3h | ❌ |
| S3-11 | Mock test: TraccarClient HTTP responses | `tests/test_traccar_client.cpp` | C++ dev1 | 4h | ❌ |
| S3-12 | Integration: Queue → Traccar mock server | `tests/integration_queue_traccar.cpp` | C++ dev2 | 5h | ❌ |

### Deliverables
- ✅ TraccarClient sends to map.bd62.ru:5055
- ✅ MessageQueue: 95%+ coverage, retry works
- ✅ Queue survives 50000 messages + memory stable
- ✅ Network errors gracefully degrade to offline mode

---

## 🎨 Sprint 4: UI — Kotlin + Jetpack Compose (2 недели)

### Цели
- Реализовать MapScreen с MapLibre GL
- Реализовать StatsScreen
- Реализовать SettingsScreen
- Взаимодействие с C++ Core через JNI

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S4-1 | Dependency setup (MapLibre, Compose, Room) | `build.gradle` | Kotlin dev1 | 2h | ❌ |
| S4-2 | Theme.kt + GlassCard component (glassmorphism) | `ui/theme/Theme.kt` + `ui/components/GlassCard.kt` | Kotlin dev1 | 4h | ❌ |
| S4-3 | MapScreen layout + MapLibre integration | `ui/screens/MapScreen.kt` | Kotlin dev1 | 6h | ❌ |
| S4-4 | Speedometer component (circular gauge) | `ui/components/Speedometer.kt` | Kotlin dev2 | 3h | ❌ |
| S4-5 | Compass component (rotating needle) | `ui/components/CompassWidget.kt` | Kotlin dev2 | 3h | ❌ |
| S4-6 | Altimeter component (вертикальная шкала) | `ui/components/AltimeterWidget.kt` | Kotlin dev2 | 2h | ❌ |
| S4-7 | MiniStatsBar (дистанция, время, REC/STOP) | `ui/components/MiniStatsBar.kt` | Kotlin dev2 | 3h | ❌ |
| S4-8 | StatsScreen with chart library (area chart) | `ui/screens/StatsScreen.kt` | Kotlin dev1 | 6h | ❌ |
| S4-9 | SettingsScreen (form, sliders, radio buttons) | `ui/screens/SettingsScreen.kt` | Kotlin dev1 | 5h | ❌ |
| S4-10 | BottomNavigationBar (Map | Stats | Settings) | `ui/navigation/Navigation.kt` | Kotlin dev2 | 2h | ❌ |
| S4-11 | MapViewModel + StateFlow integration | `viewmodel/MapViewModel.kt` | Kotlin dev1 | 4h | ❌ |
| S4-12 | StatsViewModel (Room queries + UI state) | `viewmodel/StatsViewModel.kt` | Kotlin dev1 | 3h | ❌ |
| S4-13 | SettingsViewModel (SharedPreferences + validation) | `viewmodel/SettingsViewModel.kt` | Kotlin dev2 | 3h | ❌ |
| S4-14 | TrackerBridge.kt (JNI extern fun) | `bridge/TrackerBridge.kt` | Kotlin dev2 | 3h | ❌ |

### Deliverables
- ✅ MapScreen показывает текущую позицию + трек
- ✅ Speedometer, Compass, Altimeter работают в реальном времени
- ✅ StatsScreen показывает историю + графики
- ✅ SettingsScreen работает с предпочтениями
- ✅ BottomNav работает плавно (no lag)

---

## 🔗 Sprint 5: Integration & JNI Bridge (1.5 недели)

### Цели
- Интегрировать C++ Core с Kotlin UI
- Реализовать Foreground Service
- Реализовать LocationEngine (JNI callback)
- Thread synchronization (mutex, condition_variable)

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S5-1 | Реализовать LocationEngine с JNI callback | `src/platform/android_bridge.cpp` | C++ dev1 | 6h | ❌ |
| S5-2 | Реализовать Thread-safe queue (std::mutex) | `include/umap/core/thread_safe_queue.h` | C++ dev1 | 4h | ❌ |
| S5-3 | Реализовать GPS Thread (получает FusedLocation) | `src/platform/gps_thread.cpp` | C++ dev1 | 5h | ❌ |
| S5-4 | Реализовать Upload Thread (отправляет в Traccar) | `src/platform/upload_thread.cpp` | C++ dev2 | 5h | ❌ |
| S5-5 | Реализовать Main Service (координирует потоки) | `android/service/TrackerForegroundService.kt` | Kotlin dev1 | 6h | ❌ |
| S5-6 | Реализовать Notification (persistent + actions) | `android/service/NotificationHelper.kt` | Kotlin dev2 | 3h | ❌ |
| S5-7 | Интеграция C++ сигналов в Kotlin StateFlow | `bridge/TrackerBridge.kt` | Kotlin dev2 | 4h | ❌ |
| S5-8 | Тест: Thread safety (Thread Sanitizer) | `tests/test_thread_safety.cpp` | C++ dev1 | 4h | ❌ |
| S5-9 | Тест: GPS → Upload complete flow | `tests/integration_full_flow.cpp` | C++ dev2 | 5h | ❌ |
| S5-10 | Тест: UI updates при получении GPS points | (Kotlin integration test) | Kotlin dev1 | 4h | ❌ |

### Deliverables
- ✅ Foreground Service запускается и работает в фоне
- ✅ GPS данные поступают в UI в реальном времени (<200ms latency)
- ✅ Данные отправляются на Traccar сервер
- ✅ Thread Sanitizer: 0 race conditions
- ✅ Notification актуальна (показывает скорость, статус)

---

## 🧪 Sprint 6: Testing & Performance Optimization (1.5 недели)

### Цели
- Достичь >90% code coverage
- Оптимизировать память и CPU
- Stress testing

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S6-1 | Увеличить coverage для core/ до 95%+ | `tests/` | C++ dev1 | 6h | ❌ |
| S6-2 | Увеличить coverage для data/ до 90%+ | `tests/` | C++ dev2 | 6h | ❌ |
| S6-3 | Тест: Queue recovery (50000 points) | `tests/test_queue_recovery.cpp` | C++ dev2 | 4h | ❌ |
| S6-4 | Тест: Memory profiling (idle < 40MB, tracking < 80MB) | (using Android Profiler) | C++ dev1 | 4h | ❌ |
| S6-5 | Тест: Foreground Service restart + recovery | `tests/test_service_restart.cpp` | Kotlin dev1 | 4h | ❌ |
| S6-6 | Тест: Battery drain (balanced mode < 5%/h) | (field test) | DevOps | 4h | ❌ |
| S6-7 | Benchmark: Cold start < 3s | (using speedometer) | C++ dev1 | 2h | ❌ |
| S6-8 | Optimize hot paths (profiling + flame graphs) | `src/` | C++ dev1 | 6h | ❌ |
| S6-9 | Тест: Doze mode recovery + Battery Optimization | `tests/test_doze_recovery.cpp` | Kotlin dev2 | 4h | ❌ |
| S6-10 | Load test: 1000 msg/sec through queue | (synthetic) | C++ dev2 | 3h | ❌ |

### Deliverables
- ✅ Code coverage: >90%
- ✅ Memory: idle <40MB, tracking <80MB
- ✅ CPU: KalmanFilter <10ms, upload <500ms
- ✅ Cold start: <3s
- ✅ Battery: <5%/h (balanced)
- ✅ Service survives Doze mode

---

## ✅ Sprint 7: Polish, Documentation & CI/CD (1 неделя)

### Цели
- MISRA compliance checklist
- Documentation (code comments, API docs)
- CI/CD pipeline finalization
- Release-ready build

### Задачи

| # | Задача | Файлы | Ответ. | Est. | Статус |
|---|--------|-------|--------|------|--------|
| S7-1 | MISRA C++ compliance audit | `MISRA_DEVIATIONS.md` | C++ lead | 6h | ❌ |
| S7-2 | Run final clang-tidy + cppcheck | CI pipeline | DevOps | 2h | ❌ |
| S7-3 | API documentation (Doxygen) | `include/umap/` | C++ dev1 | 4h | ❌ |
| S7-4 | README: build instructions + API usage | `README.md` | DevOps | 3h | ❌ |
| S7-5 | Kotlin code cleanup (ktlint, detekt) | `android/` | Kotlin dev1 | 3h | ❌ |
| S7-6 | ProGuard/R8 minify config для APK | `build.gradle` | Kotlin dev2 | 2h | ❌ |
| S7-7 | Release build (signed APK) | CI pipeline | DevOps | 3h | ❌ |
| S7-8 | Changelog + version bump (v1.0.0) | `CHANGELOG.md` | DevOps | 2h | ❌ |
| S7-9 | Final integration test on real device | (Android 14 device) | QA | 6h | ❌ |
| S7-10 | Known issues document + future work | `KNOWN_ISSUES.md` | Team | 2h | ❌ |

### Deliverables
- ✅ MISRA C++:2023 fully compliant (or deviations documented)
- ✅ Release APK signed + optimized
- ✅ Full documentation
- ✅ CI/CD green on all commits
- ✅ Tested on Android 8.0, 11, 14 real devices

---

## 📊 Временная шкала

```
Week 1   [Sprint 0]  — Инициализация (CI/CD, структура)
Week 2   [Sprint 1]  — Utils & JNI bridge
Week 3   [Sprint 2a] — Kalman Filter
Week 4   [Sprint 2b] — State Machine & tests
Week 5   [Sprint 3]  — Network & Message Queue
Week 6   [Sprint 4a] — UI design & components
Week 7   [Sprint 4b] — ViewModels & integration
Week 8   [Sprint 5]  — Service, GPS Thread, Upload Thread
Week 9   [Sprint 6a] — Testing & optimization
Week 10  [Sprint 6b] — Performance tuning
Week 11  [Sprint 7a] — Polish & MISRA audit
Week 12  [Sprint 7b] — Release & documentation
```

**Итого: ~12 недель, 5–6 разработчиков (parallelizable)**

---

## 🎯 Key Milestones

| Milestone | Target Week | Gate |
|-----------|------------|------|
| M1: CI/CD готов | Week 1 | Pipeline зелёный |
| M2: Core infrastructure ready | Week 2 | Тесты проходят |
| M3: Kalman + StateMachine | Week 4 | 95%+ coverage |
| M4: Network integration | Week 5 | Данные идут в Traccar |
| M5: UI готов | Week 7 | Все экраны отрисовываются |
| M6: Full integration | Week 8 | Foreground Service работает |
| M7: Release-ready | Week 12 | APK signed + documented |

---

## 🔴 Risks & Mitigations

| Risk | Вероятность | Impact | Mitigation |
|------|-----------|--------|------------|
| JNI complexity | Medium | High | Early spike (Sprint 1–2) |
| Kalman Filter precision | Medium | High | Extensive testing + reference papers |
| Thread safety bugs | Medium | High | Thread Sanitizer на каждый build |
| Android NDK API mismatch | Low | Medium | Test на реальных устройствах |
| Battery drain > 5%/h | Medium | High | Profiling + Doze mode handling |
| MISRA compliance overhead | Low | Medium | Static tools early (Sprint 0) |

---

## 📝 Notes

- **Team rotation:** Возможны перестановки между C++ и Kotlin ребятами для лучшего understanding architecture
- **Daily standup:** 15 min, утром по UTC
- **Sprint reviews:** Конец каждой недели, демо на реальном устройстве
- **Retro:** Каждый Sprint — learn, improve, adjust план
- **Testing:** TDD где возможно (особенно C++ core)
- **Documentation:** Inline comments на каждый нетривиальный кусок кода

---

**Документ версии:** 1.0  
**Последнее обновление:** 2026-06-03  
**Статус:** Готов к утверждению
