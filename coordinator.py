"""
coordinator.py - Central Coordinator Server for HydraAI
========================================================
This is the "brain" of the distributed system. It:
1. Manages the global neural network model
2. Creates training tasks for workers
3. Distributes tasks to connected workers
4. Receives trained updates from workers
5. Awards Hydra tokens for completed work
6. Aggregates learning from all workers

Think of it like a teacher who:
- Gives homework (tasks) to students (workers)
- Collects completed homework
- Learns from everyone's work
- Gives rewards (tokens) for good work
"""

from flask import Flask, request, jsonify  # Flask creates web servers easily
import torch
import torch.nn as nn
import uuid  # For generating unique IDs
import random
import threading  # For running background tasks
import time
from datetime import datetime

from database import HydraDatabase
from model import SimpleTransformer, SimpleTokenizer


class CoordinatorServer:
    """
    The central coordinator that manages everything.
    """

    def __init__(self, host="0.0.0.0", port=5000):
        """
        Initialize the coordinator server.

        Args:
            host: IP address to listen on (0.0.0.0 = all network interfaces)
            port: Port number (5000 is common for development)
        """
        # Create Flask app
        self.app = Flask(__name__)
        # Flask is a web framework - it handles HTTP requests and responses

        self.host = host
        self.port = port

        # Initialize database
        self.db = HydraDatabase("hydra.db")
        print("✓ Database initialized")

        # Initialize the global model
        print("Creating global neural network model...")
        self.model = SimpleTransformer(
            vocab_size=10000,       # 10,000 different tokens
            embed_dim=256,          # 256-dimensional embeddings
            num_heads=4,            # 4 attention heads
            num_layers=2,           # 2 transformer layers
            max_seq_length=512      # Max 512 tokens per sequence
        )
        print(f"✓ Model created ({sum(p.numel() for p in self.model.parameters())} parameters)")

        # Initialize tokenizer
        self.tokenizer = SimpleTokenizer(vocab_size=10000)
        print("✓ Tokenizer initialized")

        # Training data (in a real system, this would be a large dataset)
        # For demonstration, we'll use simple example texts
        self.training_data = [
            "The quick brown fox jumps over the lazy dog",
            "Python is a great programming language for beginners",
            "Machine learning models learn from data",
            "Neural networks are inspired by the human brain",
            "Distributed computing allows many computers to work together",
            "Blockchain technology enables decentralized systems",
            "Deep learning has revolutionized artificial intelligence",
            "Natural language processing helps computers understand text",
            "Computer vision allows machines to interpret images",
            "Reinforcement learning trains agents through rewards"
        ]
        print(f"✓ Loaded {len(self.training_data)} training examples")

        # Tokens per task (how much workers earn for completing one task)
        self.tokens_per_task = 1.0

        # Lock for thread-safe model updates
        self.model_lock = threading.Lock()
        # This prevents multiple workers from updating the model at the same time

        # Setup Flask routes (API endpoints)
        self.setup_routes()

        # Generate some initial tasks
        self.generate_tasks(num_tasks=20)

    def setup_routes(self):
        """
        Setup API endpoints (URLs) that workers can call.

        Each route is like a function that gets called when someone visits a URL.
        """

        @self.app.route('/health', methods=['GET'])
        def health_check():
            """
            Health check endpoint - is the server running?

            URL: http://coordinator:5000/health
            Method: GET
            Returns: {"status": "healthy"}
            """
            return jsonify({"status": "healthy", "timestamp": datetime.now().isoformat()})

        @self.app.route('/register', methods=['POST'])
        def register_worker():
            """
            Register a new worker (user).

            URL: http://coordinator:5000/register
            Method: POST
            Body: {"user_id": "alice123"}
            Returns: Success or error message
            """
            # Get data from request
            data = request.json  # JSON data sent by worker

            user_id = data.get('user_id')
            if not user_id:
                return jsonify({"error": "user_id is required"}), 400
                # 400 = Bad Request (client made a mistake)

            # Try to create user
            success = self.db.create_user(user_id)

            if success:
                return jsonify({
                    "message": "Worker registered successfully",
                    "user_id": user_id,
                    "initial_tokens": 0
                })
            else:
                # User already exists - that's okay, just log them in
                user = self.db.get_user(user_id)
                return jsonify({
                    "message": "Worker already registered",
                    "user_id": user_id,
                    "current_tokens": user['total_tokens']
                })

        @self.app.route('/get_task', methods=['POST'])
        def get_task():
            """
            Get a training task to work on.

            URL: http://coordinator:5000/get_task
            Method: POST
            Body: {"user_id": "alice123"}
            Returns: Task data and model parameters
            """
            data = request.json
            user_id = data.get('user_id')

            if not user_id:
                return jsonify({"error": "user_id is required"}), 400

            # Make sure user exists
            user = self.db.get_user(user_id)
            if not user:
                return jsonify({"error": "User not registered"}), 404
                # 404 = Not Found

            # Get a pending task
            task = self.db.get_pending_task()

            if not task:
                return jsonify({"message": "No tasks available"}), 404

            # Assign task to this worker
            self.db.assign_task(task['task_id'], user_id)

            # Get current model parameters
            with self.model_lock:  # Thread-safe access
                model_params = self.model.get_trainable_parameters()

            return jsonify({
                "task_id": task['task_id'],
                "data_batch": task['data_batch'],  # Training data (JSON string)
                "model_parameters": model_params,   # Current model weights
                "tokens_reward": task['tokens_reward'],
                "instructions": "Train the model on this data batch and return the updated parameters"
            })

        @self.app.route('/submit_result', methods=['POST'])
        def submit_result():
            """
            Submit completed training results.

            URL: http://coordinator:5000/submit_result
            Method: POST
            Body: {
                "user_id": "alice123",
                "task_id": "task-123",
                "updated_parameters": {...}
            }
            Returns: Confirmation and token reward
            """
            data = request.json
            user_id = data.get('user_id')
            task_id = data.get('task_id')
            updated_parameters = data.get('updated_parameters')

            if not all([user_id, task_id, updated_parameters]):
                return jsonify({"error": "Missing required fields"}), 400

            # Verify user exists
            user = self.db.get_user(user_id)
            if not user:
                return jsonify({"error": "User not registered"}), 404

            # Mark task as completed
            self.db.complete_task(task_id, updated_parameters)

            # Update global model (aggregate worker's contribution)
            # Using a simple averaging strategy
            with self.model_lock:
                self.aggregate_parameters(updated_parameters, learning_rate=0.1)

            # Award tokens to the worker
            tokens_earned = self.tokens_per_task
            self.db.add_tokens(
                user_id,
                tokens_earned,
                'reward',
                f'Completed training task {task_id}'
            )

            # Update work counter
            print(f"✓ Worker {user_id} completed task {task_id}, earned {tokens_earned} tokens")

            return jsonify({
                "message": "Task completed successfully",
                "tokens_earned": tokens_earned,
                "new_balance": self.db.get_user(user_id)['total_tokens']
            })

        @self.app.route('/get_balance', methods=['POST'])
        def get_balance():
            """
            Check token balance.

            URL: http://coordinator:5000/get_balance
            Method: POST
            Body: {"user_id": "alice123"}
            Returns: Current token balance and stats
            """
            data = request.json
            user_id = data.get('user_id')

            if not user_id:
                return jsonify({"error": "user_id is required"}), 400

            stats = self.db.get_user_stats(user_id)
            if not stats:
                return jsonify({"error": "User not found"}), 404

            return jsonify(stats)

        @self.app.route('/query_model', methods=['POST'])
        def query_model():
            """
            Use tokens to query the trained model.

            URL: http://coordinator:5000/query_model
            Method: POST
            Body: {
                "user_id": "alice123",
                "prompt": "What is machine learning?"
            }
            Returns: Model's response (costs tokens)
            """
            data = request.json
            user_id = data.get('user_id')
            prompt = data.get('prompt')
            tokens_cost = 0.5  # Cost per query

            if not all([user_id, prompt]):
                return jsonify({"error": "user_id and prompt are required"}), 400

            # Check user balance
            user = self.db.get_user(user_id)
            if not user:
                return jsonify({"error": "User not registered"}), 404

            if user['total_tokens'] < tokens_cost:
                return jsonify({
                    "error": "Insufficient tokens",
                    "required": tokens_cost,
                    "balance": user['total_tokens']
                }), 402  # 402 = Payment Required

            # Deduct tokens
            self.db.add_tokens(user_id, -tokens_cost, 'query', 'Model query')

            # Generate response (simplified - in real system this would be more sophisticated)
            with self.model_lock:
                response = self.generate_response(prompt)

            return jsonify({
                "response": response,
                "tokens_spent": tokens_cost,
                "new_balance": self.db.get_user(user_id)['total_tokens']
            })

    def generate_response(self, prompt):
        """
        Generate a response using the trained model.

        Args:
            prompt: Input text

        Returns:
            Generated text response

        Note: This is simplified. A real system would use beam search,
        temperature sampling, and other techniques for better generation.
        """
        # Encode the prompt
        encoded = self.tokenizer.encode(prompt, max_length=128)
        input_ids = torch.tensor([encoded['input_ids']])
        attention_mask = torch.tensor([encoded['attention_mask']])

        # Generate (simplified - just return a placeholder for now)
        with torch.no_grad():
            output = self.model(input_ids, attention_mask)

        # For demonstration, return a simple message
        # In a real system, you'd decode the model's output
        return f"Model response to: '{prompt}' (Note: This is a demonstration. The model needs more training to generate meaningful responses.)"

    def aggregate_parameters(self, worker_parameters, learning_rate=0.1):
        """
        Aggregate worker's trained parameters into the global model.

        This is called "Federated Averaging" - we blend the worker's
        learning into the global model.

        Args:
            worker_parameters: The updated parameters from a worker
            learning_rate: How much to trust the worker's update (0.1 = 10%)

        How it works:
            new_global = 0.9 * old_global + 0.1 * worker_update
        """
        current_params = self.model.get_trainable_parameters()

        # Blend worker parameters into global model
        for param_name, worker_values in worker_parameters.items():
            if param_name in current_params:
                # Get current global values
                global_values = torch.tensor(current_params[param_name])
                worker_tensor = torch.tensor(worker_values)

                # Weighted average
                new_values = (1 - learning_rate) * global_values + learning_rate * worker_tensor

                # Update model
                current_params[param_name] = new_values.tolist()

        # Load updated parameters back into model
        self.model.set_parameters(current_params)
        print(f"  ↻ Model updated with worker's learning")

    def generate_tasks(self, num_tasks=10):
        """
        Generate training tasks from the training data.

        Args:
            num_tasks: How many tasks to create
        """
        print(f"\nGenerating {num_tasks} training tasks...")

        for i in range(num_tasks):
            # Randomly select training examples
            batch = random.sample(self.training_data, min(3, len(self.training_data)))

            task_id = f"task-{uuid.uuid4().hex[:8]}"
            self.db.create_task(
                task_id=task_id,
                data_batch=batch,  # List of text examples
                tokens_reward=self.tokens_per_task
            )

        print(f"✓ Generated {num_tasks} tasks")

    def run(self):
        """
        Start the coordinator server.
        """
        print(f"\n{'='*50}")
        print(f"🚀 HydraAI Coordinator Server Starting")
        print(f"{'='*50}")
        print(f"Server: http://{self.host}:{self.port}")
        print(f"Model: {sum(p.numel() for p in self.model.parameters())} parameters")
        print(f"Ready for workers to connect!")
        print(f"{'='*50}\n")

        # Start background task generator
        def background_task_generator():
            """Generate new tasks periodically."""
            while True:
                time.sleep(60)  # Every 60 seconds
                # Check if we need more tasks
                task = self.db.get_pending_task()
                if not task:
                    print("\n⚠️  Running low on tasks, generating more...")
                    self.generate_tasks(num_tasks=10)

        # Start background thread
        task_thread = threading.Thread(target=background_task_generator, daemon=True)
        task_thread.start()

        # Start Flask server
        self.app.run(host=self.host, port=self.port, threaded=True)
        # threaded=True allows multiple workers to connect simultaneously


# Main entry point
if __name__ == "__main__":
    # Create and run coordinator
    coordinator = CoordinatorServer(host="0.0.0.0", port=5000)
    coordinator.run()
