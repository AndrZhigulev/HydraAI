"""
test_installation.py - Verify HydraAI Installation
====================================================
Run this script to check if everything is installed correctly.

Usage:
    python test_installation.py
"""

import sys


def test_python_version():
    """Check if Python version is 3.8 or higher."""
    print("Testing Python version...")
    version = sys.version_info
    if version.major >= 3 and version.minor >= 8:
        print(f"  ✓ Python {version.major}.{version.minor}.{version.micro} (OK)")
        return True
    else:
        print(f"  ✗ Python {version.major}.{version.minor}.{version.micro} (Need 3.8+)")
        return False


def test_import(module_name, package_name=None):
    """Test if a module can be imported."""
    display_name = package_name or module_name
    try:
        module = __import__(module_name)
        version = getattr(module, '__version__', 'unknown')
        print(f"  ✓ {display_name} {version}")
        return True
    except ImportError:
        print(f"  ✗ {display_name} - NOT INSTALLED")
        return False


def test_files():
    """Check if all necessary files exist."""
    import os

    print("\nTesting project files...")
    required_files = [
        "database.py",
        "model.py",
        "coordinator.py",
        "worker.py",
        "query.py",
        "config.py",
        "requirements.txt",
        "README.md"
    ]

    all_exist = True
    for filename in required_files:
        if os.path.exists(filename):
            print(f"  ✓ {filename}")
        else:
            print(f"  ✗ {filename} - MISSING")
            all_exist = False

    return all_exist


def test_database():
    """Test database functionality."""
    print("\nTesting database...")
    try:
        from database import HydraDatabase
        db = HydraDatabase("test_install.db")

        # Test creating a user
        success = db.create_user("test_user")
        if success:
            print("  ✓ Database creation works")

            # Test retrieving user
            user = db.get_user("test_user")
            if user and user['user_id'] == "test_user":
                print("  ✓ Database queries work")

                # Test adding tokens
                db.add_tokens("test_user", 10.0, "reward", "Test")
                user = db.get_user("test_user")
                if user['total_tokens'] == 10.0:
                    print("  ✓ Token system works")
                else:
                    print("  ✗ Token system failed")
                    return False
            else:
                print("  ✗ Database queries failed")
                return False
        else:
            print("  ✗ Database creation failed")
            return False

        db.close()

        # Clean up test database
        import os
        if os.path.exists("test_install.db"):
            os.remove("test_install.db")

        return True

    except Exception as e:
        print(f"  ✗ Database test failed: {e}")
        return False


def test_model():
    """Test neural network model."""
    print("\nTesting neural network model...")
    try:
        import torch
        from model import SimpleTransformer, SimpleTokenizer

        # Create small model
        model = SimpleTransformer(
            vocab_size=100,
            embed_dim=32,
            num_heads=2,
            num_layers=1,
            max_seq_length=16
        )
        print("  ✓ Model creation works")

        # Test tokenizer
        tokenizer = SimpleTokenizer(vocab_size=100)
        encoded = tokenizer.encode("test text", max_length=16)
        if 'input_ids' in encoded and 'attention_mask' in encoded:
            print("  ✓ Tokenizer works")
        else:
            print("  ✗ Tokenizer failed")
            return False

        # Test forward pass
        input_ids = torch.tensor([encoded['input_ids']])
        attention_mask = torch.tensor([encoded['attention_mask']])

        with torch.no_grad():
            output = model(input_ids, attention_mask)

        if output.shape[0] == 1:  # Batch size should be 1
            print("  ✓ Model forward pass works")
        else:
            print("  ✗ Model forward pass failed")
            return False

        return True

    except Exception as e:
        print(f"  ✗ Model test failed: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """Run all tests."""
    print("=" * 60)
    print("HydraAI Installation Test")
    print("=" * 60)

    results = []

    # Test Python version
    results.append(("Python version", test_python_version()))

    # Test dependencies
    print("\nTesting dependencies...")
    results.append(("PyTorch", test_import("torch")))
    results.append(("Flask", test_import("flask")))
    results.append(("Requests", test_import("requests")))
    results.append(("NumPy", test_import("numpy")))

    # Test files
    results.append(("Project files", test_files()))

    # Test database
    results.append(("Database", test_database()))

    # Test model
    results.append(("Neural network", test_model()))

    # Summary
    print("\n" + "=" * 60)
    print("Test Summary")
    print("=" * 60)

    passed = sum(1 for _, result in results if result)
    total = len(results)

    for name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status:8} | {name}")

    print("=" * 60)
    print(f"Results: {passed}/{total} tests passed")
    print("=" * 60)

    if passed == total:
        print("\n🎉 All tests passed! You're ready to use HydraAI!")
        print("\nNext steps:")
        print("  1. Start the coordinator: python coordinator.py")
        print("  2. Start a worker:        python worker.py your_username")
        print("  3. Query the model:       python query.py your_username")
        return 0
    else:
        print("\n⚠️  Some tests failed. Please fix the issues above.")
        print("\nCommon fixes:")
        print("  - Install dependencies: pip install -r requirements.txt")
        print("  - Make sure all files are present")
        print("  - Check Python version (need 3.8+)")
        return 1


if __name__ == "__main__":
    exit_code = main()
    sys.exit(exit_code)
