# Техническое задание: Android GPS Трекер (C++ / MISRA C++:2023)

> **Дата:** 2026-06-03  
> **Сервер:** `http://map.bd62.ru:5055` (Traccar)  
> **Язык:** C++17 (Core) + Kotlin/Jetpack Compose (UI)  
> **Стандарт:** MISRA C++:2023  
> **Цель:** Навороченный GPS трекер с красивым интерфейсом, фоновым режимом, ретраями и аналитикой

---

## Оглавление

1. [Общее описание](#1-общее-описание)
2. [Traccar протокол (порт 5055)](#2-traccar-протокол-порт-5055)
3. [Функциональные требования](#3-функциональные-требования)
4. [UI/UX Дизайн](#4-uiux-дизайн)
5. [Архитектура приложения](#5-архитектура-приложения)
6. [MISRA C++:2023 — применённые правила](#6-misra-c2023--применённые-правила)
7. [Обработка ошибок и логирование](#7-обработка-ошибок-и-логирование)
8. [Тестирование](#8-тестирование)
9. [CI/CD пайплайн](#9-cicd-пайплайн)
10. [Структура проекта](#10-структура-проекта)
11. [Deviations](#11-deviations)
12. [Приложение: Полный checklist MISRA](#12-приложение-полный-checklist-misra)

---

## 1. Общее описание

### 1.1. Назначение

Разработка приложения-трекера для Android, которое:

- Получает координаты GPS с устройства
- Отправляет их на Traccar-сервер `http://map.bd62.ru:5055` по протоколу OsmAnd
- Работает в фоне (foreground service)
- Имеет современный стеклянный интерфейс (neomorphism + glassmorphism)
- Отображает карту со слоем трека
- Ведёт статистику (дистанция, скорость, высота)
- Соответствует MISRA C++:2023 для C++ кода

### 1.2. Целевая платформа

| Параметр | Значение |
|----------|---------|
| Android API | 26+ (8.0 Oreo, foreground service) |
| Target API | 34 (Android 14) |
| Architecture | arm64-v8a, armeabi-v7a, x86_64 |
| Min RAM | 2 GB |
| C++ Standard | C++17 |
| NDK | r27+ |

### 1.3. Стек технологий

| Компонент | Технология | Причина |
|-----------|-----------|---------|
| UI | Kotlin + Jetpack Compose | Современный, declarative, лёгкий glassmorphism |
| C++ Core | C++17 (MISRA subset) | Требование ТЗ |
| Карта | MapLibre GL Native | Open source, C++ native, без API-ключей |
| HTTP | libcurl (minimal static) | Зрелый, embeddable, async multi-интерфейс |
| GPS (Android) | FusedLocationProviderClient (через JNI или Kotlin) | Location API только на Java/Kotlin |
| JSON | Пользовательский zero-alloc билдер | MISRA compliance, нет динамической памяти |
| SQLite | SQLite amalgamation | ACID, embedded, 0 конфигурации |
| Компас/Сенсоры | Android SensorManager → JNI | NDK не имеет прямого доступа к сенсорам |

---

## 2. Traccar протокол (порт 5055)

### 2.1. Формат запроса

Приоритет — **HTTP GET с query-параметрами** (максимальная совместимость).

```
GET http://map.bd62.ru:5055/?
  id=DEVICE_ID
  &lat=55.7558
  &lon=37.6173
  &timestamp=1717430000000
  &speed=42.3
  &bearing=180
  &altitude=156
  &accuracy=5
  &batt=85
  &hdop=0.8
```

### 2.2. Параметры

| Параметр | Тип | Обязат. | Описание |
|----------|-----|----------|----------|
| `id` / `deviceid` | string | **да** | Уникальный ID устройства |
| `lat` | double | **да\*** | Широта |
| `lon` | double | **да\*** | Долгота |
| `timestamp` | int64 | нет | Epoch ms |
| `speed` | double | нет | Скорость (узлы или км/ч — настраивается на сервере) |
| `bearing` / `heading` | double | нет | Курс в градусах |
| `altitude` | double | нет | Высота в метрах |
| `accuracy` | double | нет | Точность в метрах |
| `hdop` | double | нет | Horizontal Dilution of Precision |
| `batt` | int32 | нет | Уровень заряда батареи 0–100 |
| `charge` | bool | нет | true/false — на зарядке |

\* lat/lon обязательны для position update.

### 2.3. Формат POST (опционально, для серверов ≥6.7)

```json
{
  "device_id": "device-001",
  "location": {
    "timestamp": "2026-06-03T04:14:00.000Z",
    "coords": {
      "latitude": 55.7558,
      "longitude": 37.6173,
      "accuracy": 5.0,
      "speed": 42.3,
      "heading": 180.0,
      "altitude": 156.0
    },
    "battery": {
      "level": 0.85,
      "is_charging": false
    }
  }
}
```

### 2.4. Транспорт

| Параметр | Значение |
|----------|---------|
| Протокол | HTTP (без TLS, plain 5055) |
| Content-Type | `application/x-www-form-urlencoded` (GET) / `application/json` (POST) |
| Интервал | 1–30 секунд (настраивается) |
| Retry | Exponential backoff: 5s → 30s → 120s → 300s max |

### 2.5. Источники

- <https://www.traccar.org/osmand/>
- <https://github.com/traccar/traccar-client>

---

## 3. Функциональные требования

### 3.1. Основные функции

| # | Функция | Приоритет |
|---|---------|-----------|
| F1 | Получение координат GPS (Fused Location Provider, interval настраивается) | MUST |
| F2 | Отправка на Traccar сервер по HTTP | MUST |
| F3 | Фоновый трекинг (foreground service с уведомлением) | MUST |
| F4 | Офлайн-очередь с ретрансляцией (SQLite) | MUST |
| F5 | Отображение карты с текущей позицией | MUST |
| F6 | Отображение трека (polyline) на карте | MUST |
| F7 | Speedometer (текущая + средняя скорость) | MUST |
| F8 | Компас (направление движения) | MUST |
| F9 | Altimeter (высота) | SHOULD |
| F10 | Статистика: дистанция, время, макс/средняя скорость | MUST |
| F11 | Графики: скорость/высота по времени (area chart) | SHOULD |
| F12 | Настройки сервера (URL, device ID) | MUST |
| F13 | Настройки GPS (интервал, точность, фильтры) | MUST |
| F14 | Режимы батареи (Performance / Balanced / Max Savings) | SHOULD |
| F15 | Тёмная тема | MUST |

### 3.2. Фильтры

| Фильтр | Описание |
|--------|---------|
| Min speed | Не отправлять точки при скорости < N км/ч |
| Accuracy | Игнорировать точки с точностью > N метров |
| Zero-distance | Удалять дубликаты с тем же lat/lon |
| Proximity | Кластеризация близких точек (min расстояние) |

### 3.3. Режимы батареи

| Режим | GPS interval | Min accuracy | Сеть | Batch size |
|-------|-------------|-------------|------|-----------|
| Performance | 1 сек | 3 м | WiFi/Cell | 5 точек |
| Balanced | 5 сек | 10 м | WiFi/Cell | 10 точек |
| Max Savings | 30 сек | 20 м | Cell only | 25 точек |

---

## 4. UI/UX Дизайн

### 4.1. Стиль: Neo-Glassmorphism + Material Design 3

```
Фон:         #0D1117 (deep space)
Surface:     #161B22
Primary:     #00E5FF (cyan neon — скорость, активные элементы)
Secondary:   #7C4DFF (purple — маркеры, статистика)
Accent:      #FF6F00 (amber — запись, предупреждения)
Glass:       rgba(22,27,34,0.72) + backdrop-filter: blur(20px)
```

### 4.2. Экраны

#### Экран 1: Карта (главный)

```
┌─────────────────────────────────────┐
│ 🔋85%  📡12/18  💾45MB  ⚡4G  │⌂│
│                                     │
│        MAPLIBRE GL (fullscreen)     │
│        • текущая позиция (blue dot) │
│        • трек (cyan polyline, glow) │
│        • waypoints (purple pins)    │
│                                     │
│  ┌────┐  ┌────────────┐  ┌────┐    │
│  │ 🧭  │  │  42.3 km/h │  │ ⛰  │    │ ← Glass
│  │ 14° │  │  ██████░░  │  │156m│    │   карточки
│  └────┘  │ avg: 38.1  │  └────┘    │
│          └────────────┘             │
│                                     │
│  ┌──────────────────────────┐       │
│  │ 0.0 km │ 12:34  │ 🔴 REC │       │ ← MiniStats
│  │ today  │ active │ ◼ STOP │       │
│  └──────────────────────────┘       │
│                                     │
│       [🧭 Компас]  [▶ FAB]  [+/-]  │
└─────────────────────────────────────┘
```

**Компоненты:**
- MapLibre GL — fullscreen, follow GPS, tilt 45° в режиме трека
- SpeedGlass — cyan цифры при движении, красные при остановке
- Compass — круглый циферблат, вращается по heading
- Altimeter — высота GPS + барометр
- MiniStats — стеклянная панель внизу (дистанция сегодня, время, REC/STOP)
- FAB Start/Stop — неоновая пульсация (rec), конкавное нажатие
- Zoom — вертикальный стеклянный стек +/- справа

#### Экран 2: Статистика

```
┌─────────────────────────────────────┐
│ ← Назад        📊 Статистика        │
│                                     │
│  [ Today | Week | Month | Year ]    │ ← pill tabs
│                                     │
│  ┌─────────────────────────┐        │
│  │  Total Distance         │        │
│  │  12.4 km                │ ← big  │
│  │  ████████████░░░░ 78%   │        │
│  └─────────────────────────┘        │
│                                     │
│  ┌──────┐  ┌──────┐  ┌──────┐      │
│  │ ⏱2h34│  │ 🏁64.2│  │📈38.1│    │
│  └──────┘  └──────┘  └──────┘      │
│                                     │
│  ┌─────────────────────────┐        │
│  │ Speed ╱ Altitude (chart)│        │
│  │ ╱╲    ╱╲    ╱╲          │        │
│  └─────────────────────────┘        │
│                                     │
│  📅 2026-06-02  4.2 km  32 min  ▶  │
│  📅 2026-06-01  8.1 km  1h 12m  ▶  │
└─────────────────────────────────────┘
```

#### Экран 3: Настройки

Секции (glass cards):
1. **🔗 Traccar Server** — URL + device ID + статус подключения ✔
2. **📡 GPS** — слайдеры: interval (1–60s), min distance (0–100m), min accuracy (0–50m), timeout (10–120s)
3. **🔋 Battery** — 3-позиционный селектор: Performance / Balanced / Max Savings
4. **🎯 Filters** — чекбоксы: min speed, accuracy, zero-distance, proximity
5. **🌙 Theme** — radio: System / Dark / Light
6. **ℹ️ About** — версия, протокол

### 4.3. Навигация

BottomNavigationBar с 3 табами: Карта | Статистика | Настройки

---

## 5. Архитектура приложения

### 5.1. Слои

```
┌─────────────────────────────────────────────────────┐
│              PRESENTATION (Kotlin/Compose)           │
│  MapScreen  │  StatsScreen  │  SettingsScreen        │
│  MapVM      │  StatsVM      │  SettingsVM            │
│  MVI pattern: state + intent                         │
└──────────────────┬──────────────────────────────────┘
                   │ JNI bridge
┌──────────────────▼──────────────────────────────────┐
│              C++ CORE LAYER                          │
│                                                      │
│  ┌──────────────┐   ┌─────────────────────┐         │
│  │ LocationEngine│   │  TraccarClient      │         │
│  │ - JNI GPS HAL │   │  - HTTP/1.1 POST    │         │
│  │ - KalmanFilter│   │  - RetryQueue       │         │
│  │ - Barometer   │   │  - JSON builder     │         │
│  └───────┬───────┘   └─────────┬───────────┘         │
│          │                     │                      │
│  ┌───────▼───────┐   ┌────────▼───────────┐         │
│  │ StateMachine  │   │  MessageQueue       │         │
│  │ IDLE→ACQUIRE→ │   │  SQLite-backed      │         │
│  │ TRACK→UPLOAD  │   │  exp backoff retry  │         │
│  └───────┬───────┘   └────────┬───────────┘         │
│          │                     │                      │
│  ┌───────▼────────────────────▼───────────┐          │
│  │          DATA LAYER                      │         │
│  │  TrackDB │ SettingsDB │ CacheDB         │         │
│  │  (SQLite – pure C amalgamation)        │         │
│  └─────────────────────────────────────────┘         │
└──────────────────────────────────────────────────────┘
```

### 5.2. State Machine

```
        ┌──────────┐
start ──►   IDLE   ◄── stop ─────────┐
        └────┬─────┘                 │
             │ start                  │
             ▼                        │
        ┌──────────┐                 │
        │ ACQUIRING│ (3D fix)        │
        │ fix ok   │                 │
        └────┬─────┘                 │
             ▼                        │
        ┌──────────┐    pause        │
        │ TRACKING ├─────────────────►│
        │ GPS on   │                 │
        └────┬─────┘                 │
             │ batch ready            │
             ▼                        │
        ┌──────────┐                 │
        │ UPLOADING│ (HTTP POST)     │
        │ success  │                 │
        └──────────┘                 │
             │                        │
             └────────────────────────┘
```

### 5.3. Message Queue (SQLite-backed)

```
Таблица: pending_messages

| id | payload (JSON) | timestamp | retry_count | next_try_at |
|----|---------------|-----------|-------------|-------------|

Retry policy:
  Попытка 1 → +5s
  Попытка 2 → +30s
  Попытка 3 → +120s
  Попытка 4+ → +300s
  Max retries: ∞ (пока не ack'd)
```

### 5.4. Kalman Filter

```
State vector: [lat, lon, v_north, v_east]
Measurement:  [lat, lon, speed, bearing]

Цель:
  - Сгладить GPS jitter
  - Оценить скорость при кратковременных (<3s) пропаданиях сигнала
  - Отбрасывать выбросы (внезапные 100 km/h)
```

### 5.5. Background Service

```
Android os
┌─────────────────────────────────────────────────┐
│ ForegroundService                                │
│ Notification: "Трекинг активен — 42.3 km/h"      │
│                                                  │
│ wakeLock = acquire(PARTIAL_WAKE_LOCK)            │
│ while (state == TRACKING) {                      │
│   pos = locationEngine->getFix(interval);        │
│   if (pos.accuracy < MAX_ACCURACY) {             │
│     kalmanFilter->update(pos);                   │
│     trackDB->insert(kalmanPoint);                │
│     msgQueue->enqueue(kalmanPoint);              │
│   }                                              │
│   uploader->tryFlushBatch(BATCH_SIZE);            │
│   sendToUI({speed, alt, bearing, lat, lon});     │
│ }                                                │
│ wakeLock.release()                               │
└─────────────────────────────────────────────────┘
```

### 5.6. Performance targets

| Метрика | Target |
|---------|--------|
| GPS → UI latency | <200ms |
| HTTP upload | <500ms (WiFi) |
| Battery drain (balanced) | <5%/h |
| Memory (idle) | <40MB |
| Memory (tracking) | <80MB |
| Queue recovery 1000 points | <10s |
| Cold start → map ready | <3s |

---

## 6. MISRA C++:2023 — применённые правила

### 6.1. Классификация

| Категория | Описание |
|-----------|----------|
| **Mandatory (M)** | Нарушение запрещено всегда. Deviation невозможен. |
| **Required (R)** | Должно соблюдаться, если нет formal deviation. |
| **Advisory (A)** | Рекомендация. Deviation не требуется, но документируется. |

### 6.2. Ключевые директивы

| ID | Суть | Тип |
|----|------|-----|
| **M0-1-1** | Проект должен иметь политику соблюдения MISRA | Mandatory |
| **M0-1-2** | Все deviations документировать в `MISRA_DEVIATIONS.md` | Mandatory |
| **M0-1-3** | Статический анализ — обязательный этап сборки | Mandatory |
| R0-1-1 | Языковой стандарт: C++17 | Required |
| R0-1-2 | Toolchain: NDK r27+, Clang 19+ | Required |

### 6.3. Ключевые правила языка

| ID | Суть | Применение |
|----|------|------------|
| **R5-0-1** | Только fixed-width типы: `int32_t`, `uint32_t` | Все целочисленные типы |
| **M6-2-1** | Все члены класса инициализированы | In-class initializers |
| **R8-2-1** | Запрет C-style cast | `static_cast<>` всегда |
| **R10-1-1** | Инициализация всех членов | Конструкторы + default-init |
| **M10-1-1** | Виртуальные деструкторы в базовых классах | Все интерфейсы |
| **R12-1-1** | Исключения — только по политике | Используем `tl::expected` |
| **R16-1-1** | Макросы — только где необходимо | Минимум `#define` |
| **R18-0-2** | Нет raw `new`/`delete` | `std::vector`, `std::array`, placement new на pool |
| **R18-0-3** | Все строки — `std::string_view` / `std::string` | Без C-строк |
| **R18-1-1** | Проверка границ массивов | `gsl::span`, range-for |

### 6.4. Memory policy

- **Запрещён** `malloc`/`free`, `new`/`delete` в production коде
- **Только** placement new на pre-allocated pool
- **RingBuffer** фиксированного размера для GPS точек
- **SQLite** управляет своей памятью (mmap + page cache)
- **libcurl** — минимальный malloc при старте

### 6.5. Пример compliance-кода

```cpp
// СООТВЕТСТВУЕТ MISRA C++:2023

#include <cstdint>
#include <array>
#include <optional>
#include <gsl/span>

namespace umap::core {

class KalmanFilter {
public:
    KalmanFilter() noexcept = default;

    struct State {
        double lat{0.0};
        double lon{0.0};
        double v_north{0.0};
        double v_east{0.0};
    };

    struct Measurement {
        double lat{0.0};
        double lon{0.0};
        double speed{0.0};
        double bearing{0.0};
        double accuracy{10.0};
    };

    void update(const Measurement& m) noexcept;
    [[nodiscard]] State get_state() const noexcept { return state_; }

private:
    State state_;
    // Covariance matrix — fixed size, pre-allocated
    std::array<double, 16> P_{}; // 4x4 flattened
};

// RingBuffer — фиксированный размер, zero alloc
template<typename T, std::size_t N>
class RingBuffer {
    static_assert(N > 0 && N <= 4096);
    std::array<T, N> data_{};
    std::size_t head_{0};
    std::size_t count_{0};
public:
    void push(const T& item) noexcept;
    [[nodiscard]] std::optional<T> pop() noexcept;
    [[nodiscard]] bool full() const noexcept { return count_ == N; }
    [[nodiscard]] bool empty() const noexcept { return count_ == 0; }
};

using GpsPointBuffer = RingBuffer<GpsPoint, 1024>;

}
```

---

## 7. Обработка ошибок и логирование

### 7.1. Result-тип (вместо исключений)

```cpp
#include <tl/expected.hpp>

enum class Err : uint8_t {
    Ok,
    NetworkError,
    GpsTimeout,
    ParseError,
    StorageFull
};

template<typename T>
using Result = tl::expected<T, Err>;
```

### 7.2. Логирование

- **Android NDK:** `__android_log_print()` через обёртку
- **Уровни:** DEBUG / INFO / WARN / ERROR
- **Формат:** Структурированные строки через `std::string_view`
- **MISRA:** `R17-0-1` — заголовки C++ (`<cstdio>` вместо `<stdio.h>`)

### 7.3. Запрещённые паттерны

| Паттерн | MISRA | Замена |
|---------|-------|--------|
| `new`/`delete` | R18-0-2 | `std::vector` / pool |
| C-style cast | R8-2-1 | `static_cast<>` |
| `reinterpret_cast` | R8-2-2 | Только для GL/EGL (deviation) |
| `printf`/`scanf` | R17-0-1 | Logger class |
| goto | R9-6-1 | while/for/if |
| `std::exit()` | R5-0-1 | return Error |
| Исключения `throw` | R12-1-1 | `tl::expected` |

---

## 8. Тестирование

### 8.1. Unit-тесты

| Модуль | Framework | Min coverage | Критические тесты |
|--------|-----------|-------------|-------------------|
| Core (гео) | Google Test | 95% | validation, distances |
| KalmanFilter | Google Test | 95% | smooth, reject outliers |
| StateMachine | Google Test | 100% | all transitions |
| MessageQueue | Google Test | 95% | enqueue/dequeue/retry |
| TraccarClient | Google Test + mock | 90% | build url, retry |
| Config | Google Test | 100% | parse, defaults |

### 8.2. MISRA-анализ тестов

Тесты могут иметь ослабленные проверки (deviation на C-style cast в EXPECT_* макросах), **но production код проверяется полностью**.

### 8.3. Инструменты

| Инструмент | MISRA C++:2023 | Лицензия |
|-----------|---------------|----------|
| clang-tidy-19 | Основной (cppcoreguidelines) | Apache 2.0 |
| cppcheck 2.15+ | MISRA addon | GPL-3.0 |
| CodeChecker | Dashboard | Apache 2.0 |

---

## 9. CI/CD пайплайн

### 9.1. GitHub Actions (2 jobs)

```
Job 1: Static Analysis + Test
  - clang-tidy MISRA check (все предупреждения → ошибки)
  - cppcheck MISRA addon
  - Unit tests (x86_64 host)
  - Upload MISRA report artifact

Job 2: Build APK (depends on Job 1)
  - CMake → native libs (arm64-v8a, armeabi-v7a, x86_64)
  - Gradle → assembleRelease
  - jarsigner → signed APK
  - Upload APK artifact
```

### 9.2. CMake MISRA integration

```cmake
# Встраивание clang-tidy в сборку
set(CMAKE_CXX_CLANG_TIDY
    "clang-tidy"
    "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")

# Обязательные флаги
add_compile_options(
    -std=c++17
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion -Wsign-conversion
    -Wnon-virtual-dtor -Wold-style-cast
    -Woverloaded-virtual
    -fno-exceptions
    -fno-rtti
    -fstack-protector-strong
)
```

### 9.3. Pre-commit hook

```bash
#!/bin/bash
set -euo pipefail
echo "[MISRA] clang-tidy check..."
cmake -B build-check -GNinja \
  -DCMAKE_CXX_CLANG_TIDY="clang-tidy" \
  -DBUILD_TESTING=ON
cmake --build build-check 2>&1 | tee build-check/misra.log
if grep -Eq "(warning|error):" build-check/misra.log; then
    echo "[MISRA] FAILED"; exit 1
fi
echo "[MISRA] OK"
```

---

## 10. Структура проекта

```
tracker/
├── android/
│   ├── ui/
│   │   ├── screens/
│   │   │   ├── MapScreen.kt
│   │   │   ├── StatsScreen.kt
│   │   │   └── SettingsScreen.kt
│   │   ├── components/
│   │   │   ├── GlassCard.kt
│   │   │   ├── Speedometer.kt
│   │   │   ├── CompassWidget.kt
│   │   │   ├── StartStopButton.kt
│   │   │   └── MiniStatsBar.kt
│   │   ├── theme/
│   │   │   ├── Theme.kt / Color.kt / Typography.kt / Shapes.kt
│   │   └── navigation/
│   ├── viewmodel/
│   │   ├── MapViewModel.kt
│   │   ├── StatsViewModel.kt
│   │   └── SettingsViewModel.kt
│   ├── bridge/
│   │   └── TrackerBridge.kt      # JNI extern fun
│   └── service/
│       └── TrackerForegroundService.kt
│
├── core/                          # C++ core (MISRA)
│   ├── include/umap/
│   │   ├── core/        → geo_types.h, coordinate.h, kalman_filter.h
│   │   ├── data/        → traccar_client.h, message_queue.h, track_db.h
│   │   ├── platform/    → android_bridge.h, jni_types.h
│   │   └── utils/       → logger.h, config.h, ring_buffer.h, result.h
│   ├── src/
│   │   ├── core/        → kalman_filter.cpp, state_machine.cpp
│   │   ├── data/        → traccar_client.cpp, message_queue.cpp, track_db.cpp
│   │   ├── platform/    → android_bridge.cpp, jni_callbacks.cpp
│   │   └── utils/       → logger.cpp, config.cpp
│   ├── jni/             → jni_bridge.cpp
│   ├── tests/
│   │   ├── test_kalman.cpp
│   │   ├── test_state_machine.cpp
│   │   ├── test_message_queue.cpp
│   │   └── test_traccar_client.cpp
│   ├── CMakeLists.txt
│   └── .clang-tidy
│
├── .github/workflows/
│   └── build-and-test.yml
├── .githooks/
│   └── pre-commit
├── MISRA_DEVIATIONS.md
├── CMakeLists.txt
└── README.md
```

### 10.1. Namespaces

```cpp
namespace umap::core        // Алгоритмы ядра, Kalman, гео-типы
namespace umap::data        // Сеть, очереди, базы данных
namespace umap::platform    // JNI bridge, Android-специфичное
namespace umap::utils       // Logger, Config, Assert, Result
```

Запрещено `using namespace` в заголовочных файлах.

---

## 11. Deviations

Файл `MISRA_DEVIATIONS.md` в корне проекта.

| # | Правило | Причина | Область |
|---|---------|---------|---------|
| D001 | R8-2-1 (C-style cast) | JNI макросы (`CallVoidMethod`) | `platform/` |
| D002 | R8-2-2 (reinterpret_cast) | EGL/GLES API требует | `rendering/` |
| D003 | M6-2-1 (типы) | JNI использует `int` для callback | `platform/` |
| D004 | Макросы Google Test | `EXPECT_EQ`, `ASSERT_TRUE` — C-style cast внутри | `tests/` |

---

## 12. Приложение: Полный checklist MISRA

### Sprint 0 — Инициализация

- [ ] Утверждена политика MISRA (M0-1-1)
- [ ] Создан `.clang-tidy` с проверками
- [ ] Создан `MISRA_DEVIATIONS.md`
- [ ] CMake + `CMAKE_CXX_CLANG_TIDY` настроен
- [ ] GitHub Actions pipeline
- [ ] Pre-commit hook

### Каждый коммит

- [ ] clang-tidy 0 warnings/errors (кроме deviations)
- [ ] cppcheck MISRA addon пройден
- [ ] Unit tests pass
- [ ] APK собирается
- [ ] Deviations актуальны

### Code Review

- [ ] Нет C-style cast (кроме deviations)
- [ ] Все члены инициализированы
- [ ] Нет `new`/`delete` / `malloc`/`free`
- [ ] Fixed-width типы (`int32_t` etc.)
- [ ] Result pattern, не исключения
- [ ] Логи без secrets

---

*Документ сформирован на основе протокола Traccar v5, MISRA C++:2023, Android NDK r27+ и руководства AUTOSAR C++14.*
