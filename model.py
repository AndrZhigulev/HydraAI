"""
model.py - Simple Universal Neural Network for HydraAI
=======================================================
This is a simple transformer-based neural network that can learn from text and code.
It's small enough to train on personal computers but powerful enough to be useful!

What is a Transformer?
- A type of neural network that's good at understanding sequences (like text)
- Used in ChatGPT, BERT, and other modern AI systems
- Works by "paying attention" to different parts of the input
"""

import torch  # PyTorch - the neural network library
import torch.nn as nn  # Neural network building blocks
import torch.nn.functional as F  # Mathematical functions
import json
import os


class SimpleTransformer(nn.Module):
    """
    A simplified transformer model that can handle text and code.

    Think of this as the "brain" that learns from the training data.

    Args:
        vocab_size: How many different words/tokens the model knows (default: 10,000)
        embed_dim: Size of the internal representations (default: 256)
        num_heads: How many "attention heads" to use (default: 4)
        num_layers: How many transformer layers to stack (default: 2)
        max_seq_length: Maximum length of input sequences (default: 512)
    """

    def __init__(self, vocab_size=10000, embed_dim=256, num_heads=4, num_layers=2, max_seq_length=512):
        super(SimpleTransformer, self).__init__()
        # super() calls the parent class (nn.Module) initialization
        # This is required for PyTorch models

        # Save configuration
        self.vocab_size = vocab_size
        self.embed_dim = embed_dim
        self.num_heads = num_heads
        self.num_layers = num_layers
        self.max_seq_length = max_seq_length

        # ===== LAYER 1: Embedding Layer =====
        # This converts words/tokens (numbers) into vectors (lists of numbers)
        # Example: word "hello" (ID: 42) -> [0.5, -0.3, 0.8, ..., 0.1] (256 numbers)
        self.embedding = nn.Embedding(vocab_size, embed_dim)

        # ===== LAYER 2: Positional Encoding =====
        # This tells the model WHERE each word is in the sentence
        # Important because "dog bites man" is different from "man bites dog"!
        self.positional_encoding = nn.Parameter(
            torch.zeros(1, max_seq_length, embed_dim)
        )
        # nn.Parameter means this will be learned during training

        # ===== LAYER 3: Transformer Encoder Layers =====
        # These are the "thinking" layers that process the information
        # We stack multiple layers for deeper understanding
        encoder_layer = nn.TransformerEncoderLayer(
            d_model=embed_dim,           # Size of embeddings
            nhead=num_heads,             # Number of attention heads
            dim_feedforward=embed_dim * 4,  # Size of internal feedforward network
            dropout=0.1,                 # Dropout prevents overfitting (randomly turns off 10% of neurons)
            batch_first=True             # Our batches come first in dimensions
        )

        # Stack multiple encoder layers
        self.transformer = nn.TransformerEncoder(
            encoder_layer,
            num_layers=num_layers
        )

        # ===== LAYER 4: Output Layer =====
        # This converts the transformer output back to vocabulary predictions
        # Example: [0.5, -0.3, 0.8, ..., 0.1] -> probabilities for each word
        self.output_layer = nn.Linear(embed_dim, vocab_size)
        # Linear layer does: output = input × weights + bias

    def forward(self, input_ids, attention_mask=None):
        """
        Forward pass - this is where the actual computation happens.

        Args:
            input_ids: The input tokens (shape: [batch_size, sequence_length])
                      Example: [[1, 42, 567, 8], [2, 99, 3, 0]]
            attention_mask: Which tokens to pay attention to (1 = real token, 0 = padding)

        Returns:
            logits: Predictions for next token (shape: [batch_size, sequence_length, vocab_size])

        How it works:
            Input tokens -> Embeddings -> Add Position -> Transformer -> Output predictions
        """

        # Get the size of our input
        batch_size, seq_length = input_ids.size()

        # Step 1: Convert tokens to embeddings
        # Shape: [batch_size, seq_length] -> [batch_size, seq_length, embed_dim]
        x = self.embedding(input_ids)

        # Step 2: Add positional information
        # We slice [:, :seq_length, :] to match the current sequence length
        x = x + self.positional_encoding[:, :seq_length, :]

        # Step 3: Create attention mask if needed
        # Attention mask prevents the model from looking at padding tokens
        if attention_mask is not None:
            # Convert 0s and 1s to True/False (True = ignore this token)
            attention_mask = attention_mask == 0

        # Step 4: Pass through transformer layers
        # This is where the "thinking" happens - the model processes the sequence
        x = self.transformer(x, src_key_padding_mask=attention_mask)

        # Step 5: Convert to output predictions
        # Shape: [batch_size, seq_length, embed_dim] -> [batch_size, seq_length, vocab_size]
        logits = self.output_layer(x)

        return logits

    def get_trainable_parameters(self):
        """
        Get the parameters that need to be trained.

        Returns:
            Dictionary of parameter names and their values

        Why this is useful:
            - Workers need to send these back to the coordinator
            - This is what gets updated during training
        """
        return {name: param.data.cpu().numpy().tolist()
                for name, param in self.named_parameters()}

    def set_parameters(self, parameters):
        """
        Load parameters into the model.

        Args:
            parameters: Dictionary of parameter names and values

        Why this is useful:
            - Workers receive initial parameters from coordinator
            - Coordinator aggregates parameters from multiple workers
        """
        state_dict = {}
        for name, param_data in parameters.items():
            # Convert list back to PyTorch tensor
            state_dict[name] = torch.tensor(param_data)

        # Load into the model
        self.load_state_dict(state_dict, strict=False)

    def save_model(self, path):
        """
        Save the model to a file.

        Args:
            path: Where to save the model
        """
        # Create directory if it doesn't exist
        os.makedirs(os.path.dirname(path) if os.path.dirname(path) else '.', exist_ok=True)

        # Save both the model weights and configuration
        checkpoint = {
            'model_state_dict': self.state_dict(),
            'config': {
                'vocab_size': self.vocab_size,
                'embed_dim': self.embed_dim,
                'num_heads': self.num_heads,
                'num_layers': self.num_layers,
                'max_seq_length': self.max_seq_length
            }
        }
        torch.save(checkpoint, path)
        print(f"Model saved to {path}")

    def load_model(self, path):
        """
        Load a saved model from a file.

        Args:
            path: Where to load the model from
        """
        checkpoint = torch.load(path)
        self.load_state_dict(checkpoint['model_state_dict'])
        print(f"Model loaded from {path}")

    @staticmethod
    def from_pretrained(path):
        """
        Create a model and load weights from a saved file.

        Args:
            path: Path to the saved model

        Returns:
            Loaded model ready to use
        """
        checkpoint = torch.load(path)
        config = checkpoint['config']

        # Create model with saved configuration
        model = SimpleTransformer(
            vocab_size=config['vocab_size'],
            embed_dim=config['embed_dim'],
            num_heads=config['num_heads'],
            num_layers=config['num_layers'],
            max_seq_length=config['max_seq_length']
        )

        # Load weights
        model.load_state_dict(checkpoint['model_state_dict'])
        return model


class SimpleTokenizer:
    """
    A basic tokenizer that converts text to numbers and back.

    Tokenization is the process of splitting text into pieces (tokens)
    and assigning each piece a unique number (ID).

    Example:
        Text: "Hello world!"
        Tokens: ["Hello", "world", "!"]
        IDs: [42, 567, 8]
    """

    def __init__(self, vocab_size=10000):
        """
        Initialize tokenizer.

        Args:
            vocab_size: Maximum number of unique tokens to track
        """
        self.vocab_size = vocab_size
        self.token_to_id = {"<PAD>": 0, "<UNK>": 1, "<START>": 2, "<END>": 3}
        # Special tokens:
        # <PAD> = padding (to make all sequences same length)
        # <UNK> = unknown (for words we haven't seen before)
        # <START> = start of sequence
        # <END> = end of sequence

        self.id_to_token = {0: "<PAD>", 1: "<UNK>", 2: "<START>", 3: "<END>"}
        self.next_id = 4  # Next available ID

    def encode(self, text, max_length=512):
        """
        Convert text to a list of token IDs.

        Args:
            text: The text to encode
            max_length: Maximum sequence length (will pad or truncate)

        Returns:
            Dictionary with:
                - input_ids: List of token IDs
                - attention_mask: Which positions are real tokens (1) vs padding (0)
        """
        # Simple word-level tokenization (split by spaces and punctuation)
        # In a production system, you'd use a more sophisticated tokenizer
        tokens = text.lower().split()

        # Convert tokens to IDs
        input_ids = []
        for token in tokens[:max_length - 2]:  # Leave room for START and END
            if token not in self.token_to_id:
                # New token - add it to our vocabulary if we have room
                if self.next_id < self.vocab_size:
                    self.token_to_id[token] = self.next_id
                    self.id_to_token[self.next_id] = token
                    self.next_id += 1
                    input_ids.append(self.token_to_id[token])
                else:
                    # Vocabulary is full - use <UNK> token
                    input_ids.append(1)
            else:
                input_ids.append(self.token_to_id[token])

        # Add START and END tokens
        input_ids = [2] + input_ids + [3]

        # Create attention mask (1 for real tokens)
        attention_mask = [1] * len(input_ids)

        # Pad to max_length
        while len(input_ids) < max_length:
            input_ids.append(0)  # <PAD>
            attention_mask.append(0)  # Don't attend to padding

        return {
            'input_ids': input_ids[:max_length],
            'attention_mask': attention_mask[:max_length]
        }

    def decode(self, token_ids):
        """
        Convert token IDs back to text.

        Args:
            token_ids: List of token IDs

        Returns:
            The decoded text string
        """
        tokens = []
        for id in token_ids:
            if id in self.id_to_token:
                token = self.id_to_token[id]
                # Skip special tokens in output
                if token not in ["<PAD>", "<START>", "<END>"]:
                    tokens.append(token)

        return " ".join(tokens)


# Example usage and testing
if __name__ == "__main__":
    print("=== Testing HydraAI Neural Network ===\n")

    # Create a small model
    print("1. Creating model...")
    model = SimpleTransformer(
        vocab_size=1000,    # Small vocabulary for testing
        embed_dim=128,      # Small embedding size
        num_heads=2,        # Few attention heads
        num_layers=2,       # Two layers
        max_seq_length=128  # Short sequences
    )
    print(f"   Model created with {sum(p.numel() for p in model.parameters())} parameters")

    # Create tokenizer
    print("\n2. Creating tokenizer...")
    tokenizer = SimpleTokenizer(vocab_size=1000)

    # Test encoding
    print("\n3. Testing tokenization...")
    text = "hello world this is a test"
    encoded = tokenizer.encode(text, max_length=16)
    print(f"   Text: '{text}'")
    print(f"   Token IDs: {encoded['input_ids'][:10]}...")
    print(f"   Attention mask: {encoded['attention_mask'][:10]}...")

    # Test forward pass
    print("\n4. Testing forward pass...")
    input_ids = torch.tensor([encoded['input_ids']])  # Add batch dimension
    attention_mask = torch.tensor([encoded['attention_mask']])

    with torch.no_grad():  # Don't compute gradients (we're just testing)
        output = model(input_ids, attention_mask)
    print(f"   Input shape: {input_ids.shape}")
    print(f"   Output shape: {output.shape}")

    # Test saving and loading
    print("\n5. Testing save/load...")
    model.save_model("test_model.pt")
    loaded_model = SimpleTransformer.from_pretrained("test_model.pt")
    print("   Model saved and loaded successfully!")

    print("\n=== All tests passed! ===")
