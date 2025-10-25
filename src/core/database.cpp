/**
 * @file database.cpp
 * @brief Implementation of Database class
 */

#include "hydra/database.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <chrono>

namespace hydra {

// =============================================================================
// Constructor and Destructor
// =============================================================================

Database::Database(const std::string& db_path) {
    // Open SQLite database
    // SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE = open existing or create new
    // SQLITE_OPEN_FULLMUTEX = thread-safe mode
    int rc = sqlite3_open_v2(
        db_path.c_str(),
        &db_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
        nullptr
    );

    if (rc != SQLITE_OK) {
        std::string error_msg = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw std::runtime_error("Failed to open database: " + error_msg);
    }

    // Create tables if they don't exist
    create_tables();
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// Move constructor
Database::Database(Database&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

// Move assignment
Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        if (db_) {
            sqlite3_close(db_);
        }
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

// =============================================================================
// Private Helper Methods
// =============================================================================

void Database::create_tables() {
    // SQL statements to create tables
    const char* users_table = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id TEXT PRIMARY KEY,
            created_at TEXT NOT NULL,
            total_tokens REAL DEFAULT 0.0,
            total_work_done INTEGER DEFAULT 0
        )
    )";

    const char* tasks_table = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            task_id TEXT PRIMARY KEY,
            created_at TEXT NOT NULL,
            assigned_to TEXT,
            status TEXT NOT NULL,
            data_batch TEXT NOT NULL,
            result TEXT,
            tokens_reward REAL NOT NULL,
            completed_at TEXT
        )
    )";

    const char* transactions_table = R"(
        CREATE TABLE IF NOT EXISTS transactions (
            transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id TEXT NOT NULL,
            amount REAL NOT NULL,
            type TEXT NOT NULL,
            description TEXT,
            timestamp TEXT NOT NULL
        )
    )";

    // Execute table creation
    execute(users_table);
    execute(tasks_table);
    execute(transactions_table);

    // Create indices for better performance
    execute("CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status)");
    execute("CREATE INDEX IF NOT EXISTS idx_transactions_user ON transactions(user_id)");
}

bool Database::execute(const std::string& sql) {
    char* error_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error_msg);

    if (rc != SQLITE_OK) {
        std::string error = error_msg;
        sqlite3_free(error_msg);
        return false;
    }
    return true;
}

std::string Database::current_timestamp() {
    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    // Format as ISO 8601
    std::ostringstream oss;
    oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

// =============================================================================
// User Operations
// =============================================================================

bool Database::create_user(const std::string& user_id) {
    const char* sql = "INSERT INTO users (user_id, created_at, total_tokens, total_work_done) "
                     "VALUES (?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, 0.0);
    sqlite3_bind_int(stmt, 4, 0);

    // Execute
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::optional<User> Database::get_user(const std::string& user_id) {
    const char* sql = "SELECT * FROM users WHERE user_id = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User user;
        user.user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        user.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.total_tokens = sqlite3_column_double(stmt, 2);
        user.total_work_done = sqlite3_column_int(stmt, 3);

        sqlite3_finalize(stmt);
        return user;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool Database::add_tokens(const std::string& user_id, double amount,
                         const std::string& transaction_type,
                         const std::string& description) {
    // Start transaction
    execute("BEGIN TRANSACTION");

    // Update user balance
    const char* update_sql = "UPDATE users SET total_tokens = total_tokens + ? WHERE user_id = ?";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db_, update_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        execute("ROLLBACK");
        return false;
    }

    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_text(stmt, 2, user_id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        execute("ROLLBACK");
        return false;
    }
    sqlite3_finalize(stmt);

    // Log transaction
    const char* insert_sql = "INSERT INTO transactions (user_id, amount, type, description, timestamp) "
                            "VALUES (?, ?, ?, ?, ?)";

    if (sqlite3_prepare_v2(db_, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        execute("ROLLBACK");
        return false;
    }

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, amount);
    sqlite3_bind_text(stmt, 3, transaction_type.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        execute("COMMIT");
        return true;
    }

    execute("ROLLBACK");
    return false;
}

// =============================================================================
// Task Operations
// =============================================================================

bool Database::create_task(const std::string& task_id,
                          const std::string& data_batch,
                          double tokens_reward) {
    const char* sql = "INSERT INTO tasks (task_id, created_at, status, data_batch, tokens_reward) "
                     "VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, task_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, "pending", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, data_batch.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 5, tokens_reward);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::optional<Task> Database::get_pending_task() {
    const char* sql = "SELECT * FROM tasks WHERE status = 'pending' LIMIT 1";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.task_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        task.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (sqlite3_column_text(stmt, 2)) {
            task.assigned_to = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }

        task.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        task.data_batch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (sqlite3_column_text(stmt, 5)) {
            task.result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        }

        task.tokens_reward = sqlite3_column_double(stmt, 6);

        if (sqlite3_column_text(stmt, 7)) {
            task.completed_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }

        sqlite3_finalize(stmt);
        return task;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool Database::assign_task(const std::string& task_id, const std::string& user_id) {
    const char* sql = "UPDATE tasks SET status = 'assigned', assigned_to = ? WHERE task_id = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, task_id.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

bool Database::complete_task(const std::string& task_id, const std::string& result) {
    const char* sql = "UPDATE tasks SET status = 'completed', result = ?, completed_at = ? "
                     "WHERE task_id = ?";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, result.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, task_id.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;
}

std::vector<Task> Database::get_user_tasks(const std::string& user_id,
                                           const std::string& status) {
    std::vector<Task> tasks;

    std::string sql = "SELECT * FROM tasks WHERE assigned_to = ?";
    if (!status.empty()) {
        sql += " AND status = ?";
    }
    sql += " ORDER BY created_at DESC";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return tasks;
    }

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    if (!status.empty()) {
        sqlite3_bind_text(stmt, 2, status.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task;
        task.task_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        task.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        if (sqlite3_column_text(stmt, 2)) {
            task.assigned_to = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        }

        task.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        task.data_batch = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (sqlite3_column_text(stmt, 5)) {
            task.result = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        }

        task.tokens_reward = sqlite3_column_double(stmt, 6);

        if (sqlite3_column_text(stmt, 7)) {
            task.completed_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        }

        tasks.push_back(std::move(task));
    }

    sqlite3_finalize(stmt);
    return tasks;
}

// =============================================================================
// Transaction Operations
// =============================================================================

std::vector<Transaction> Database::get_transactions(const std::string& user_id, int limit) {
    std::vector<Transaction> transactions;

    std::string sql = "SELECT * FROM transactions WHERE user_id = ? ORDER BY timestamp DESC";
    if (limit > 0) {
        sql += " LIMIT " + std::to_string(limit);
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return transactions;
    }

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction tx;
        tx.transaction_id = sqlite3_column_int(stmt, 0);
        tx.user_id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        tx.amount = sqlite3_column_double(stmt, 2);
        tx.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        if (sqlite3_column_text(stmt, 4)) {
            tx.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        }

        tx.timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        transactions.push_back(std::move(tx));
    }

    sqlite3_finalize(stmt);
    return transactions;
}

std::optional<User> Database::get_user_stats(const std::string& user_id) {
    return get_user(user_id);
}

} // namespace hydra
