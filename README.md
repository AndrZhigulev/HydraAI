# 🧠 HydraAI - Distributed Neural Network Training

**Welcome to HydraAI!** This is a beginner-friendly distributed machine learning system where you can:
- 💻 **Contribute** your computer's resources to train AI
- 🪙 **Earn** Hydra tokens for your contributions
- 🤖 **Use** the trained AI with your tokens
- 📚 **Learn** about distributed computing, neural networks, and tokenomics

---

## 📖 Table of Contents

1. [What is HydraAI?](#what-is-hydraai)
2. [How It Works](#how-it-works)
3. [Installation](#installation)
4. [Quick Start](#quick-start)
5. [Usage Guide](#usage-guide)
6. [For Beginners](#for-beginners)
7. [Architecture](#architecture)
8. [Troubleshooting](#troubleshooting)

---

## 🎯 What is HydraAI?

HydraAI is a **distributed neural network training system**. Instead of one powerful computer training an AI model, many people contribute their personal computers to train together!

**The Token Economy:**
- ✅ **Earn tokens** by running the worker on your PC (contributing computing power)
- ✅ **Spend tokens** to query the trained AI model
- ✅ **Fair rewards** - the more you contribute, the more you earn

**Real-World Comparison:**
Think of it like [Folding@home](https://foldingathome.org/) (distributed protein folding) but for general AI training, with a built-in token reward system!

---

## 🔄 How It Works

```
┌─────────────────────────────────┐
│   YOU run worker.py on your PC │
│                                 │
│   ↓ Connects to coordinator    │
│   ↓ Gets training task          │
│   ↓ Trains neural network       │
│   ↓ Sends results back          │
│   ↓ EARNS TOKENS! 🎉            │
└─────────────────────────────────┘
```

**Step-by-step:**
1. You start the **worker** on your PC
2. Worker connects to the **coordinator server**
3. Coordinator sends a small training task
4. Your PC trains the neural network (uses CPU/GPU)
5. Worker sends trained results back
6. You earn **Hydra tokens**!
7. Repeat and accumulate tokens
8. Use tokens to query the AI model

---

## 💾 Installation

### Prerequisites

- **Python 3.8 or higher** ([Download Python](https://www.python.org/downloads/))
- **Basic terminal/command prompt knowledge**
- **Internet connection**

### Step 1: Clone or Download

Download this project to your computer:

```bash
# If you have git:
git clone <repository-url>
cd HydraAI

# Or download as ZIP and extract
```

### Step 2: Install Dependencies

Open terminal/command prompt in the HydraAI folder and run:

```bash
pip install -r requirements.txt
```

This installs:
- `torch` - Neural network library
- `flask` - Web server (for coordinator)
- `requests` - HTTP communication
- `numpy` - Numerical operations
- `transformers` - AI model architecture

**Note:** PyTorch installation might take a few minutes!

### Step 3: Verify Installation

Test that everything is installed:

```bash
python -c "import torch; print('PyTorch version:', torch.__version__)"
python -c "import flask; print('Flask version:', flask.__version__)"
```

If you see version numbers, you're good to go! ✅

---

## 🚀 Quick Start

### For Testing Locally (Everything on One Computer)

This is perfect for learning and testing!

#### Terminal 1: Start the Coordinator

```bash
python coordinator.py
```

You should see:
```
✓ Database initialized
✓ Model created (1234567 parameters)
✓ Tokenizer initialized
🚀 HydraAI Coordinator Server Starting
Server: http://0.0.0.0:5000
Ready for workers to connect!
```

**Keep this running!**

#### Terminal 2: Start a Worker

Open a **new terminal window** and run:

```bash
python worker.py alice123
```

Replace `alice123` with your desired username.

You should see:
```
📝 Registering with coordinator...
✓ Registered successfully!
📥 Requesting task from coordinator...
✓ Received task: task-abc123
🔧 Starting training...
```

**Keep this running to earn tokens!**

#### Terminal 3: Query the Model

After your worker has completed a few tasks (wait ~30 seconds), open a **third terminal** and run:

```bash
python query.py alice123 "What is machine learning?"
```

You should see your balance and the model's response!

---

## 📚 Usage Guide

### Running the Coordinator (Server)

**Basic:**
```bash
python coordinator.py
```

**Custom host/port:**
```python
# Edit coordinator.py at the bottom:
coordinator = CoordinatorServer(host="0.0.0.0", port=8080)
```

**What it does:**
- Manages the global AI model
- Creates and distributes training tasks
- Tracks user tokens
- Aggregates learning from all workers

### Running a Worker (Client)

**Basic with auto-generated ID:**
```bash
python worker.py
```

**With your own user ID:**
```bash
python worker.py alice123
```

**Connect to remote coordinator:**
```bash
python worker.py alice123 http://192.168.1.100:5000
```

**Customize training:**

Edit `worker.py` at the bottom:
```python
worker.work_loop(
    num_tasks=10,            # Complete 10 tasks then stop (None = run forever)
    sleep_between_tasks=3    # Wait 3 seconds between tasks
)
```

**What it does:**
- Connects to coordinator
- Downloads training tasks
- Trains the neural network locally
- Uploads results
- Earns you tokens!

### Querying the Model

**Single query:**
```bash
python query.py alice123 "What is Python?"
```

**Interactive mode:**
```bash
python query.py alice123
```

Then you can type prompts continuously:
```
Your prompt: What is machine learning?
💬 Response: ...
Your prompt: balance
💰 Your balance: 15.5 tokens
Your prompt: quit
```

**Commands in interactive mode:**
- Type any question to query the model (costs 0.5 tokens)
- `balance` or `b` - Check your token balance
- `quit` or `exit` or `q` - Exit

---

## 👶 For Beginners

### What's Going On?

Let me break down what each file does:

#### 1. `database.py` - The Memory
- Stores user accounts
- Tracks token balances
- Remembers completed tasks
- Uses SQLite (a simple file-based database)

**Key Concepts:**
- **Database**: Like a spreadsheet that programs can use
- **Tables**: Different sheets (users, tasks, transactions)
- **SQL**: Language to talk to databases

#### 2. `model.py` - The Brain
- Defines the neural network architecture
- A "transformer" model (like a tiny GPT)
- Can learn from text and code

**Key Concepts:**
- **Neural Network**: Math that learns patterns from data
- **Transformer**: Type of neural network good at sequences
- **Parameters**: The numbers the model learns (weights)
- **Training**: Adjusting parameters to improve predictions

#### 3. `coordinator.py` - The Manager
- Web server that manages everything
- Creates tasks for workers
- Collects results
- Awards tokens

**Key Concepts:**
- **Server**: A program that responds to network requests
- **API**: URLs that workers can call to get/send data
- **Flask**: Python library for making web servers

#### 4. `worker.py` - The Employee
- Runs on your PC
- Gets work from coordinator
- Trains the model
- Earns tokens

**Key Concepts:**
- **Client**: Program that connects to a server
- **HTTP Request**: Asking the server for something
- **Training Loop**: Repeatedly adjusting the model to reduce errors

#### 5. `query.py` - The User Interface
- Lets you use the trained AI
- Spends your earned tokens
- Interactive or single-query mode

### How Does Training Work?

Let's explain it simply:

1. **Model starts "dumb"** - Random weights, makes random predictions
2. **Worker gets training data** - Example: "Machine learning is awesome"
3. **Model tries to predict** - Given "Machine learning is", predict next word
4. **Calculate error** - How wrong was the prediction?
5. **Adjust weights** - Make tiny changes to improve
6. **Repeat thousands of times** - Model gets smarter!

**Real-World Analogy:**
- You're learning to throw darts
- First throws are random (dumb model)
- Each throw, you adjust your aim (training)
- After many throws, you hit bullseye more often (trained model)!

### How Do Tokens Work?

**Earning (Mining):**
```
Run worker → Complete task → Earn 1.0 tokens
```

**Spending:**
```
Query model → Use AI → Spend 0.5 tokens
```

**Why Tokens?**
- **Fair**: More work = more tokens = more usage
- **Decentralized**: Everyone contributes and benefits
- **Educational**: Learn about tokenomics

---

## 🏗️ Architecture

### System Components

```
┌─────────────────────────────────────────┐
│       COORDINATOR SERVER                │
│  ┌─────────────────────────────────┐   │
│  │  Flask Web Server               │   │
│  │  - /register (sign up)          │   │
│  │  - /get_task (get work)         │   │
│  │  - /submit_result (submit work) │   │
│  │  - /query_model (use AI)        │   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  Global Neural Network Model    │   │
│  │  - SimpleTransformer            │   │
│  │  - Aggregates all worker updates│   │
│  └─────────────────────────────────┘   │
│  ┌─────────────────────────────────┐   │
│  │  Database (SQLite)              │   │
│  │  - Users & tokens               │   │
│  │  - Tasks                        │   │
│  │  - Transactions                 │   │
│  └─────────────────────────────────┘   │
└─────────────────────────────────────────┘
                 ↕ ↕ ↕
        ┌────────┘ │ └────────┐
        │          │          │
┌───────▼───┐ ┌───▼──────┐ ┌─▼────────┐
│  Worker 1 │ │ Worker 2 │ │ Worker 3 │
│  (PC #1)  │ │ (PC #2)  │ │ (PC #3)  │
└───────────┘ └──────────┘ └──────────┘
```

### Data Flow

**Getting a Task:**
```
Worker → POST /get_task → Coordinator
Coordinator → Returns: task + model parameters
Worker → Trains locally
Worker → POST /submit_result → Coordinator
Coordinator → Awards tokens
```

**Querying the Model:**
```
User → POST /query_model → Coordinator
Coordinator → Checks balance (enough tokens?)
Coordinator → Runs inference
Coordinator → Returns response + deducts tokens
```

### Model Architecture

```
Input Text: "Hello world"
      ↓
[Tokenization] → [5, 42, 7, 0, 0, ...]
      ↓
[Embedding Layer] → [256-dim vectors]
      ↓
[Positional Encoding] → Add position info
      ↓
[Transformer Layer 1] → Self-attention + feedforward
      ↓
[Transformer Layer 2] → Self-attention + feedforward
      ↓
[Output Layer] → Predictions for each token
      ↓
Output: Probabilities for next word
```

**Parameters:**
- Vocabulary: 10,000 tokens
- Embedding dimension: 256
- Attention heads: 4
- Layers: 2
- Max sequence length: 512
- **Total parameters: ~1-2 million**

Small enough to train on a laptop! 💻

---

## ❓ Troubleshooting

### Problem: "Connection refused" when starting worker

**Solution:**
- Make sure the coordinator is running first
- Check the coordinator URL is correct
- If coordinator is on another computer, use its IP address

### Problem: "ModuleNotFoundError: No module named 'torch'"

**Solution:**
```bash
pip install -r requirements.txt
```

If that fails, install PyTorch separately:
```bash
pip install torch
```

### Problem: Worker keeps saying "No tasks available"

**Solution:**
- This is normal if many workers are running
- Wait a few seconds, new tasks are generated automatically
- The coordinator generates 10 new tasks every 60 seconds

### Problem: "Insufficient tokens" when querying

**Solution:**
- Run the worker to earn more tokens!
- Each completed task earns 1.0 tokens
- Each query costs 0.5 tokens
- Complete at least 1 task before querying

### Problem: Training is very slow

**Solution:**
- This is normal on older computers
- Reduce `num_epochs` in `worker.py` (line 37): change from `3` to `1`
- Reduce model size in `coordinator.py` and `worker.py`:
  ```python
  model = SimpleTransformer(
      vocab_size=5000,   # Reduce from 10000
      embed_dim=128,     # Reduce from 256
      num_layers=1       # Reduce from 2
  )
  ```

### Problem: "CUDA out of memory" error

**Solution:**
- The model is trying to use your GPU but running out of memory
- Force CPU usage: Edit `model.py`, add at the top:
  ```python
  import os
  os.environ['CUDA_VISIBLE_DEVICES'] = ''
  ```

---

## 📊 Testing the System

Want to see it in action? Here's a complete test:

### Terminal 1: Coordinator
```bash
python coordinator.py
```

### Terminal 2: Worker Alice
```bash
python worker.py alice
```

### Terminal 3: Worker Bob
```bash
python worker.py bob
```

### Terminal 4: Monitor
Wait 30 seconds, then check balances:

```bash
# Check Alice's balance
python query.py alice
> balance
> quit

# Check Bob's balance
python query.py bob
> balance
> quit
```

You should see both users earning tokens!

---

## 🎓 Learning Resources

Want to understand the code better?

**Python Basics:**
- [Python.org Official Tutorial](https://docs.python.org/3/tutorial/)
- [Real Python](https://realpython.com/)

**Neural Networks:**
- [3Blue1Brown - Neural Networks](https://www.youtube.com/watch?v=aircAruvnKk) (Video)
- [PyTorch Tutorials](https://pytorch.org/tutorials/)

**Distributed Systems:**
- [Federated Learning Explained](https://federated.withgoogle.com/)

**Flask (Web Servers):**
- [Flask Quickstart](https://flask.palletsprojects.com/quickstart/)

---

## 🚀 Next Steps

Want to improve HydraAI? Try these challenges:

### Beginner:
- ✅ Add more training data in `coordinator.py`
- ✅ Change the token rewards
- ✅ Customize the model size

### Intermediate:
- ✅ Add support for image training data
- ✅ Implement better text generation
- ✅ Add a web dashboard (HTML/CSS/JavaScript)
- ✅ Save the model periodically

### Advanced:
- ✅ Implement real blockchain-based tokens
- ✅ Add encryption for secure communication
- ✅ Implement Byzantine fault tolerance
- ✅ Scale to thousands of workers
- ✅ Add GPUs/CUDA optimization

---

## 📝 License

This is an educational project. Feel free to learn from it, modify it, and share it!

---

## 🙏 Credits

Created as an educational introduction to:
- Distributed machine learning (Federated Learning)
- Neural networks (Transformers)
- Client-server architecture
- Tokenomics

**Technologies used:**
- PyTorch (Neural networks)
- Flask (Web server)
- SQLite (Database)
- Python (Everything!)

---

## 💡 Tips for Success

1. **Start small**: Run coordinator and one worker locally first
2. **Monitor logs**: Watch what the programs print - it's educational!
3. **Experiment**: Change numbers, see what happens (you can't break anything permanently)
4. **Ask questions**: Read the comments in the code - every line is explained!
5. **Be patient**: Training takes time, especially on older computers

---

**Happy learning, and enjoy contributing to HydraAI!** 🧠⚡

Questions? Check the code comments - every line is documented for beginners!
