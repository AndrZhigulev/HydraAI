"""
database.py - Database Management for HydraAI
==============================================
This file manages all database operations for storing user data, tokens, and training tasks.
We use SQLite because it's simple - just a file, no complex setup needed!
"""

import sqlite3  # Built-in Python library for simple databases
import json      # For storing complex data as text
from datetime import datetime  # For timestamps


class HydraDatabase:
    """
    This class handles all database operations.
    Think of it as a librarian that organizes and retrieves information.
    """

    def __init__(self, db_path="hydra.db"):
        """
        Initialize the database connection.

        Args:
            db_path: Where to store the database file (default: hydra.db)

        Explanation:
            - Creates a connection to the database file
            - If the file doesn't exist, SQLite creates it automatically
        """
        self.db_path = db_path  # Save the path for later use
        self.connection = sqlite3.connect(db_path, check_same_thread=False)
        # check_same_thread=False allows multiple threads to use this connection

        self.connection.row_factory = sqlite3.Row
        # This makes results come back as dictionaries instead of tuples
        # Example: you can do row['username'] instead of row[0]

        self.create_tables()  # Set up the database structure

    def create_tables(self):
        """
        Create the database tables if they don't exist.

        Tables are like spreadsheets - they organize data in rows and columns.
        """
        cursor = self.connection.cursor()  # Cursor executes SQL commands

        # Table 1: USERS - Stores user accounts
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS users (
                user_id TEXT PRIMARY KEY,     -- Unique ID for each user (like a username)
                created_at TEXT,               -- When the account was created
                total_tokens REAL DEFAULT 0,   -- How many Hydra tokens they have (REAL = decimal number)
                total_work_done INTEGER DEFAULT 0  -- How many training tasks they completed
            )
        """)

        # Table 2: TASKS - Stores training tasks
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS tasks (
                task_id TEXT PRIMARY KEY,      -- Unique ID for each task
                created_at TEXT,               -- When task was created
                assigned_to TEXT,              -- Which user is working on it (NULL if unassigned)
                status TEXT,                   -- Status: 'pending', 'assigned', 'completed', 'failed'
                data_batch TEXT,               -- The training data (stored as JSON text)
                result TEXT,                   -- The trained model updates (stored as JSON text)
                tokens_reward REAL,            -- How many tokens this task is worth
                completed_at TEXT              -- When task was finished (NULL if not finished)
            )
        """)

        # Table 3: TRANSACTIONS - Log of all token movements
        cursor.execute("""
            CREATE TABLE IF NOT EXISTS transactions (
                transaction_id INTEGER PRIMARY KEY AUTOINCREMENT,  -- Auto-incrementing ID
                user_id TEXT,                  -- Which user
                amount REAL,                   -- How many tokens (positive = earned, negative = spent)
                type TEXT,                     -- Type: 'reward', 'query', 'trade'
                description TEXT,              -- What this transaction was for
                timestamp TEXT                 -- When it happened
            )
        """)

        self.connection.commit()  # Save changes to the database

    def create_user(self, user_id):
        """
        Create a new user account.

        Args:
            user_id: Unique identifier for the user (like a username)

        Returns:
            True if successful, False if user already exists
        """
        try:
            cursor = self.connection.cursor()
            cursor.execute("""
                INSERT INTO users (user_id, created_at, total_tokens, total_work_done)
                VALUES (?, ?, ?, ?)
            """, (
                user_id,                           # The user's ID
                datetime.now().isoformat(),        # Current time in ISO format
                0,                                  # Start with 0 tokens
                0                                   # Start with 0 work done
            ))
            # The ? marks are placeholders - SQLite safely inserts the values
            # This prevents SQL injection attacks!

            self.connection.commit()  # Save the changes
            return True
        except sqlite3.IntegrityError:
            # This error happens if user_id already exists (PRIMARY KEY constraint)
            return False

    def get_user(self, user_id):
        """
        Get information about a user.

        Args:
            user_id: The user to look up

        Returns:
            Dictionary with user info, or None if user doesn't exist
        """
        cursor = self.connection.cursor()
        cursor.execute("SELECT * FROM users WHERE user_id = ?", (user_id,))
        row = cursor.fetchone()  # Get one result

        if row:
            # Convert the Row object to a regular dictionary
            return dict(row)
        return None

    def add_tokens(self, user_id, amount, transaction_type, description):
        """
        Add (or subtract) tokens from a user's balance.

        Args:
            user_id: Which user
            amount: How many tokens (positive to add, negative to subtract)
            transaction_type: 'reward', 'query', or 'trade'
            description: Human-readable description of this transaction

        Returns:
            True if successful, False if failed
        """
        cursor = self.connection.cursor()

        # First, update the user's token balance
        cursor.execute("""
            UPDATE users
            SET total_tokens = total_tokens + ?
            WHERE user_id = ?
        """, (amount, user_id))

        # Then, log this transaction for transparency
        cursor.execute("""
            INSERT INTO transactions (user_id, amount, type, description, timestamp)
            VALUES (?, ?, ?, ?, ?)
        """, (
            user_id,
            amount,
            transaction_type,
            description,
            datetime.now().isoformat()
        ))

        self.connection.commit()  # Save changes
        return True

    def create_task(self, task_id, data_batch, tokens_reward):
        """
        Create a new training task.

        Args:
            task_id: Unique ID for this task
            data_batch: The training data (will be converted to JSON)
            tokens_reward: How many tokens workers earn for completing this

        Returns:
            True if successful
        """
        cursor = self.connection.cursor()
        cursor.execute("""
            INSERT INTO tasks (task_id, created_at, status, data_batch, tokens_reward)
            VALUES (?, ?, ?, ?, ?)
        """, (
            task_id,
            datetime.now().isoformat(),
            'pending',                      # New tasks start as 'pending'
            json.dumps(data_batch),         # Convert Python data to JSON text
            tokens_reward
        ))

        self.connection.commit()
        return True

    def get_pending_task(self):
        """
        Get one pending task that needs to be worked on.

        Returns:
            Dictionary with task info, or None if no pending tasks
        """
        cursor = self.connection.cursor()
        cursor.execute("""
            SELECT * FROM tasks
            WHERE status = 'pending'
            LIMIT 1
        """)
        # LIMIT 1 means "just give me one result"

        row = cursor.fetchone()
        if row:
            return dict(row)
        return None

    def assign_task(self, task_id, user_id):
        """
        Assign a task to a specific worker.

        Args:
            task_id: Which task
            user_id: Which user is working on it
        """
        cursor = self.connection.cursor()
        cursor.execute("""
            UPDATE tasks
            SET status = 'assigned', assigned_to = ?
            WHERE task_id = ?
        """, (user_id, task_id))

        self.connection.commit()

    def complete_task(self, task_id, result):
        """
        Mark a task as completed and save the results.

        Args:
            task_id: Which task
            result: The training results (will be converted to JSON)
        """
        cursor = self.connection.cursor()
        cursor.execute("""
            UPDATE tasks
            SET status = 'completed',
                result = ?,
                completed_at = ?
            WHERE task_id = ?
        """, (
            json.dumps(result),             # Save result as JSON
            datetime.now().isoformat(),
            task_id
        ))

        self.connection.commit()

    def get_user_stats(self, user_id):
        """
        Get statistics about a user's activity.

        Args:
            user_id: Which user

        Returns:
            Dictionary with stats (tokens, tasks completed, etc.)
        """
        user = self.get_user(user_id)
        if not user:
            return None

        cursor = self.connection.cursor()

        # Count how many tasks this user completed
        cursor.execute("""
            SELECT COUNT(*) as completed_tasks
            FROM tasks
            WHERE assigned_to = ? AND status = 'completed'
        """, (user_id,))

        task_count = cursor.fetchone()['completed_tasks']

        return {
            'user_id': user_id,
            'total_tokens': user['total_tokens'],
            'tasks_completed': task_count,
            'member_since': user['created_at']
        }

    def close(self):
        """
        Close the database connection.
        Always call this when you're done!
        """
        self.connection.close()


# Example usage (for testing):
if __name__ == "__main__":
    # This code only runs if you run this file directly
    print("Testing HydraDatabase...")

    # Create a database
    db = HydraDatabase("test_hydra.db")

    # Create a test user
    db.create_user("user123")
    print("Created user: user123")

    # Get user info
    user = db.get_user("user123")
    print(f"User info: {user}")

    # Add some tokens
    db.add_tokens("user123", 10.5, "reward", "Completed training task")
    print("Added 10.5 tokens")

    # Check balance
    user = db.get_user("user123")
    print(f"New balance: {user['total_tokens']} tokens")

    # Create a task
    db.create_task("task001", {"data": [1, 2, 3]}, 5.0)
    print("Created task: task001")

    # Get stats
    stats = db.get_user_stats("user123")
    print(f"User stats: {stats}")

    db.close()
    print("\nDatabase test completed!")
