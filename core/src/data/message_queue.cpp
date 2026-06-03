// SPDX-License-Identifier: Apache-2.0
// MessageQueue: SQLite-backed offline queue with retry and TTL

#include "umap/data/message_queue.h"

#include <sqlite3.h>
#include <cstdio>
#include <ctime>

namespace umap::data {

struct MessageQueue::Impl {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt_enqueue = nullptr;
    sqlite3_stmt* stmt_dequeue_select = nullptr;
    sqlite3_stmt* stmt_dequeue_mark = nullptr;
    sqlite3_stmt* stmt_mark_sent = nullptr;
    sqlite3_stmt* stmt_mark_failed = nullptr;
    sqlite3_stmt* stmt_pending_count = nullptr;
    sqlite3_stmt* stmt_total_count = nullptr;
    sqlite3_stmt* stmt_purge_expired = nullptr;
    sqlite3_stmt* stmt_get_retry = nullptr;
};

namespace {

constexpr const char* SQL_CREATE_TABLE =
    "CREATE TABLE IF NOT EXISTS messages ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  payload TEXT NOT NULL,"
    "  retry_count INTEGER NOT NULL DEFAULT 0,"
    "  created_at INTEGER NOT NULL,"
    "  next_retry_at INTEGER NOT NULL"
    ");";

constexpr const char* SQL_CREATE_INDEX =
    "CREATE INDEX IF NOT EXISTS idx_messages_next_retry "
    "ON messages(next_retry_at, created_at);";

constexpr const char* SQL_ENQUEUE =
    "INSERT INTO messages (payload, created_at, next_retry_at) "
    "VALUES (?, ?, ?);";

constexpr const char* SQL_DEQUEUE_SELECT =
    "SELECT id, payload, retry_count, created_at "
    "FROM messages "
    "WHERE next_retry_at <= ? "
    "ORDER BY created_at ASC LIMIT 1;";

constexpr const char* SQL_DEQUEUE_MARK =
    "UPDATE messages SET next_retry_at = ? "
    "WHERE id = ?;";

constexpr const char* SQL_MARK_SENT =
    "DELETE FROM messages WHERE id = ?;";

constexpr const char* SQL_MARK_FAILED =
    "UPDATE messages SET retry_count = retry_count + 1, "
    "next_retry_at = ? WHERE id = ?;";

constexpr const char* SQL_PENDING_COUNT =
    "SELECT COUNT(*) FROM messages WHERE next_retry_at <= ?;";

constexpr const char* SQL_TOTAL_COUNT =
    "SELECT COUNT(*) FROM messages;";

constexpr const char* SQL_PURGE_EXPIRED =
    "DELETE FROM messages WHERE created_at <= ?;";

constexpr const char* SQL_GET_RETRY =
    "SELECT retry_count FROM messages WHERE id = ?;";

constexpr int64_t SECONDS_PER_HOUR = 3600;

}  // namespace

MessageQueue::MessageQueue(const char* db_path) : impl_(new Impl()) {
    if (db_path == nullptr) {
        return;
    }

    int rc = sqlite3_open(db_path, &impl_->db);
    if (rc != SQLITE_OK) {
        impl_->db = nullptr;
        return;
    }

    sqlite3_exec(impl_->db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(impl_->db, "PRAGMA synchronous=NORMAL;", nullptr, nullptr, nullptr);

    create_tables();
}

MessageQueue::~MessageQueue() noexcept {
    finalize();
    delete impl_;
}

void MessageQueue::create_tables() {
    if (impl_->db == nullptr) {
        return;
    }
    sqlite3_exec(impl_->db, SQL_CREATE_TABLE, nullptr, nullptr, nullptr);
    sqlite3_exec(impl_->db, SQL_CREATE_INDEX, nullptr, nullptr, nullptr);

    sqlite3_prepare_v2(impl_->db, SQL_ENQUEUE, -1, &impl_->stmt_enqueue, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_DEQUEUE_SELECT, -1, &impl_->stmt_dequeue_select, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_DEQUEUE_MARK, -1, &impl_->stmt_dequeue_mark, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_MARK_SENT, -1, &impl_->stmt_mark_sent, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_MARK_FAILED, -1, &impl_->stmt_mark_failed, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_PENDING_COUNT, -1, &impl_->stmt_pending_count, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_TOTAL_COUNT, -1, &impl_->stmt_total_count, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_PURGE_EXPIRED, -1, &impl_->stmt_purge_expired, nullptr);
    sqlite3_prepare_v2(impl_->db, SQL_GET_RETRY, -1, &impl_->stmt_get_retry, nullptr);
}

void MessageQueue::finalize() noexcept {
    if (impl_->stmt_enqueue) { sqlite3_finalize(impl_->stmt_enqueue); impl_->stmt_enqueue = nullptr; }
    if (impl_->stmt_dequeue_select) { sqlite3_finalize(impl_->stmt_dequeue_select); impl_->stmt_dequeue_select = nullptr; }
    if (impl_->stmt_dequeue_mark) { sqlite3_finalize(impl_->stmt_dequeue_mark); impl_->stmt_dequeue_mark = nullptr; }
    if (impl_->stmt_mark_sent) { sqlite3_finalize(impl_->stmt_mark_sent); impl_->stmt_mark_sent = nullptr; }
    if (impl_->stmt_mark_failed) { sqlite3_finalize(impl_->stmt_mark_failed); impl_->stmt_mark_failed = nullptr; }
    if (impl_->stmt_pending_count) { sqlite3_finalize(impl_->stmt_pending_count); impl_->stmt_pending_count = nullptr; }
    if (impl_->stmt_total_count) { sqlite3_finalize(impl_->stmt_total_count); impl_->stmt_total_count = nullptr; }
    if (impl_->stmt_purge_expired) { sqlite3_finalize(impl_->stmt_purge_expired); impl_->stmt_purge_expired = nullptr; }
    if (impl_->stmt_get_retry) { sqlite3_finalize(impl_->stmt_get_retry); impl_->stmt_get_retry = nullptr; }
    if (impl_->db) { sqlite3_close(impl_->db); impl_->db = nullptr; }
}

int64_t MessageQueue::now_seconds() const noexcept {
    return static_cast<int64_t>(std::time(nullptr));
}

int64_t MessageQueue::calculate_backoff(uint32_t retry_count) const noexcept {
    // Exponential backoff: 2^retry_count seconds, capped at 1 hour
    constexpr int64_t BASE_DELAY = 2;
    constexpr int64_t MAX_DELAY = 3600;

    int64_t delay = BASE_DELAY;
    for (uint32_t i = 0U; i < retry_count && delay < MAX_DELAY; ++i) {
        delay *= 2;
    }
    return delay < MAX_DELAY ? delay : MAX_DELAY;
}

utils::Result<void> MessageQueue::enqueue(const char* payload) {
    if (impl_->db == nullptr || impl_->stmt_enqueue == nullptr || payload == nullptr) {
        return utils::Unexpected(utils::ErrorCode::InvalidConfig);
    }

    int64_t now = now_seconds();
    sqlite3_bind_text(impl_->stmt_enqueue, 1, payload, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(impl_->stmt_enqueue, 2, now);
    sqlite3_bind_int64(impl_->stmt_enqueue, 3, now);

    int rc = sqlite3_step(impl_->stmt_enqueue);
    sqlite3_reset(impl_->stmt_enqueue);
    sqlite3_clear_bindings(impl_->stmt_enqueue);

    if (rc != SQLITE_DONE) {
        return utils::Unexpected(utils::ErrorCode::InternalError);
    }
    return utils::Result<void>();
}

utils::Result<QueuedMessage> MessageQueue::dequeue() {
    if (impl_->db == nullptr || impl_->stmt_dequeue_select == nullptr ||
        impl_->stmt_dequeue_mark == nullptr) {
        return utils::Unexpected(utils::ErrorCode::InvalidConfig);
    }

    int64_t now = now_seconds();

    sqlite3_bind_int64(impl_->stmt_dequeue_select, 1, now);
    utils::Result<QueuedMessage> result;
    int rc = sqlite3_step(impl_->stmt_dequeue_select);
    if (rc == SQLITE_ROW) {
        QueuedMessage msg;
        msg.id = static_cast<uint64_t>(sqlite3_column_int64(impl_->stmt_dequeue_select, 0));
        const char* text = reinterpret_cast<const char*>(
            sqlite3_column_text(impl_->stmt_dequeue_select, 1));
        if (text != nullptr) {
            msg.payload = text;
        }
        msg.retry_count = static_cast<uint32_t>(
            sqlite3_column_int(impl_->stmt_dequeue_select, 2));
        msg.created_at = sqlite3_column_int64(impl_->stmt_dequeue_select, 3);
        msg.next_retry_at = now + 86400;

        sqlite3_reset(impl_->stmt_dequeue_select);
        sqlite3_clear_bindings(impl_->stmt_dequeue_select);

        sqlite3_bind_int64(impl_->stmt_dequeue_mark, 1, msg.next_retry_at);
        sqlite3_bind_int64(impl_->stmt_dequeue_mark, 2, static_cast<sqlite3_int64>(msg.id));
        sqlite3_step(impl_->stmt_dequeue_mark);
        sqlite3_reset(impl_->stmt_dequeue_mark);
        sqlite3_clear_bindings(impl_->stmt_dequeue_mark);

        result = msg;
    } else {
        sqlite3_reset(impl_->stmt_dequeue_select);
        sqlite3_clear_bindings(impl_->stmt_dequeue_select);
        result = utils::Unexpected(utils::ErrorCode::NotFound);
    }

    return result;
}

utils::Result<void> MessageQueue::mark_sent(uint64_t message_id) {
    if (impl_->db == nullptr || impl_->stmt_mark_sent == nullptr) {
        return utils::Unexpected(utils::ErrorCode::InvalidConfig);
    }

    sqlite3_bind_int64(impl_->stmt_mark_sent, 1, static_cast<sqlite3_int64>(message_id));
    int rc = sqlite3_step(impl_->stmt_mark_sent);
    sqlite3_reset(impl_->stmt_mark_sent);
    sqlite3_clear_bindings(impl_->stmt_mark_sent);

    if (rc != SQLITE_DONE) {
        return utils::Unexpected(utils::ErrorCode::InternalError);
    }
    return utils::Result<void>();
}

utils::Result<void> MessageQueue::mark_failed(uint64_t message_id) {
    if (impl_->db == nullptr || impl_->stmt_mark_failed == nullptr) {
        return utils::Unexpected(utils::ErrorCode::InvalidConfig);
    }

    auto retry_result = get_retry_count(message_id);
    uint32_t retry_count = 0U;
    if (retry_result.has_value()) {
        retry_count = retry_result.value();
    }

    int64_t next_retry = now_seconds() + calculate_backoff(retry_count + 1U);

    sqlite3_bind_int64(impl_->stmt_mark_failed, 1, next_retry);
    sqlite3_bind_int64(impl_->stmt_mark_failed, 2, static_cast<sqlite3_int64>(message_id));
    int rc = sqlite3_step(impl_->stmt_mark_failed);
    sqlite3_reset(impl_->stmt_mark_failed);
    sqlite3_clear_bindings(impl_->stmt_mark_failed);

    if (rc != SQLITE_DONE) {
        return utils::Unexpected(utils::ErrorCode::InternalError);
    }
    return utils::Result<void>();
}

uint32_t MessageQueue::pending_count() const noexcept {
    if (impl_->db == nullptr || impl_->stmt_pending_count == nullptr) {
        return 0U;
    }

    int64_t now = now_seconds();
    sqlite3_bind_int64(impl_->stmt_pending_count, 1, now);
    uint32_t count = 0U;
    if (sqlite3_step(impl_->stmt_pending_count) == SQLITE_ROW) {
        count = static_cast<uint32_t>(sqlite3_column_int(impl_->stmt_pending_count, 0));
    }
    sqlite3_reset(impl_->stmt_pending_count);
    sqlite3_clear_bindings(impl_->stmt_pending_count);
    return count;
}

uint32_t MessageQueue::total_count() const noexcept {
    if (impl_->db == nullptr || impl_->stmt_total_count == nullptr) {
        return 0U;
    }

    uint32_t count = 0U;
    if (sqlite3_step(impl_->stmt_total_count) == SQLITE_ROW) {
        count = static_cast<uint32_t>(sqlite3_column_int(impl_->stmt_total_count, 0));
    }
    sqlite3_reset(impl_->stmt_total_count);
    return count;
}

uint32_t MessageQueue::purge_expired(uint32_t ttl_hours) {
    if (impl_->db == nullptr || impl_->stmt_purge_expired == nullptr) {
        return 0U;
    }

    int64_t cutoff = now_seconds() - static_cast<int64_t>(ttl_hours) * SECONDS_PER_HOUR;
    sqlite3_bind_int64(impl_->stmt_purge_expired, 1, cutoff);

    int rc = sqlite3_step(impl_->stmt_purge_expired);
    uint32_t deleted = 0U;
    if (rc == SQLITE_DONE) {
        deleted = static_cast<uint32_t>(sqlite3_changes(impl_->db));
    }
    sqlite3_reset(impl_->stmt_purge_expired);
    sqlite3_clear_bindings(impl_->stmt_purge_expired);
    return deleted;
}

void MessageQueue::reset() {
    if (impl_->db != nullptr) {
        sqlite3_exec(impl_->db, "DELETE FROM messages;", nullptr, nullptr, nullptr);
    }
}

utils::Result<uint32_t> MessageQueue::get_retry_count(uint64_t message_id) const noexcept {
    if (impl_->db == nullptr || impl_->stmt_get_retry == nullptr) {
        return utils::Unexpected(utils::ErrorCode::InvalidConfig);
    }

    sqlite3_bind_int64(impl_->stmt_get_retry, 1, static_cast<sqlite3_int64>(message_id));
    utils::Result<uint32_t> result;
    if (sqlite3_step(impl_->stmt_get_retry) == SQLITE_ROW) {
        result = static_cast<uint32_t>(sqlite3_column_int(impl_->stmt_get_retry, 0));
    } else {
        result = utils::Unexpected(utils::ErrorCode::NotFound);
    }
    sqlite3_reset(impl_->stmt_get_retry);
    sqlite3_clear_bindings(impl_->stmt_get_retry);
    return result;
}

}  // namespace umap::data
