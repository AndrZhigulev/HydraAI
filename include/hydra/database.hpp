/**
 * @file database.hpp
 * @brief Database management for HydraAI using SQLite3
 *
 * This header defines the database interface for storing:
 * - User accounts and token balances
 * - Training tasks and their status
 * - Transaction history
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <ctime>

// Forward declare SQLite3 types to avoid including sqlite3.h in header
struct sqlite3;
struct sqlite3_stmt;

namespace hydra {

/**
 * @struct User
 * @brief Represents a user account in the system
 */
struct User {
    std::string user_id;           // Unique user identifier
    std::string created_at;        // ISO 8601 timestamp
    double total_tokens{0.0};      // Current token balance
    int total_work_done{0};        // Number of completed tasks
};

/**
 * @struct Task
 * @brief Represents a training task
 */
struct Task {
    std::string task_id;           // Unique task identifier
    std::string created_at;        // When task was created
    std::string assigned_to;       // User ID (empty if unassigned)
    std::string status;            // "pending", "assigned", "completed", "failed"
    std::string data_batch;        // Training data (JSON string)
    std::string result;            // Trained parameters (JSON string)
    double tokens_reward{0.0};     // Token reward for completion
    std::string completed_at;      // When task was completed
};

/**
 * @struct Transaction
 * @brief Represents a token transaction
 */
struct Transaction {
    int transaction_id{0};         // Auto-incremented ID
    std::string user_id;           // User who made the transaction
    double amount{0.0};            // Token amount (positive = earned, negative = spent)
    std::string type;              // "reward", "query", "trade"
    std::string description;       // Human-readable description
    std::string timestamp;         // ISO 8601 timestamp
};

/**
 * @class Database
 * @brief Main database class for HydraAI
 *
 * Manages all database operations including users, tasks, and transactions.
 * Uses SQLite3 for simplicity and portability.
 *
 * Thread Safety: This class is NOT thread-safe. Use external synchronization
 * if accessing from multiple threads.
 *
 * Example usage:
 * @code
 * hydra::Database db("hydra.db");
 * db.create_user("alice123");
 * db.add_tokens("alice123", 10.0, "reward", "Completed task");
 * auto user = db.get_user("alice123");
 * @endcode
 */
class Database {
public:
    /**
     * @brief Constructor - opens/creates database
     * @param db_path Path to SQLite database file
     * @throws std::runtime_error if database cannot be opened
     */
    explicit Database(const std::string& db_path = "hydra.db");

    /**
     * @brief Destructor - closes database connection
     */
    ~Database();

    // Disable copying (database connection shouldn't be copied)
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    // Allow moving
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;

    // =========================================================================
    // User Operations
    // =========================================================================

    /**
     * @brief Create a new user account
     * @param user_id Unique user identifier
     * @return true if successful, false if user already exists
     */
    bool create_user(const std::string& user_id);

    /**
     * @brief Get user information
     * @param user_id User to look up
     * @return User object if found, std::nullopt otherwise
     */
    std::optional<User> get_user(const std::string& user_id);

    /**
     * @brief Add or subtract tokens from user's balance
     * @param user_id User to modify
     * @param amount Token amount (positive = add, negative = subtract)
     * @param transaction_type Type of transaction ("reward", "query", "trade")
     * @param description Human-readable description
     * @return true if successful
     */
    bool add_tokens(const std::string& user_id, double amount,
                   const std::string& transaction_type,
                   const std::string& description);

    // =========================================================================
    // Task Operations
    // =========================================================================

    /**
     * @brief Create a new training task
     * @param task_id Unique task identifier
     * @param data_batch Training data (will be serialized to JSON)
     * @param tokens_reward Token reward for completing this task
     * @return true if successful
     */
    bool create_task(const std::string& task_id,
                    const std::string& data_batch,
                    double tokens_reward);

    /**
     * @brief Get one pending task
     * @return Task object if found, std::nullopt if no pending tasks
     */
    std::optional<Task> get_pending_task();

    /**
     * @brief Assign a task to a worker
     * @param task_id Task to assign
     * @param user_id User to assign to
     * @return true if successful
     */
    bool assign_task(const std::string& task_id, const std::string& user_id);

    /**
     * @brief Mark a task as completed
     * @param task_id Task to complete
     * @param result Training results (JSON string)
     * @return true if successful
     */
    bool complete_task(const std::string& task_id, const std::string& result);

    /**
     * @brief Get all tasks for a user
     * @param user_id User to query
     * @param status Filter by status (empty string = all statuses)
     * @return Vector of tasks
     */
    std::vector<Task> get_user_tasks(const std::string& user_id,
                                     const std::string& status = "");

    // =========================================================================
    // Transaction Operations
    // =========================================================================

    /**
     * @brief Get transaction history for a user
     * @param user_id User to query
     * @param limit Maximum number of transactions to return (0 = no limit)
     * @return Vector of transactions (newest first)
     */
    std::vector<Transaction> get_transactions(const std::string& user_id,
                                             int limit = 0);

    /**
     * @brief Get user statistics
     * @param user_id User to query
     * @return User object with stats if found
     */
    std::optional<User> get_user_stats(const std::string& user_id);

private:
    sqlite3* db_{nullptr};         // SQLite database connection

    /**
     * @brief Initialize database tables
     * Creates tables if they don't exist
     */
    void create_tables();

    /**
     * @brief Execute a SQL statement without results
     * @param sql SQL statement to execute
     * @return true if successful
     */
    bool execute(const std::string& sql);

    /**
     * @brief Get current timestamp in ISO 8601 format
     * @return ISO 8601 timestamp string
     */
    static std::string current_timestamp();
};

} // namespace hydra
