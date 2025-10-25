# HydraAI - Distributed Neural Network Training System

## 🎯 What is HydraAI?

HydraAI is a distributed neural network training system where users contribute their computer's resources to train AI models and earn Hydra tokens as rewards.

## 🏗️ System Architecture (Simplified)

```
┌─────────────────────────────────────────┐
│      COORDINATOR SERVER                 │
│  - Manages training tasks               │
│  - Distributes work to workers          │
│  - Tracks token rewards                 │
│  - Stores the global model              │
└─────────────────────────────────────────┘
            ↕️  ↕️  ↕️
┌──────────┐ ┌──────────┐ ┌──────────┐
│ Worker 1 │ │ Worker 2 │ │ Worker 3 │
│ (User PC)│ │ (User PC)│ │ (User PC)│
│          │ │          │ │          │
│ - Gets   │ │ - Gets   │ │ - Gets   │
│   tasks  │ │   tasks  │ │   tasks  │
│ - Trains │ │ - Trains │ │ - Trains │
│ - Earns  │ │ - Earns  │ │ - Earns  │
│   tokens │ │   tokens │ │   tokens │
└──────────┘ └──────────┘ └──────────┘
```

## 📦 Components

### 1. **Coordinator Server** (`coordinator.py`)
   - Central brain of the system
   - Manages the main neural network model
   - Sends training batches to workers
   - Receives trained updates from workers
   - Awards Hydra tokens for completed work

### 2. **Worker Client** (`worker.py`)
   - Runs on user's PC
   - Connects to coordinator
   - Downloads training tasks
   - Trains the neural network locally
   - Sends results back
   - Earns tokens

### 3. **Neural Network** (`model.py`)
   - A simple but universal transformer model
   - Can handle text, code, and other data
   - Small enough to train on personal computers

### 4. **Token System** (`token_system.py`)
   - Tracks user balances
   - Awards tokens for work
   - Allows spending tokens for queries

### 5. **Database** (`database.py`)
   - SQLite database (simple, no setup needed)
   - Stores user accounts, tokens, and tasks

## 🔄 How It Works (Step-by-Step)

1. **User starts worker**: User runs the worker client on their PC
2. **Worker connects**: Worker connects to the coordinator server
3. **Get task**: Coordinator sends a small training batch to the worker
4. **Train locally**: Worker trains the neural network on their PC
5. **Send results**: Worker sends the trained weights back
6. **Earn tokens**: Coordinator verifies the work and awards Hydra tokens
7. **Repeat**: Process repeats, gradually improving the global model

## 💰 Token Economy

- **Earn tokens**: By contributing computing power for training
- **Spend tokens**: To make queries to the trained neural network
- **Trade tokens**: (Future feature) Exchange with other users

## 🛠️ Technology Stack

- **Language**: Python 3.8+ (simple and beginner-friendly)
- **Neural Network**: PyTorch (popular and easy to use)
- **Networking**: Flask (simple web framework)
- **Database**: SQLite (no setup required)
- **Data Processing**: NumPy, basic Python libraries

## 🚀 Quick Start

1. Install dependencies: `pip install -r requirements.txt`
2. Start coordinator: `python coordinator.py`
3. Start worker: `python worker.py`
4. Earn tokens and contribute to AI!

## 📚 For Beginners

This project teaches you about:
- **Distributed Computing**: Multiple computers working together
- **Neural Networks**: How AI learns from data
- **Client-Server Architecture**: How networked applications work
- **Tokenomics**: Digital reward systems
- **Python Programming**: Real-world application development
