# ⚡ Quick Start Guide - 5 Minutes to HydraAI

This is the fastest way to get HydraAI running!

---

## Step 1: Install Dependencies (2 minutes)

```bash
pip install -r requirements.txt
```

**Wait for it to finish...**

---

## Step 2: Test Installation (30 seconds)

```bash
python test_installation.py
```

**You should see:** `🎉 All tests passed!`

If you see errors, run `pip install -r requirements.txt` again.

---

## Step 3: Start the Coordinator (10 seconds)

Open a terminal and run:

```bash
python coordinator.py
```

**You should see:**
```
🚀 HydraAI Coordinator Server Starting
Server: http://0.0.0.0:5000
Ready for workers to connect!
```

**✅ Keep this terminal open!**

---

## Step 4: Start a Worker (10 seconds)

Open a **NEW** terminal (keep the first one running!) and run:

```bash
python worker.py myusername
```

Replace `myusername` with any username you want.

**You should see:**
```
✓ Registered successfully!
📥 Requesting task from coordinator...
✓ Received task: task-abc123
🔧 Starting training...
```

**✅ Keep this terminal open too!**

---

## Step 5: Earn Tokens! (Wait 30 seconds)

Watch your worker terminal. You'll see:

```
Epoch 1/3: loss = 2.1234
Epoch 2/3: loss = 1.8765
Epoch 3/3: loss = 1.5432
✓ Training completed!
📤 Submitting results...
✓ Task submitted successfully!
🎉 Earned: 1.0 tokens
💰 New balance: 1.0 tokens
```

**Congratulations!** You just earned your first Hydra tokens!

---

## Step 6: Query the Model (Once you have 0.5+ tokens)

Open a **THIRD** terminal and run:

```bash
python query.py myusername "What is machine learning?"
```

**You should see:**
```
💬 Response: Model response to: 'What is machine learning?'...
💰 Cost: 0.5 tokens
   New balance: 0.5 tokens
```

---

## 🎉 You're Done!

You now have HydraAI running!

**What's happening:**
- ⚙️  **Terminal 1**: Coordinator managing everything
- 🔧 **Terminal 2**: Worker earning tokens by training
- 💬 **Terminal 3**: You querying the model

---

## What's Next?

### Keep Earning
Leave the worker running to accumulate tokens!

### Check Balance
```bash
python query.py myusername
> balance
```

### Interactive Queries
```bash
python query.py myusername
> What is Python?
> How does AI work?
> balance
> quit
```

### Add More Workers
Open another terminal:
```bash
python worker.py another_username
```

### Stop Everything
Press `Ctrl+C` in each terminal

---

## Troubleshooting

**"Connection refused"**
- Make sure coordinator is running first (Step 3)
- Check the coordinator terminal for errors

**"No module named 'torch'"**
- Run: `pip install -r requirements.txt`
- Wait for PyTorch to finish installing (it's big!)

**"No tasks available"**
- Wait a few seconds, tasks are generated automatically
- The coordinator creates new tasks every 60 seconds

**"Insufficient tokens"**
- Run the worker longer to earn more tokens
- Each task earns 1.0 tokens
- Each query costs 0.5 tokens

---

## Configuration

Want to customize HydraAI? Edit `config.py`!

**Slower computer?** Use this at the top of `coordinator.py`:
```python
from config import SLOW_COMPUTER_PRESET as MODEL_CONFIG
```

And the same in `worker.py`!

---

## Learn More

- 📚 **Full documentation**: See `README.md`
- 🎓 **Learn the code**: See `BEGINNERS_GUIDE.md`
- 🏗️  **Architecture**: See `ARCHITECTURE.md`

---

**Happy training!** 🧠⚡
