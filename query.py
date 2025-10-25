"""
query.py - Query Interface for HydraAI
========================================
This allows users to spend their earned tokens to query the trained model.

Usage:
    python query.py <user_id> "What is machine learning?"
"""

import requests
import sys


class HydraQuery:
    """
    Simple interface to query the HydraAI model.
    """

    def __init__(self, user_id, coordinator_url="http://localhost:5000"):
        """
        Initialize the query interface.

        Args:
            user_id: Your user ID
            coordinator_url: URL of the coordinator server
        """
        self.user_id = user_id
        self.coordinator_url = coordinator_url.rstrip('/')  # Remove trailing slash

    def check_balance(self):
        """
        Check your current token balance.

        Returns:
            Dictionary with balance info, or None if error
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
                print(f"Error: {response.json()}")
                return None

        except requests.exceptions.RequestException as e:
            print(f"Connection error: {e}")
            return None

    def query_model(self, prompt):
        """
        Query the trained model with a prompt.

        Args:
            prompt: Your question or prompt for the model

        Returns:
            Model's response, or None if error

        Note:
            This costs tokens! Make sure you have enough balance.
        """
        try:
            print(f"\n🤔 Querying model...")
            print(f"Prompt: \"{prompt}\"")

            response = requests.post(
                f"{self.coordinator_url}/query_model",
                json={
                    "user_id": self.user_id,
                    "prompt": prompt
                },
                timeout=30
            )

            if response.status_code == 200:
                data = response.json()
                print(f"\n💬 Response:")
                print(f"   {data['response']}")
                print(f"\n💰 Cost: {data['tokens_spent']} tokens")
                print(f"   New balance: {data['new_balance']} tokens")
                return data['response']

            elif response.status_code == 402:
                # Payment required - insufficient tokens
                data = response.json()
                print(f"\n❌ Insufficient tokens!")
                print(f"   Required: {data['required']} tokens")
                print(f"   Your balance: {data['balance']} tokens")
                print(f"\n💡 Tip: Run the worker to earn more tokens!")
                return None

            else:
                print(f"Error: {response.json()}")
                return None

        except requests.exceptions.RequestException as e:
            print(f"Connection error: {e}")
            return None

    def interactive_mode(self):
        """
        Interactive mode - keep querying until user quits.
        """
        print(f"\n{'='*50}")
        print(f"🤖 HydraAI Interactive Query Mode")
        print(f"{'='*50}")
        print(f"User: {self.user_id}")

        # Check initial balance
        stats = self.check_balance()
        if stats:
            print(f"Balance: {stats['total_tokens']} tokens")
            print(f"Tasks completed: {stats['tasks_completed']}")
        else:
            print("Could not retrieve balance")

        print(f"\nType 'quit' or 'exit' to stop")
        print(f"Type 'balance' to check your token balance")
        print(f"{'='*50}\n")

        while True:
            try:
                # Get user input
                prompt = input("Your prompt: ").strip()

                if not prompt:
                    continue

                # Check for special commands
                if prompt.lower() in ['quit', 'exit', 'q']:
                    print("\n👋 Goodbye!")
                    break

                if prompt.lower() in ['balance', 'bal', 'b']:
                    stats = self.check_balance()
                    if stats:
                        print(f"\n💰 Your Stats:")
                        print(f"   Balance: {stats['total_tokens']} tokens")
                        print(f"   Tasks completed: {stats['tasks_completed']}")
                        print(f"   Member since: {stats['member_since']}\n")
                    continue

                # Query the model
                self.query_model(prompt)
                print()  # Empty line for readability

            except KeyboardInterrupt:
                print("\n\n👋 Goodbye!")
                break
            except Exception as e:
                print(f"\nError: {e}\n")


# Main entry point
if __name__ == "__main__":
    # Parse command line arguments
    if len(sys.argv) < 2:
        print("Usage:")
        print("  Interactive mode: python query.py <user_id>")
        print("  Single query:     python query.py <user_id> \"your prompt here\"")
        print("\nExample:")
        print("  python query.py alice123")
        print("  python query.py alice123 \"What is machine learning?\"")
        sys.exit(1)

    user_id = sys.argv[1]

    # Optional: custom coordinator URL
    coordinator_url = "http://localhost:5000"
    if len(sys.argv) > 3:
        coordinator_url = sys.argv[3]

    # Create query interface
    query = HydraQuery(user_id, coordinator_url)

    # If prompt provided, do single query
    if len(sys.argv) > 2:
        prompt = sys.argv[2]
        query.query_model(prompt)
    else:
        # Otherwise, enter interactive mode
        query.interactive_mode()
