/**
 * @file coding_assistant_example.cpp
 * @brief Example usage of HydraAI Coding Assistant
 *
 * This demonstrates all major features:
 * - File system access
 * - Conversation memory
 * - Code generation
 * - Project analysis
 * - Semantic search
 */

#include "hydra/coding_assistant.hpp"
#include <iostream>
#include <iomanip>

using namespace hydra::assistant;

void print_separator(const std::string& title = "") {
    std::cout << "\n" << std::string(60, '=') << "\n";
    if (!title.empty()) {
        std::cout << title << "\n";
        std::cout << std::string(60, '=') << "\n";
    }
}

/**
 * Example 1: Basic file operations
 */
void example_file_operations() {
    print_separator("Example 1: File Operations");

    // Set up permissions (sandbox)
    FilePermissions perms;
    perms.allowed_directories = {"/home/user/my_project"};
    perms.allow_read = true;
    perms.allow_write = true;  // Requires confirmation

    CodingAssistant assistant(perms);
    assistant.initialize();

    // Read a file
    std::cout << "Reading file...\n";
    auto file = assistant.read_file("/home/user/my_project/src/main.cpp");

    if (file) {
        std::cout << "✓ File loaded successfully!\n";
        std::cout << "  Language: " << file->language << "\n";
        std::cout << "  Lines: " << file->line_count << "\n";
        std::cout << "  Functions: " << file->functions.size() << "\n";
        for (const auto& func : file->functions) {
            std::cout << "    - " << func << "()\n";
        }
    }

    // Search files semantically
    std::cout << "\nSearching for database-related code...\n";
    auto results = assistant.search_project("database connection and queries");

    std::cout << "✓ Found " << results.size() << " relevant files:\n";
    for (size_t i = 0; i < std::min(size_t(5), results.size()); ++i) {
        std::cout << "  " << (i + 1) << ". " << results[i].path << "\n";
    }
}

/**
 * Example 2: Conversational interaction
 */
void example_conversation() {
    print_separator("Example 2: Conversation");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Start conversation
    auto conv_id = assistant.start_conversation("/home/user/my_project");
    std::cout << "Started conversation: " << conv_id << "\n\n";

    // Conversation turn 1
    std::cout << "You: Tell me about the database layer\n";
    auto response1 = assistant.chat(conv_id,
        "Tell me about the database layer");
    std::cout << "AI: " << response1 << "\n\n";

    // Conversation turn 2 (remembers context)
    std::cout << "You: How is it used in the coordinator?\n";
    auto response2 = assistant.chat(conv_id,
        "How is it used in the coordinator?");
    std::cout << "AI: " << response2 << "\n\n";

    // Conversation turn 3
    std::cout << "You: Can you show me an example?\n";
    auto response3 = assistant.chat(conv_id,
        "Can you show me an example?");
    std::cout << "AI: " << response3 << "\n";
}

/**
 * Example 3: Code generation
 */
void example_code_generation() {
    print_separator("Example 3: Code Generation");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Generate a function
    std::cout << "Requesting: Binary search tree implementation\n\n";

    auto code = assistant.generate_code(
        "Implement a binary search tree with insert, find, and delete operations",
        "cpp"
    );

    std::cout << "Generated Code:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << code.code << "\n";
    std::cout << std::string(60, '-') << "\n\n";

    std::cout << "Explanation:\n" << code.explanation << "\n\n";

    std::cout << "Metadata:\n";
    std::cout << "  Language: " << code.language << "\n";
    std::cout << "  Confidence: " << (code.confidence * 100) << "%\n";
    std::cout << "  Complete: " << (code.is_complete ? "Yes" : "No") << "\n";

    if (!code.dependencies.empty()) {
        std::cout << "  Dependencies:\n";
        for (const auto& dep : code.dependencies) {
            std::cout << "    - " << dep << "\n";
        }
    }

    if (!code.warnings.empty()) {
        std::cout << "  Warnings:\n";
        for (const auto& warning : code.warnings) {
            std::cout << "    ⚠️  " << warning << "\n";
        }
    }

    // Optionally save to file (requires user confirmation)
    std::cout << "\nSave to file? (This would prompt user in real usage)\n";
    // assistant.write_code_to_file("bst.hpp", code);
}

/**
 * Example 4: Code fixing
 */
void example_code_fixing() {
    print_separator("Example 4: Code Fixing");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Broken code with error
    std::string broken_code = R"(
def calculate_average(numbers):
    total = 0
    for num in numbers
        total += num
    return total / len(numbers)
)";

    std::string error = "SyntaxError: invalid syntax at line 3";

    std::cout << "Broken Code:\n" << broken_code << "\n";
    std::cout << "Error: " << error << "\n\n";

    // Fix it
    auto fixed = assistant.fix_code(broken_code, error);

    std::cout << "Fixed Code:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << fixed.code << "\n";
    std::cout << std::string(60, '-') << "\n\n";

    std::cout << "Explanation:\n" << fixed.explanation << "\n";
}

/**
 * Example 5: Code explanation
 */
void example_code_explanation() {
    print_separator("Example 5: Code Explanation");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Complex code to explain
    std::string complex_code = R"(
template<typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(value));
        cond_.notify_one();
    }

    bool try_pop(T& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) return false;
        value = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    void wait_and_pop(T& value) {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [this]{ return !queue_.empty(); });
        value = std::move(queue_.front());
        queue_.pop();
    }
};
)";

    std::cout << "Code to explain:\n" << complex_code << "\n\n";

    auto explanation = assistant.explain_code(complex_code);

    std::cout << "Explanation:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << explanation << "\n";
    std::cout << std::string(60, '-') << "\n";
}

/**
 * Example 6: Project analysis
 */
void example_project_analysis() {
    print_separator("Example 6: Project Analysis");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    std::cout << "Analyzing project...\n\n";

    auto analysis = assistant.analyze_codebase("/home/user/my_project");

    std::cout << "Project Statistics:\n";
    std::cout << "  Total Files: " << analysis.total_files << "\n";
    std::cout << "  Total Lines: " << analysis.total_lines << "\n\n";

    std::cout << "Languages:\n";
    for (const auto& [lang, count] : analysis.languages) {
        std::cout << "  " << std::left << std::setw(15) << lang
                  << ": " << count << " files\n";
    }

    std::cout << "\nDependencies:\n";
    for (const auto& dep : analysis.dependencies) {
        std::cout << "  - " << dep << "\n";
    }

    if (!analysis.potential_issues.empty()) {
        std::cout << "\nPotential Issues:\n";
        for (const auto& issue : analysis.potential_issues) {
            std::cout << "  ⚠️  " << issue << "\n";
        }
    }

    std::cout << "\nArchitecture Summary:\n";
    std::cout << std::string(60, '-') << "\n";
    std::cout << analysis.architecture_summary << "\n";
    std::cout << std::string(60, '-') << "\n";
}

/**
 * Example 7: Security audit
 */
void example_security_audit() {
    print_separator("Example 7: Security Audit");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    std::cout << "Running security audit...\n\n";

    auto vulnerabilities = assistant.security_audit("/home/user/my_project");

    if (vulnerabilities.empty()) {
        std::cout << "✓ No security vulnerabilities found!\n";
    } else {
        std::cout << "Found " << vulnerabilities.size() << " potential issues:\n\n";

        for (size_t i = 0; i < vulnerabilities.size(); ++i) {
            std::cout << (i + 1) << ". 🔒 " << vulnerabilities[i] << "\n";
        }
    }
}

/**
 * Example 8: Multi-language support
 */
void example_multi_language() {
    print_separator("Example 8: Multi-Language Support");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Generate same functionality in different languages
    std::string description = "Function to check if a string is a palindrome";

    std::vector<std::string> languages = {"cpp", "python", "javascript", "rust"};

    for (const auto& lang : languages) {
        std::cout << "\n" << lang << ":\n";
        std::cout << std::string(40, '-') << "\n";

        auto code = assistant.generate_code(description, lang);
        std::cout << code.code << "\n";
    }
}

/**
 * Example 9: Interactive session (simulated)
 */
void example_interactive_session() {
    print_separator("Example 9: Interactive Session");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Index project for better responses
    std::cout << "Indexing project for semantic search...\n";
    assistant.index_project("/home/user/my_project");
    std::cout << "✓ Project indexed!\n\n";

    auto conv_id = assistant.start_conversation("/home/user/my_project");

    // Simulated interactive session
    struct Turn {
        std::string user_msg;
        std::string ai_response_preview;
    };

    std::vector<Turn> session = {
        {
            "What files handle database operations?",
            "Database operations are handled in:\n  - src/core/database.cpp\n  - include/hydra/database.hpp\n..."
        },
        {
            "Can you explain the transaction system?",
            "The transaction system uses SQLite transactions for ACID properties...\n..."
        },
        {
            "I need to add a new table for user sessions. Can you help?",
            "I'll create a sessions table. Here's the SQL and C++ code:\n..."
        },
        {
            "Write the code and save it",
            "✓ Generated:\n  - src/core/sessions.cpp\n  - include/hydra/sessions.hpp\n  - migrations/001_add_sessions.sql\n..."
        }
    };

    for (const auto& turn : session) {
        std::cout << "You: " << turn.user_msg << "\n";
        // In real usage: auto response = assistant.chat(conv_id, turn.user_msg);
        std::cout << "AI:  " << turn.ai_response_preview << "\n\n";
    }
}

/**
 * Example 10: Advanced features
 */
void example_advanced_features() {
    print_separator("Example 10: Advanced Features");

    FilePermissions perms = create_safe_permissions("/home/user/my_project");
    CodingAssistant assistant(perms);
    assistant.initialize();

    // Code completion
    std::cout << "1. Code Completion:\n";
    std::string partial = "def fibonacci(n):\n    if n <= 1:\n        return n\n    ";
    auto completed = assistant.complete_code(partial);
    std::cout << "Completed code:\n" << completed << "\n\n";

    // Suggest improvements
    std::cout << "2. Code Improvements:\n";
    auto suggestions = assistant.suggest_improvements(
        "/home/user/my_project/src/old_code.cpp"
    );
    for (const auto& suggestion : suggestions) {
        std::cout << "  💡 " << suggestion << "\n";
    }
    std::cout << "\n";

    // Generate documentation
    std::cout << "3. Documentation Generation:\n";
    auto docs = assistant.generate_docs(
        "/home/user/my_project/include/hydra/model.hpp"
    );
    std::cout << docs << "\n";
}

/**
 * Main function - runs all examples
 */
int main(int argc, char* argv[]) {
    std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║           HydraAI Coding Assistant Examples               ║
║                                                           ║
║  Demonstrating intelligent code understanding,            ║
║  generation, and project analysis                         ║
╚═══════════════════════════════════════════════════════════╝
)" << "\n";

    try {
        // Run examples
        example_file_operations();
        example_conversation();
        example_code_generation();
        example_code_fixing();
        example_code_explanation();
        example_project_analysis();
        example_security_audit();
        example_multi_language();
        example_interactive_session();
        example_advanced_features();

        print_separator("All Examples Completed Successfully!");

        std::cout << R"(
Next Steps:
1. Explore the full API in include/hydra/coding_assistant.hpp
2. Read the documentation in docs/CODING_ASSISTANT.md
3. Try the interactive GUI: ./hydra_assistant --interactive
4. Integrate into your IDE for real-time assistance

Happy coding! 🚀
)" << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
