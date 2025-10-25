"""
config.py - Configuration Settings for HydraAI
===============================================
This file contains all the settings you can customize.
Change these values to adjust how HydraAI works!
"""

# ==============================================================================
# COORDINATOR SETTINGS
# ==============================================================================

# Network settings
COORDINATOR_HOST = "0.0.0.0"  # 0.0.0.0 = listen on all network interfaces
COORDINATOR_PORT = 5000       # Port number for the server

# Model architecture settings
MODEL_CONFIG = {
    "vocab_size": 10000,      # Number of unique tokens (words) the model knows
    "embed_dim": 256,         # Size of word embeddings (higher = more expressive but slower)
    "num_heads": 4,           # Number of attention heads (higher = more complex)
    "num_layers": 2,          # Number of transformer layers (higher = deeper understanding)
    "max_seq_length": 512     # Maximum length of input sequences
}

# Training settings
TOKENS_PER_TASK = 1.0         # How many Hydra tokens workers earn per completed task
QUERY_COST = 0.5              # How many tokens it costs to query the model
TASK_GENERATION_INTERVAL = 60 # How often (in seconds) to generate new tasks
INITIAL_TASKS = 20            # How many tasks to create at startup

# Training data (feel free to add more examples!)
TRAINING_DATA = [
    "The quick brown fox jumps over the lazy dog",
    "Python is a great programming language for beginners",
    "Machine learning models learn patterns from data",
    "Neural networks are inspired by the human brain",
    "Distributed computing allows many computers to work together",
    "Blockchain technology enables decentralized systems",
    "Deep learning has revolutionized artificial intelligence",
    "Natural language processing helps computers understand text",
    "Computer vision allows machines to interpret images",
    "Reinforcement learning trains agents through rewards",
    "Data science combines statistics and programming",
    "Cloud computing provides scalable infrastructure",
    "Artificial intelligence is transforming many industries",
    "Open source software encourages collaboration",
    "Version control systems help manage code changes",
]

# ==============================================================================
# WORKER SETTINGS
# ==============================================================================

# Training configuration
WORKER_CONFIG = {
    "learning_rate": 0.001,   # How fast the model learns (lower = slower but more stable)
    "num_epochs": 3,          # How many times to train on each batch (higher = more learning)
    "batch_size": 1           # How many examples to process at once
}

# Work loop settings
SLEEP_BETWEEN_TASKS = 5       # Seconds to wait between tasks
DEFAULT_NUM_TASKS = None      # How many tasks to complete (None = unlimited)

# ==============================================================================
# DATABASE SETTINGS
# ==============================================================================

DATABASE_PATH = "hydra.db"    # Where to store the database file

# ==============================================================================
# DISPLAY SETTINGS
# ==============================================================================

# Color codes for terminal output (optional - for prettier output)
COLORS = {
    "GREEN": "\033[92m",
    "YELLOW": "\033[93m",
    "RED": "\033[91m",
    "BLUE": "\033[94m",
    "RESET": "\033[0m"
}

# Whether to use colors (set to False if your terminal doesn't support colors)
USE_COLORS = True

# ==============================================================================
# BEGINNER-FRIENDLY PRESETS
# ==============================================================================

# If you're on an older/slower computer, use these settings:
SLOW_COMPUTER_PRESET = {
    "vocab_size": 5000,       # Smaller vocabulary
    "embed_dim": 128,         # Smaller embeddings
    "num_heads": 2,           # Fewer attention heads
    "num_layers": 1,          # Single layer
    "max_seq_length": 256,    # Shorter sequences
    "num_epochs": 1           # Train less per batch
}

# If you want faster training (but less quality):
FAST_TRAINING_PRESET = {
    "vocab_size": 3000,
    "embed_dim": 64,
    "num_heads": 2,
    "num_layers": 1,
    "max_seq_length": 128,
    "num_epochs": 1
}

# ==============================================================================
# USAGE INSTRUCTIONS
# ==============================================================================

"""
How to use these presets:

1. In coordinator.py, replace MODEL_CONFIG with SLOW_COMPUTER_PRESET:

   from config import SLOW_COMPUTER_PRESET as MODEL_CONFIG

2. In worker.py, do the same:

   from config import SLOW_COMPUTER_PRESET as MODEL_CONFIG

3. Both coordinator and worker must use the SAME settings!
"""
