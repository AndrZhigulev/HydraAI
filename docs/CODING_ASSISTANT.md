# 🤖 HydraAI Coding Assistant

**An intelligent AI that understands your code, remembers your conversations, and writes excellent code.**

---

## 🎯 What Makes This Special?

### Traditional AI Assistants

```
❌ No file access (can't see your code)
❌ No memory (forgets previous conversations)
❌ Generic responses (not tailored to your project)
❌ Limited languages (focuses on one or two)
```

### HydraAI Coding Assistant

```
✅ Full file system access (with permission)
✅ Perfect memory (remembers all conversations)
✅ Project-aware (understands your codebase)
✅ Multi-language (Python, C++, JavaScript, Rust, Go, etc.)
✅ Code generation, completion, fixing, explaining
✅ Semantic search (find relevant code)
✅ Security auditing
```

---

## 🚀 Quick Start

### 1. Initialize with Workspace

```cpp
#include "hydra/coding_assistant.hpp"

using namespace hydra::assistant;

// Set up permissions
FilePermissions perms;
perms.allowed_directories = {"/home/user/my_project"};
perms.allow_read = true;
perms.allow_write = true;  // Requires confirmation for each write

// Create assistant
CodingAssistant assistant(perms);
assistant.initialize();

// Index your project for faster search
assistant.index_project("/home/user/my_project");
```

### 2. Start Conversing

```cpp
// Start conversation
std::string conv_id = assistant.start_conversation("/home/user/my_project");

// Ask questions
auto response = assistant.chat(conv_id,
    "What does the DatabaseManager class do?");
// → "The DatabaseManager class handles all SQLite operations. It's defined
//    in src/core/database.cpp:15. It provides methods for CRUD operations..."

// Request code
response = assistant.chat(conv_id,
    "Write a function to calculate Fibonacci numbers");
// → Generates optimized Fibonacci function with explanation

// Get help with errors
response = assistant.chat(conv_id,
    "I'm getting 'segmentation fault' in line 42 of main.cpp. Can you help?");
// → Analyzes the file, finds the issue, explains it, and suggests fix
```

### 3. Generate Code

```cpp
// Generate complete function
auto code = assistant.generate_code(
    "Create a binary search tree with insert, delete, and search",
    "cpp"
);

std::cout << "Code:\n" << code.code << "\n";
std::cout << "Explanation:\n" << code.explanation << "\n";

// Save to file (asks user for confirmation)
assistant.write_code_to_file("bst.cpp", code);
```

---

## 🧠 Core Features

### 1. File System Access (Sandboxed)

**Safe file operations with user permissions:**

```cpp
// Read file
auto file_context = assistant.read_file("src/main.cpp");
if (file_context) {
    std::cout << "Language: " << file_context->language << "\n";
    std::cout << "Functions: ";
    for (auto& fn : file_context->functions) {
        std::cout << fn << " ";
    }
}

// Search files
auto results = assistant.search_project("database connection");
// → Finds all files mentioning database connections

// Write code to file (with confirmation)
CodeSnippet code = assistant.generate_code(...);
assistant.write_code_to_file("output.cpp", code);
// → Prompts: "Allow writing to output.cpp? [y/N]"
```

**Security Features:**
- Sandboxed access (only allowed directories)
- Automatic blocking of sensitive files (.env, *.key, wallet.dat, etc.)
- User confirmation for writes
- No execute permissions by default
- Size limits (won't read huge files)

### 2. Perfect Memory

**The assistant remembers everything:**

```cpp
// Session 1
assistant.chat(conv_id, "I'm working on a REST API in Python");
assistant.chat(conv_id, "It uses Flask and PostgreSQL");

// Session 2 (hours later, same conv_id)
assistant.chat(conv_id, "How should I handle authentication?");
// → "For your Flask REST API with PostgreSQL, I recommend JWT tokens..."
//    (Remembers you're using Flask and PostgreSQL!)

// Search old conversations
auto convs = memory.search_conversations("authentication");
// → Finds all past conversations about authentication
```

**What's Stored:**
- All messages (user and assistant)
- Referenced files
- Project context
- Code snippets
- Timestamps

**Benefits:**
- Continuity across sessions
- Learn from past interactions
- Reference old solutions
- Build project knowledge over time

### 3. Code Generation

**Multi-language support:**

```cpp
// Python
auto py_code = assistant.generate_code(
    "Create a Flask API endpoint for user registration",
    "python"
);

// C++
auto cpp_code = assistant.generate_code(
    "Implement a thread-safe queue using std::mutex",
    "cpp"
);

// JavaScript
auto js_code = assistant.generate_code(
    "React component for a todo list",
    "javascript"
);

// Rust
auto rust_code = assistant.generate_code(
    "Parse JSON using serde",
    "rust"
);
```

**Code Quality:**
```cpp
// Generated code includes:
code.code            // The actual code
code.explanation     // What it does and why
code.language        // Programming language
code.dependencies    // Required libraries
code.confidence      // Quality score (0.0-1.0)
code.warnings        // Potential issues
```

### 4. Code Fixing

**Automatic error resolution:**

```cpp
// Your broken code
std::string broken = R"(
def calculate_average(numbers):
    total = 0
    for num in numbers
        total += num
    return total / len(numbers)
)";

// Error message
std::string error = "SyntaxError: invalid syntax at line 3";

// Fix it
auto fixed = assistant.fix_code(broken, error, "python");

std::cout << fixed.code << "\n";
// → Adds missing colon after 'for num in numbers'

std::cout << fixed.explanation << "\n";
// → "Added missing colon after for statement..."
```

### 5. Code Explanation

**Understand any code:**

```cpp
std::string complex_code = R"(
class Singleton {
    static Singleton* instance;
    Singleton() {}
public:
    static Singleton* getInstance() {
        if (!instance) instance = new Singleton();
        return instance;
    }
};
)";

auto explanation = assistant.explain_code(complex_code);
// → "This implements the Singleton design pattern. It ensures only
//    one instance of the class exists. The constructor is private to
//    prevent direct instantiation. getInstance() lazily creates the
//    instance on first call. Warning: Not thread-safe - consider
//    using std::call_once or a static local variable instead."
```

### 6. Semantic Search (RAG)

**Find code by meaning, not keywords:**

```cpp
// Traditional search (keyword matching)
grep "database" *.cpp    // Finds literal "database"

// Semantic search (meaning-based)
auto results = assistant.search_project(
    "code that connects to SQL server"
);
// → Finds database connection code even if it doesn't say "SQL server"
//   Understands: "MySQLConnector", "db.connect()", "connection pool", etc.
```

**How it works:**
1. Converts code to vector embeddings (numerical representation)
2. Stores in vector database
3. Searches by similarity (not just keywords)
4. Returns most relevant code

**Example use cases:**
```cpp
// Find error handling
auto error_code = assistant.search_project(
    "how is error handling done?"
);

// Find similar patterns
auto auth_code = assistant.search_project(
    "user authentication implementation"
);

// Find specific functionality
auto crypto_code = assistant.search_project(
    "encryption and decryption"
);
```

### 7. Project Analysis

**Understand your entire codebase:**

```cpp
auto analysis = assistant.analyze_codebase("/home/user/my_project");

std::cout << "Total Files: " << analysis.total_files << "\n";
std::cout << "Total Lines: " << analysis.total_lines << "\n";
std::cout << "Languages:\n";
for (auto& [lang, count] : analysis.languages) {
    std::cout << "  " << lang << ": " << count << " files\n";
}
std::cout << "Dependencies:\n";
for (auto& dep : analysis.dependencies) {
    std::cout << "  - " << dep << "\n";
}
std::cout << "\nPotential Issues:\n";
for (auto& issue : analysis.potential_issues) {
    std::cout << "  ⚠️  " << issue << "\n";
}
std::cout << "\nArchitecture:\n" << analysis.architecture_summary;
```

**Output Example:**
```
Total Files: 127
Total Lines: 15,432
Languages:
  C++: 89 files
  Python: 23 files
  JavaScript: 12 files
  CMake: 3 files
Dependencies:
  - SQLite3
  - OpenGL
  - ImGui
  - nlohmann/json

Potential Issues:
  ⚠️  Memory leak in src/core/model.cpp:234
  ⚠️  Unsafe pointer dereference in src/worker/trainer.cpp:89
  ⚠️  TODO comment in src/p2p/dht.cpp:156

Architecture:
This is a peer-to-peer distributed system built with C++23.
It uses a client-server pattern locally (worker/coordinator)
but operates in a mesh network globally. Core components:
- P2P networking layer (libp2p)
- Blockchain for consensus
- Neural network training engine
...
```

### 8. Code Refactoring

```cpp
std::string messy_code = R"(
void process(int* data, int size) {
    for(int i=0;i<size;++i){
        if(data[i]>0){
            data[i]=data[i]*2;
        }else{
            data[i]=0;
        }
    }
}
)";

auto refactored = assistant.refactor_code(
    messy_code,
    "Make it modern C++, use STL, improve readability"
);

std::cout << refactored.code << "\n";
```

**Output:**
```cpp
void process(std::span<int> data) {
    std::ranges::transform(data, data.begin(),
        [](int value) {
            return value > 0 ? value * 2 : 0;
        }
    );
}
```

### 9. Security Auditing

```cpp
auto vulnerabilities = assistant.security_audit("/home/user/my_project");

for (auto& vuln : vulnerabilities) {
    std::cout << "🔒 " << vuln << "\n";
}
```

**Detects:**
- SQL injection vulnerabilities
- Buffer overflows
- Use-after-free
- Hardcoded credentials
- Insecure randomness
- Path traversal vulnerabilities
- XSS vulnerabilities
- CSRF issues
- And more...

---

## 💡 Advanced Usage

### RAG (Retrieval-Augmented Generation)

**Context-aware responses using your files:**

```cpp
// Without RAG (generic answer)
"How do I connect to database?"
→ "You can use libraries like SQLite, PostgreSQL..."

// With RAG (project-specific answer)
"How do I connect to database?"
→ "In your project, you use the DatabaseManager class (src/core/database.cpp).
   Here's how to connect:

   DatabaseManager db("my.db");
   if (!db.initialize()) {
       // handle error
   }

   This is based on how it's used in src/coordinator/main.cpp:45"
```

**How to use:**
```cpp
// RAG is automatic, but you can control it
CodingAssistant assistant(perms);

// Index project for RAG
assistant.index_project("/path/to/project");

// Now all responses use your code as context
auto response = assistant.chat(conv_id, "How is error handling done?");
// → References YOUR error handling code, not generic examples
```

### Multi-file Context

```cpp
// Working with multiple files
std::string conv_id = assistant.start_conversation();

assistant.chat(conv_id, "Read src/database.cpp");
// → File is now in conversation context

assistant.chat(conv_id, "Read src/api.cpp");
// → Both files now in context

assistant.chat(conv_id, "How does the API layer use the database?");
// → Analyzes both files and explains the connection
```

### Code Translation

```cpp
// Python to C++
std::string python_code = R"(
def fibonacci(n):
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)
)";

auto cpp_code = code_gen.translate_code(
    python_code,
    "python",
    "cpp"
);

std::cout << cpp_code.code << "\n";
```

**Output:**
```cpp
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}
```

### Test Generation

```cpp
std::string code = R"(
int add(int a, int b) {
    return a + b;
}
)";

auto tests = code_gen.generate_tests(code, "cpp");
std::cout << tests.code << "\n";
```

**Output:**
```cpp
#include <gtest/gtest.h>

TEST(AddTest, PositiveNumbers) {
    EXPECT_EQ(add(2, 3), 5);
}

TEST(AddTest, NegativeNumbers) {
    EXPECT_EQ(add(-2, -3), -5);
}

TEST(AddTest, MixedSign) {
    EXPECT_EQ(add(-2, 3), 1);
    EXPECT_EQ(add(2, -3), -1);
}

TEST(AddTest, Zero) {
    EXPECT_EQ(add(0, 0), 0);
    EXPECT_EQ(add(5, 0), 5);
}

TEST(AddTest, LargeNumbers) {
    EXPECT_EQ(add(1000000, 2000000), 3000000);
}
```

---

## 🎮 Interactive GUI

```
┌──────────────────────────────────────────────────────────────┐
│ HydraAI Coding Assistant                           [_][□][X] │
├──────────────────────────────────────────────────────────────┤
│ ┌─── Chat ──────────────────────────┐ ┌─── Files ─────────┐ │
│ │ You: Read src/main.cpp            │ │ Active Files:     │ │
│ │                                    │ │ ✓ src/main.cpp   │ │
│ │ AI: I've analyzed main.cpp. It    │ │ ✓ src/database.h │ │
│ │ contains the entry point for the  │ │                   │ │
│ │ application. Key functions:       │ │ [Browse...]       │ │
│ │ - main() at line 15               │ │ [Search...]       │ │
│ │ - initialize() at line 45         │ │                   │ │
│ │                                    │ └───────────────────┘ │
│ │ You: Write a function to parse    │ ┌─── Memory ────────┐ │
│ │ JSON config files                 │ │ Conversations: 15 │ │
│ │                                    │ │                   │ │
│ │ AI: Here's a JSON parser:         │ │ Today:            │ │
│ │                                    │ │ • Config parsing  │ │
│ │ ```cpp                             │ │ • Bug fix in DB   │ │
│ │ #include <fstream>                │ │                   │ │
│ │ #include <nlohmann/json.hpp>      │ │ Yesterday:        │ │
│ │                                    │ │ • API endpoints   │ │
│ │ Config parse_config(              │ │ • Code review     │ │
│ │     const std::string& path) {    │ │                   │ │
│ │   std::ifstream file(path);       │ │ [View All...]     │ │
│ │   nlohmann::json j;               │ └───────────────────┘ │
│ │   file >> j;                      │                       │
│ │   ...                              │                       │
│ │ ```                                │                       │
│ │                                    │                       │
│ │ Save to file? [Yes] [No]          │                       │
│ │                                    │                       │
│ │ [Type your message...]            │                       │
│ └────────────────────────────────────┘                       │
│ ┌─── Code Preview ─────────────────────────────────────────┐ │
│ │ Language: C++                                            │ │
│ │ Confidence: 95%                                          │ │
│ │ Dependencies: nlohmann/json                              │ │
│ │                                                           │ │
│ │ #include <fstream>                                       │ │
│ │ #include <nlohmann/json.hpp>                            │ │
│ │                                                           │ │
│ │ struct Config {                                          │ │
│ │     std::string database_path;                           │ │
│ │     int port;                                            │ │
│ │     // ... more fields                                   │ │
│ │ };                                                       │ │
│ │                                                           │ │
│ │ [Copy] [Save As...] [Explain] [Test]                   │ │
│ └──────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────┘
```

---

## 🔐 Security & Privacy

### File Access Control

```cpp
FilePermissions perms;

// Whitelist approach (safe)
perms.allowed_directories = {
    "/home/user/my_project",
    "/home/user/another_project"
};

// Blacklist sensitive paths
perms.forbidden_paths = {
    "/home/user/.ssh",
    "/home/user/.gnupg",
    "/etc/passwd"
};

// Automatic forbidden patterns (built-in)
// - *.key, *.pem (private keys)
// - *.password, *.secret
// - .env, credentials.json
// - wallet.dat (crypto wallets)
```

### User Confirmation

```cpp
// All writes require confirmation
bool write_file(...) {
    std::cout << "Allow writing to " << path << "? [y/N]: ";
    std::string response;
    std::cin >> response;

    if (response != "y" && response != "Y") {
        return false;  // User denied
    }

    // Proceed with write
}
```

### Data Privacy

**What stays local:**
- All file contents (never sent to network)
- Conversation history (stored locally)
- Vector embeddings (computed locally)

**What's shared (P2P network):**
- Training gradients (mathematical updates, not raw data)
- Model weights (shared knowledge, not your code)
- Blockchain transactions (token movements)

**You control:**
- Which directories AI can access
- Whether AI can write files
- What conversations to keep/delete

---

## 📊 Supported Languages

### Full Support (Generation, Completion, Fixing, Explanation)
- ✅ C++ (all standards: C++11 to C++23)
- ✅ Python (2.7, 3.x)
- ✅ JavaScript / TypeScript
- ✅ Rust
- ✅ Go
- ✅ Java
- ✅ C#
- ✅ Swift
- ✅ Kotlin

### Partial Support (Basic understanding)
- ⚠️ Ruby
- ⚠️ PHP
- ⚠️ Scala
- ⚠️ Haskell
- ⚠️ Lua
- ⚠️ R

### Markup & Config
- ✅ HTML/CSS
- ✅ JSON/YAML/TOML
- ✅ Markdown
- ✅ SQL
- ✅ Bash/Shell scripts

---

## 🎯 Example Workflows

### Workflow 1: Debug Crash

```
You: My program crashes with segfault at line 42 in main.cpp

AI: [Reads main.cpp]
    Found the issue. Line 42 dereferences a nullptr:

    Node* current = head;  // Line 40
    // ... (missing null check)
    current->next = ...;   // Line 42 - CRASH if head is null

    Fix:
    if (current == nullptr) {
        return;  // or handle appropriately
    }
    current->next = ...;

You: Can you fix it and add proper error handling?

AI: [Generates fixed version with full error handling]
    Here's the corrected code:
    ...
    Should I write this to main.cpp?

You: Yes

AI: ✓ File updated. Also generated tests in main_test.cpp.
```

### Workflow 2: Add New Feature

```
You: I need to add user authentication to my Flask API

AI: [Searches project, finds it's a Flask app]
    I see you're using Flask. I'll create JWT-based authentication.
    Do you want to use an existing library or custom implementation?

You: Use PyJWT library

AI: [Generates complete auth system]
    Created:
    - auth.py: Authentication logic
    - middleware.py: JWT verification middleware
    - models.py: User model
    - tests/test_auth.py: Unit tests

    Updated:
    - requirements.txt: Added PyJWT
    - app.py: Integrated auth middleware

    Should I save these changes?

You: Yes

AI: ✓ All files saved. Run 'pip install -r requirements.txt' to install PyJWT.
```

### Workflow 3: Code Review

```
You: Review src/api/handlers.py for security issues

AI: [Analyzes file]
    Found 3 security vulnerabilities:

    🔒 SQL Injection (High Risk)
       Line 45: f"SELECT * FROM users WHERE id={user_id}"
       Fix: Use parameterized queries

    🔒 XSS Vulnerability (Medium Risk)
       Line 78: Returning user input unescaped
       Fix: Use html.escape() or template engine

    🔒 Missing Authentication (High Risk)
       Line 102: /admin endpoint has no auth check
       Fix: Add @require_auth decorator

    Should I fix these issues?

You: Yes, fix all

AI: [Generates secure version]
    ✓ Fixed all vulnerabilities
    ✓ Added input validation
    ✓ Added authentication middleware
    ✓ Generated security tests
```

---

## 🚀 Performance

### Indexing Speed
- Small project (100 files): ~5 seconds
- Medium project (1,000 files): ~30 seconds
- Large project (10,000 files): ~5 minutes

### Search Speed
- Semantic search: <100ms (after indexing)
- File content search: <50ms
- Code completion: <200ms

### Memory Usage
- Base: ~500MB
- Per indexed file: ~1KB
- Vector database: ~10MB per 1,000 files

---

## 🔧 Configuration

### config.json
```json
{
  "assistant": {
    "workspace_root": "/home/user/projects",
    "allowed_directories": [
      "/home/user/projects/project1",
      "/home/user/projects/project2"
    ],
    "permissions": {
      "allow_read": true,
      "allow_write": true,
      "allow_execute": false,
      "max_file_size_mb": 10
    },
    "preferences": {
      "preferred_language": "cpp",
      "code_style": "google",
      "auto_complete": true,
      "auto_fix": false,
      "suggestions": true
    },
    "memory": {
      "db_path": "./memory.db",
      "max_conversations": 1000,
      "auto_summarize": true
    },
    "vector_db": {
      "db_path": "./vectors.db",
      "embedding_model": "code-bert",
      "max_results": 10
    }
  }
}
```

---

## 📚 API Reference

See full API documentation in the header file:
- `include/hydra/coding_assistant.hpp`

Key classes:
- `CodingAssistant` - Main interface
- `FileSystemAccess` - Safe file operations
- `ConversationMemory` - Long-term memory
- `VectorDatabase` - Semantic search
- `CodeGenerator` - Code generation
- `RAGPipeline` - Context-aware responses

---

## 🎓 Best Practices

### 1. Start Conversations with Context
```cpp
// Bad
chat(id, "Fix this bug");

// Good
chat(id, "I'm working on a C++ REST API using Boost.Beast. "
         "There's a memory leak in the connection handler. "
         "File: src/api/connection.cpp");
```

### 2. Index Your Project First
```cpp
// Before asking questions
assistant.index_project("/path/to/project");

// Now it understands your codebase
assistant.chat(id, "How does authentication work?");
```

### 3. Be Specific
```cpp
// Vague
"Write a server"

// Specific
"Write a multithreaded TCP server in C++ using Boost.Asio
 that handles JSON requests and responds with JSON"
```

### 4. Review Generated Code
```cpp
auto code = assistant.generate_code(...);

// Check confidence
if (code.confidence < 0.8) {
    std::cout << "Low confidence. Review carefully.\n";
}

// Check warnings
for (auto& warning : code.warnings) {
    std::cout << "⚠️  " << warning << "\n";
}
```

---

## 🔮 Roadmap

### Current (v1.0)
- ✅ File system access with permissions
- ✅ Conversation memory
- ✅ Code generation (10+ languages)
- ✅ Semantic search (RAG)
- ✅ Security auditing

### Next (v1.1)
- 🔄 IDE integration (VSCode, CLion, Vim)
- 🔄 Real-time code completion (as you type)
- 🔄 Automatic refactoring suggestions
- 🔄 Code review comments

### Future (v2.0)
- 📋 Multi-file refactoring
- 📋 Architecture visualization
- 📋 Performance profiling
- 📋 Automated testing
- 📋 Documentation generation

---

## ❓ FAQ

**Q: Does it send my code to the cloud?**
A: No! All processing is local. Only training gradients (math, not code) are shared P2P.

**Q: Can it write production-ready code?**
A: It generates high-quality code, but always review it. Check the `confidence` score.

**Q: Will it remember my conversations?**
A: Yes, forever (or until you delete them). Perfect continuity across sessions.

**Q: Can it understand my entire codebase?**
A: Yes, after indexing. It builds a semantic understanding of your project.

**Q: Is my data safe?**
A: Yes. Sandboxed file access, user confirmations for writes, automatic blocking of sensitive files.

**Q: Can I limit what it can access?**
A: Absolutely. Set `allowed_directories` and it can't access anything else.

---

**Transform your coding experience. Let HydraAI be your intelligent pair programmer!** 🤖✨

```bash
./hydra_assistant --workspace ~/my_project --interactive
```
