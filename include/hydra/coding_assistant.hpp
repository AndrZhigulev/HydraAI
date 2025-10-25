/**
 * @file coding_assistant.hpp
 * @brief Intelligent coding assistant with file access, memory, and code generation
 *
 * This extends HydraAI with powerful coding capabilities:
 * - Read/write files on user's PC (with permission)
 * - Remember all conversations and context
 * - Generate high-quality code in multiple languages
 * - Search through files using semantic understanding
 * - Learn from user's codebase
 */

#pragma once

#include "model.hpp"
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <filesystem>
#include <map>

namespace hydra::assistant {

namespace fs = std::filesystem;

/**
 * @struct FilePermissions
 * @brief Controls what the AI can access on the file system
 */
struct FilePermissions {
    std::vector<fs::path> allowed_directories;   // Directories AI can access
    std::vector<fs::path> forbidden_paths;       // Explicitly forbidden paths

    bool allow_read{true};                       // Can read files
    bool allow_write{false};                     // Can write files (requires user confirmation)
    bool allow_execute{false};                   // Can execute code (very dangerous!)

    size_t max_file_size_mb{10};                 // Maximum file size to read
    size_t max_files_per_query{50};              // Max files to process at once

    // Forbidden patterns (for security)
    std::vector<std::string> forbidden_patterns{
        "*.key", "*.pem",                        // Private keys
        "*.password", "*.secret",                // Secrets
        ".env", "credentials.json",              // Credentials
        "id_rsa", "id_ed25519",                  // SSH keys
        "wallet.dat"                             // Crypto wallets
    };
};

/**
 * @struct Message
 * @brief A single message in a conversation
 */
struct Message {
    enum class Role {
        USER,           // Message from user
        ASSISTANT,      // Message from AI
        SYSTEM          // System message (context, instructions)
    };

    Role role;
    std::string content;
    uint64_t timestamp{0};

    // Optional metadata
    std::map<std::string, std::string> metadata;
    std::vector<std::string> referenced_files;   // Files mentioned in this message
    std::string code_language;                   // If message contains code
};

/**
 * @struct Conversation
 * @brief A complete conversation thread
 */
struct Conversation {
    std::string conversation_id;
    std::string title;                           // Generated summary
    std::vector<Message> messages;
    uint64_t created_at{0};
    uint64_t updated_at{0};

    // Context
    std::vector<fs::path> active_files;          // Files being worked on
    std::string project_path;                    // Current project directory
    std::map<std::string, std::string> variables; // Session variables
};

/**
 * @struct FileContext
 * @brief Information about a file for the AI
 */
struct FileContext {
    fs::path path;
    std::string content;
    std::string language;                        // Programming language
    std::vector<std::string> imports;            // Import statements
    std::vector<std::string> functions;          // Function names
    std::vector<std::string> classes;            // Class names
    size_t line_count{0};
    size_t char_count{0};
    uint64_t last_modified{0};

    // Semantic embedding for search
    std::vector<float> embedding;                // Vector representation
};

/**
 * @struct CodeSnippet
 * @brief Generated code with metadata
 */
struct CodeSnippet {
    std::string code;
    std::string language;
    std::string explanation;
    std::vector<std::string> dependencies;       // Required libraries
    std::string filename;                        // Suggested filename
    bool is_complete{false};                     // Full file vs snippet

    // Quality metrics
    float confidence{0.0f};                      // 0.0 to 1.0
    std::vector<std::string> warnings;           // Potential issues
};

/**
 * @class FileSystemAccess
 * @brief Safe file system access with permissions
 */
class FileSystemAccess {
public:
    explicit FileSystemAccess(const FilePermissions& permissions);

    /**
     * @brief Check if path is allowed
     */
    bool is_allowed(const fs::path& path) const;

    /**
     * @brief Read file contents
     * @return File contents or error message
     */
    std::optional<std::string> read_file(const fs::path& path);

    /**
     * @brief Write to file (requires user confirmation)
     * @return true if successful
     */
    bool write_file(const fs::path& path, const std::string& content);

    /**
     * @brief List files in directory
     */
    std::vector<fs::path> list_directory(const fs::path& dir,
                                         const std::string& pattern = "*");

    /**
     * @brief Search files by content
     * @param query Search query
     * @param directory Where to search
     * @return Matching files with snippets
     */
    std::vector<std::pair<fs::path, std::string>>
        search_files(const std::string& query, const fs::path& directory);

    /**
     * @brief Get file metadata
     */
    std::optional<FileContext> analyze_file(const fs::path& path);

    /**
     * @brief Create new file (requires confirmation)
     */
    bool create_file(const fs::path& path, const std::string& content);

    /**
     * @brief Delete file (requires confirmation)
     */
    bool delete_file(const fs::path& path);

private:
    FilePermissions permissions_;

    bool matches_forbidden_pattern(const fs::path& path) const;
    std::string detect_language(const fs::path& path) const;
};

/**
 * @class ConversationMemory
 * @brief Long-term memory for conversations
 */
class ConversationMemory {
public:
    explicit ConversationMemory(const std::string& db_path = "./memory.db");

    /**
     * @brief Start a new conversation
     */
    std::string create_conversation(const std::string& title = "");

    /**
     * @brief Add message to conversation
     */
    void add_message(const std::string& conversation_id, const Message& message);

    /**
     * @brief Get full conversation
     */
    std::optional<Conversation> get_conversation(const std::string& id);

    /**
     * @brief Get all conversations
     */
    std::vector<Conversation> list_conversations(size_t limit = 50);

    /**
     * @brief Search conversations by content
     */
    std::vector<Conversation> search_conversations(const std::string& query);

    /**
     * @brief Get conversation summary
     */
    std::string summarize_conversation(const std::string& conversation_id);

    /**
     * @brief Delete conversation
     */
    bool delete_conversation(const std::string& conversation_id);

    /**
     * @brief Get recent context (last N messages across all conversations)
     * Useful for maintaining continuity
     */
    std::vector<Message> get_recent_context(size_t limit = 10);

private:
    std::string db_path_;

    // In-memory cache of recent conversations
    std::map<std::string, Conversation> cache_;
};

/**
 * @class VectorDatabase
 * @brief Semantic search using embeddings
 */
class VectorDatabase {
public:
    explicit VectorDatabase(const std::string& db_path = "./vectors.db");

    /**
     * @brief Add file to vector database
     */
    void index_file(const fs::path& path, const FileContext& context);

    /**
     * @brief Search for similar files/content
     * @param query Natural language query
     * @param limit Maximum results
     * @return Files ranked by relevance
     */
    std::vector<FileContext> semantic_search(const std::string& query,
                                             size_t limit = 10);

    /**
     * @brief Find similar code snippets
     */
    std::vector<std::string> find_similar_code(const std::string& code,
                                               const std::string& language);

    /**
     * @brief Index entire directory
     */
    void index_directory(const fs::path& directory, bool recursive = true);

    /**
     * @brief Get embedding for text
     */
    std::vector<float> get_embedding(const std::string& text);

private:
    std::string db_path_;

    // Vector index (could use FAISS, Annoy, or custom implementation)
    struct VectorIndex;
    std::unique_ptr<VectorIndex> index_;
};

/**
 * @class CodeGenerator
 * @brief Specialized code generation engine
 */
class CodeGenerator {
public:
    CodeGenerator();

    /**
     * @brief Generate code from description
     * @param description What the code should do
     * @param language Target programming language
     * @param context Additional context (existing code, files, etc.)
     * @return Generated code snippet
     */
    CodeSnippet generate_code(const std::string& description,
                             const std::string& language,
                             const std::vector<FileContext>& context = {});

    /**
     * @brief Complete partial code
     */
    std::string complete_code(const std::string& partial_code,
                             const std::string& language);

    /**
     * @brief Fix code errors
     */
    CodeSnippet fix_code(const std::string& broken_code,
                        const std::string& error_message,
                        const std::string& language);

    /**
     * @brief Explain code
     */
    std::string explain_code(const std::string& code,
                            const std::string& language);

    /**
     * @brief Refactor code
     */
    CodeSnippet refactor_code(const std::string& code,
                             const std::string& refactoring_goal);

    /**
     * @brief Convert code between languages
     */
    CodeSnippet translate_code(const std::string& code,
                              const std::string& from_language,
                              const std::string& to_language);

    /**
     * @brief Generate tests for code
     */
    CodeSnippet generate_tests(const std::string& code,
                              const std::string& language);

private:
    // Specialized models for different languages
    std::map<std::string, std::shared_ptr<Model>> language_models_;

    // Code-specific tokenizer
    class CodeTokenizer;
    std::unique_ptr<CodeTokenizer> tokenizer_;
};

/**
 * @class RAGPipeline
 * @brief Retrieval-Augmented Generation for context-aware responses
 */
class RAGPipeline {
public:
    RAGPipeline(VectorDatabase& vector_db, FileSystemAccess& fs_access);

    /**
     * @brief Generate response with retrieved context
     * @param query User's question/request
     * @param conversation Current conversation for context
     * @return Response with cited sources
     */
    struct RAGResponse {
        std::string answer;
        std::vector<FileContext> sources;        // Retrieved files used
        std::vector<std::string> citations;      // References
        float confidence{0.0f};
    };

    RAGResponse generate_response(const std::string& query,
                                  const Conversation& conversation);

    /**
     * @brief Retrieve relevant context for query
     */
    std::vector<FileContext> retrieve_context(const std::string& query,
                                              size_t max_results = 5);

private:
    VectorDatabase& vector_db_;
    FileSystemAccess& fs_access_;
};

/**
 * @class CodingAssistant
 * @brief Main intelligent coding assistant
 */
class CodingAssistant {
public:
    /**
     * @brief Constructor
     * @param permissions File system permissions
     */
    explicit CodingAssistant(const FilePermissions& permissions);

    /**
     * @brief Initialize assistant
     */
    bool initialize();

    // =========================================================================
    // Conversation Interface
    // =========================================================================

    /**
     * @brief Start new conversation
     * @param project_path Optional project directory
     * @return Conversation ID
     */
    std::string start_conversation(const std::string& project_path = "");

    /**
     * @brief Send message and get response
     * @param conversation_id Active conversation
     * @param message User's message
     * @return Assistant's response
     */
    std::string chat(const std::string& conversation_id,
                    const std::string& message);

    /**
     * @brief Get conversation history
     */
    Conversation get_conversation(const std::string& conversation_id);

    // =========================================================================
    // File Operations
    // =========================================================================

    /**
     * @brief Read and analyze file
     */
    std::optional<FileContext> read_file(const fs::path& path);

    /**
     * @brief Write code to file (with user confirmation)
     */
    bool write_code_to_file(const fs::path& path, const CodeSnippet& code);

    /**
     * @brief Search files in project
     */
    std::vector<FileContext> search_project(const std::string& query);

    /**
     * @brief Index project for faster search
     */
    void index_project(const fs::path& project_root);

    // =========================================================================
    // Code Generation
    // =========================================================================

    /**
     * @brief Generate code from description
     */
    CodeSnippet generate_code(const std::string& description,
                             const std::string& language);

    /**
     * @brief Fix code with error
     */
    CodeSnippet fix_code(const std::string& code,
                        const std::string& error);

    /**
     * @brief Explain code snippet
     */
    std::string explain_code(const std::string& code);

    /**
     * @brief Complete partial code
     */
    std::string complete_code(const std::string& partial_code);

    // =========================================================================
    // Advanced Features
    // =========================================================================

    /**
     * @brief Analyze entire codebase
     */
    struct CodebaseAnalysis {
        size_t total_files{0};
        size_t total_lines{0};
        std::map<std::string, size_t> languages;
        std::vector<std::string> dependencies;
        std::vector<std::string> potential_issues;
        std::string architecture_summary;
    };

    CodebaseAnalysis analyze_codebase(const fs::path& project_root);

    /**
     * @brief Suggest improvements
     */
    std::vector<std::string> suggest_improvements(const fs::path& file);

    /**
     * @brief Find security vulnerabilities
     */
    std::vector<std::string> security_audit(const fs::path& project_root);

    /**
     * @brief Generate documentation
     */
    std::string generate_docs(const fs::path& file);

    // =========================================================================
    // Settings
    // =========================================================================

    /**
     * @brief Set programming language preference
     */
    void set_preferred_language(const std::string& language);

    /**
     * @brief Set code style
     */
    void set_code_style(const std::string& style);  // e.g., "google", "pep8"

    /**
     * @brief Enable/disable features
     */
    void enable_auto_complete(bool enable);
    void enable_auto_fix(bool enable);
    void enable_suggestions(bool enable);

private:
    FilePermissions permissions_;
    std::unique_ptr<FileSystemAccess> fs_access_;
    std::unique_ptr<ConversationMemory> memory_;
    std::unique_ptr<VectorDatabase> vector_db_;
    std::unique_ptr<CodeGenerator> code_gen_;
    std::unique_ptr<RAGPipeline> rag_;

    // Active conversations
    std::map<std::string, Conversation> active_conversations_;

    // Settings
    std::string preferred_language_;
    std::string code_style_;
    bool auto_complete_{true};
    bool auto_fix_{false};
    bool suggestions_{true};

    // Internal methods
    void update_context(Conversation& conversation, const std::string& message);
    std::vector<FileContext> get_relevant_files(const std::string& query,
                                                const Conversation& conv);
};

/**
 * @brief Helper function to create default permissions
 */
FilePermissions create_safe_permissions(const fs::path& workspace_root);

/**
 * @brief Helper function to detect programming language from code
 */
std::string detect_language_from_code(const std::string& code);

/**
 * @brief Helper function to extract code blocks from markdown
 */
std::vector<CodeSnippet> extract_code_blocks(const std::string& markdown);

} // namespace hydra::assistant
