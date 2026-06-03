# Техническое задание
## Android GPS Tracker (Revision 1.1)

Данный документ является консолидированной версией ТЗ на Android GPS Tracker на базе исходного документа и утверждённых архитектурных изменений Revision 1.1.

### Ключевые положения

- Android 8.0+ (Target API 34+)
- Kotlin + Jetpack Compose
- C++17 Core
- MISRA C++:2023
- Traccar (OsmAnd protocol)
- HTTP транспорт (без TLS, по требованиям проекта)
- Foreground Service
- Offline Queue
- Room Database
- MapLibre
- Kalman Filter
- CI/CD и статический анализ

### Архитектура

Presentation Layer:
- Jetpack Compose
- ViewModel
- MVI

Core Layer:
- Kalman Filter
- Geo Algorithms
- State Machine
- Track Processing

Data Layer:
- Room Database
- Repository
- Upload Queue

### Потоки

1. UI Thread
2. Location Thread
3. Upload Thread
4. Database Thread

Используются:
- std::mutex
- std::condition_variable
- std::atomic

### Ограничения очереди

MAX_QUEUE_SIZE = 50000

MESSAGE_TTL_HOURS = 168

Политика:
- удаление сообщений старше 7 суток;
- удаление самых старых записей при переполнении.

### Политика хранения треков

TRACK_RETENTION_DAYS = 90

Дополнительно:
MAX_TRACK_POINTS = 1000000

### Kalman Filter

Модель:
Constant Velocity Model

Состояние:
[lat, lon, v_north, v_east]

Измерение:
[lat, lon, speed, bearing]

Требуется документирование:
- F matrix
- Q matrix
- H matrix
- R matrix
- P matrix

### Android 14+

Обязательные разрешения:

- ACCESS_FINE_LOCATION
- ACCESS_COARSE_LOCATION
- FOREGROUND_SERVICE
- FOREGROUND_SERVICE_LOCATION
- POST_NOTIFICATIONS
- WAKE_LOCK

Дополнительно:
- Doze Mode handling
- Battery Optimization handling

### Политика памяти

Запрещено:
- malloc/free
- new/delete

Разрешено:
- std::string
- std::vector
- std::array
- SQLite/Room внутренние аллокации
- libcurl внутренние аллокации

### Производительность

- GPS → UI < 200 ms
- Upload < 500 ms
- Memory tracking < 100 MB
- Queue recovery 50000 points < 15 sec
- Cold start < 3 sec
- Battery drain balanced < 7%/hour

### Тестирование

Добавлены тесты:

- Queue Overflow Test
- Queue TTL Cleanup Test
- Thread Safety Test
- Kalman Outlier Rejection Test
- Doze Recovery Test
- Foreground Service Restart Test
- Room Migration Test

### Статический анализ

- clang-tidy
- cppcheck
- Google Test
- GitHub Actions

### Статус

Версия документа:
Revision 1.1

Все изменения аудита приняты, кроме:
- перехода на HTTPS;
- обязательной аутентификации устройства.

Оба пункта оставлены без изменений по требованиям проекта.
