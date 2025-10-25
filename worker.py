"""
worker.py - Worker Client for HydraAI
=======================================
This runs on the user's personal computer. It:
1. Connects to the coordinator server
2. Gets training tasks
3. Trains the neural network locally using the PC's resources
4. Sends results back to the coordinator
5. Earns Hydra tokens for the work!

Think of this as an employee who:
- Gets assignments from the boss (coordinator)
- Does the work at their desk (local PC)
- Submits the work
- Gets paid (tokens)
"""

import requests  # For making HTTP requests to the coordinator
import torch
import torch.nn as nn
import torch.optim as optim
import time
import json
from datetime import datetime

from model import SimpleTransformer, SimpleTokenizer


class HydraWorker:
    """
    The worker client that runs on user's PC.
    """

    def __init__(self, user_id, coordinator_url="http://localhost:5000"):
        """
        Initialize the worker.

        Args:
            user_id: Unique identifier for this user (like a username)
            coordinator_url: URL of the coordinator server
        """
        self.user_id = user_id
        self.coordinator_url = coordinator_url
        # Remove trailing slash if present
        if self.coordinator_url.endswith('/'):
            self.coordinator_url = self.coordinator_url[:-1]

        # Initialize local model (will be synced with coordinator)
        self.model = None
        self.tokenizer = SimpleTokenizer(vocab_size=10000)

        # Training configuration
        self.learning_rate = 0.001  # How fast the model learns
        self.num_epochs = 3         # How many times to train on each batch

        print(f"Worker initialized for user: {self.user_id}")
        print(f"Coordinator: {self.coordinator_url}")

    def register(self):
        """
        Register with the coordinator server.

        Returns:
            True if successful, False otherwise
        """
        try:
            print(f"\n📝 Registering with coordinator...")
            response = requests.post(
                f"{self.coordinator_url}/register",
                json={"user_id": self.user_id},
                timeout=10  # 10 second timeout
            )
            # requests.post() sends an HTTP POST request
            # json=... automatically converts Python dict to JSON

            if response.status_code == 200:
                data = response.json()  # Parse JSON response
                print(f"✓ Registered successfully!")
                print(f"  Message: {data['message']}")
                if 'current_tokens' in data:
                    print(f"  Current balance: {data['current_tokens']} tokens")
                return True
            else:
                print(f"✗ Registration failed: {response.json()}")
                return False

        except requests.exceptions.RequestException as e:
            # This catches network errors, timeouts, etc.
            print(f"✗ Connection error: {e}")
            print(f"  Make sure the coordinator is running at {self.coordinator_url}")
            return False

    def get_task(self):
        """
        Get a training task from the coordinator.

        Returns:
            Dictionary with task details, or None if no tasks available
        """
        try:
            print(f"\n📥 Requesting task from coordinator...")
            response = requests.post(
                f"{self.coordinator_url}/get_task",
                json={"user_id": self.user_id},
                timeout=30
            )

            if response.status_code == 200:
                task_data = response.json()
                print(f"✓ Received task: {task_data['task_id']}")
                print(f"  Reward: {task_data['tokens_reward']} tokens")
                return task_data
            elif response.status_code == 404:
                print("⚠️  No tasks available at the moment")
                return None
            else:
                print(f"✗ Error getting task: {response.json()}")
                return None

        except requests.exceptions.RequestException as e:
            print(f"✗ Connection error: {e}")
            return None

    def train_on_task(self, task_data):
        """
        Train the model on a task's data.

        Args:
            task_data: The task information from coordinator

        Returns:
            Updated model parameters, or None if training failed

        How training works:
        1. Load model parameters from coordinator
        2. Load training data
        3. Train the model for several epochs
        4. Return the updated parameters
        """
        try:
            print(f"\n🔧 Starting training on task {task_data['task_id']}...")

            # Step 1: Load model parameters from coordinator
            model_params = task_data['model_parameters']
            print(f"  Loading model parameters...")

            # Create model if we don't have one yet
            if self.model is None:
                self.model = SimpleTransformer(
                    vocab_size=10000,
                    embed_dim=256,
                    num_heads=4,
                    num_layers=2,
                    max_seq_length=512
                )

            # Load parameters into model
            self.model.set_parameters(model_params)
            print(f"  ✓ Model loaded")

            # Step 2: Prepare training data
            data_batch = json.loads(task_data['data_batch'])
            print(f"  Training on {len(data_batch)} examples")

            # Step 3: Setup training
            optimizer = optim.Adam(self.model.parameters(), lr=self.learning_rate)
            # Adam is a popular optimization algorithm - it adjusts the model weights
            # lr = learning rate (how big the adjustment steps are)

            criterion = nn.CrossEntropyLoss()
            # CrossEntropyLoss measures how wrong the model's predictions are
            # The goal is to minimize this loss

            # Step 4: Train for multiple epochs
            self.model.train()  # Put model in training mode
            total_loss = 0

            for epoch in range(self.num_epochs):
                epoch_loss = 0

                # Train on each example in the batch
                for text_example in data_batch:
                    # Tokenize the text
                    encoded = self.tokenizer.encode(text_example, max_length=128)
                    input_ids = torch.tensor([encoded['input_ids']])
                    attention_mask = torch.tensor([encoded['attention_mask']])

                    # Create target (what the model should predict)
                    # For language modeling, we predict the next token
                    # So target is input shifted by one position
                    target = input_ids[:, 1:]  # Remove first token
                    input_for_model = input_ids[:, :-1]  # Remove last token
                    attention_for_model = attention_mask[:, :-1]

                    # Zero the gradients (clear previous calculations)
                    optimizer.zero_grad()

                    # Forward pass (make predictions)
                    output = self.model(input_for_model, attention_for_model)

                    # Reshape for loss calculation
                    output = output.reshape(-1, output.size(-1))
                    target = target.reshape(-1)

                    # Calculate loss (how wrong are we?)
                    loss = criterion(output, target)

                    # Backward pass (calculate gradients)
                    loss.backward()
                    # This calculates how much each parameter contributed to the error

                    # Update parameters (learn!)
                    optimizer.step()
                    # This adjusts the parameters to reduce the error

                    epoch_loss += loss.item()

                total_loss += epoch_loss
                avg_loss = epoch_loss / len(data_batch)
                print(f"  Epoch {epoch + 1}/{self.num_epochs}: loss = {avg_loss:.4f}")

            # Step 5: Extract updated parameters
            updated_parameters = self.model.get_trainable_parameters()
            print(f"  ✓ Training completed!")
            print(f"  Final average loss: {total_loss / (self.num_epochs * len(data_batch)):.4f}")

            return updated_parameters

        except Exception as e:
            print(f"✗ Training error: {e}")
            import traceback
            traceback.print_exc()
            return None

    def submit_result(self, task_id, updated_parameters):
        """
        Submit training results to the coordinator.

        Args:
            task_id: The task ID
            updated_parameters: The trained model parameters

        Returns:
            True if successful, False otherwise
        """
        try:
            print(f"\n📤 Submitting results for task {task_id}...")
            response = requests.post(
                f"{self.coordinator_url}/submit_result",
                json={
                    "user_id": self.user_id,
                    "task_id": task_id,
                    "updated_parameters": updated_parameters
                },
                timeout=60  # Longer timeout for uploading parameters
            )

            if response.status_code == 200:
                data = response.json()
                print(f"✓ Task submitted successfully!")
                print(f"  🎉 Earned: {data['tokens_earned']} tokens")
                print(f"  💰 New balance: {data['new_balance']} tokens")
                return True
            else:
                print(f"✗ Submission failed: {response.json()}")
                return False

        except requests.exceptions.RequestException as e:
            print(f"✗ Connection error: {e}")
            return False

    def check_balance(self):
        """
        Check current token balance.

        Returns:
            Dictionary with balance and stats, or None if failed
        """
        try:
            response = requests.post(
                f"{self.coordinator_url}/get_balance",
                json={"user_id": self.user_id},
                timeout=10
            )

            if response.status_code == 200:
                return response.json()
            else:
                print(f"✗ Error checking balance: {response.json()}")
                return None

        except requests.exceptions.RequestException as e:
            print(f"✗ Connection error: {e}")
            return None

    def work_loop(self, num_tasks=None, sleep_between_tasks=5):
        """
        Main work loop - continuously get and complete tasks.

        Args:
            num_tasks: How many tasks to complete (None = run forever)
            sleep_between_tasks: Seconds to wait between tasks

        This is the main function that keeps the worker running!
        """
        print(f"\n{'='*50}")
        print(f"🚀 Starting HydraAI Worker")
        print(f"{'='*50}")
        print(f"User ID: {self.user_id}")
        print(f"Coordinator: {self.coordinator_url}")
        print(f"Tasks to complete: {'Unlimited' if num_tasks is None else num_tasks}")
        print(f"{'='*50}\n")

        # Register with coordinator
        if not self.register():
            print("Failed to register. Exiting.")
            return

        tasks_completed = 0

        try:
            while True:
                # Check if we've completed enough tasks
                if num_tasks is not None and tasks_completed >= num_tasks:
                    print(f"\n✓ Completed {tasks_completed} tasks. Stopping.")
                    break

                # Get a task
                task_data = self.get_task()

                if task_data is None:
                    print(f"⏸️  Waiting {sleep_between_tasks} seconds before trying again...")
                    time.sleep(sleep_between_tasks)
                    continue

                # Train on the task
                updated_parameters = self.train_on_task(task_data)

                if updated_parameters is None:
                    print(f"⚠️  Training failed, skipping this task")
                    time.sleep(sleep_between_tasks)
                    continue

                # Submit results
                success = self.submit_result(task_data['task_id'], updated_parameters)

                if success:
                    tasks_completed += 1
                    print(f"\n📊 Progress: {tasks_completed} tasks completed")

                # Check balance periodically
                if tasks_completed % 5 == 0:
                    stats = self.check_balance()
                    if stats:
                        print(f"\n💰 Current Stats:")
                        print(f"   Balance: {stats['total_tokens']} tokens")
                        print(f"   Tasks completed: {stats['tasks_completed']}")
                        print(f"   Member since: {stats['member_since']}")

                # Wait before next task
                print(f"\n⏳ Waiting {sleep_between_tasks} seconds before next task...")
                time.sleep(sleep_between_tasks)

        except KeyboardInterrupt:
            # User pressed Ctrl+C
            print(f"\n\n⚠️  Worker stopped by user")
            print(f"Tasks completed this session: {tasks_completed}")

        # Final stats
        print(f"\n{'='*50}")
        print(f"📊 Session Summary")
        print(f"{'='*50}")
        stats = self.check_balance()
        if stats:
            print(f"Total balance: {stats['total_tokens']} tokens")
            print(f"Total tasks completed: {stats['tasks_completed']}")
        print(f"Tasks completed this session: {tasks_completed}")
        print(f"{'='*50}\n")


# Main entry point
if __name__ == "__main__":
    import sys

    # Get user ID from command line or use default
    if len(sys.argv) > 1:
        user_id = sys.argv[1]
    else:
        # Generate a random user ID for testing
        import uuid
        user_id = f"worker-{uuid.uuid4().hex[:8]}"
        print(f"No user ID provided, using: {user_id}")

    # Get coordinator URL from command line or use default
    coordinator_url = "http://localhost:5000"
    if len(sys.argv) > 2:
        coordinator_url = sys.argv[2]

    # Create worker
    worker = HydraWorker(user_id=user_id, coordinator_url=coordinator_url)

    # Start working!
    # You can customize these parameters:
    worker.work_loop(
        num_tasks=None,           # None = run forever, or set a number like 10
        sleep_between_tasks=5     # Wait 5 seconds between tasks
    )
