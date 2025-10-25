/**
 * @file blockchain.hpp
 * @brief Simple blockchain for HydraAI token tracking
 *
 * This is a lightweight blockchain optimized for tracking training contributions
 * and token rewards. Unlike Bitcoin (proof-of-work mining), this uses
 * "Proof-of-Training" where tokens are earned by training neural networks.
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <memory>

namespace hydra::p2p {

/**
 * @struct Transaction
 * @brief Represents a token transaction or training reward
 */
struct Transaction {
    enum class Type {
        REWARD,         // Earned tokens for training
        TRANSFER,       // Sent tokens to another user
        QUERY,          // Spent tokens on model query
        GENESIS         // Initial token distribution
    };

    std::string tx_id;                      // Transaction ID (hash)
    Type type;                              // Transaction type
    std::string from;                       // Sender wallet address (empty for rewards)
    std::string to;                         // Recipient wallet address
    double amount{0.0};                     // Amount in HYDRA tokens
    uint64_t timestamp{0};                  // Unix timestamp
    std::string signature;                  // Cryptographic signature
    std::string metadata;                   // Additional data (e.g., task ID)

    /**
     * @brief Calculate transaction ID (hash of transaction data)
     */
    std::string calculate_id() const;

    /**
     * @brief Verify transaction signature
     * @return true if signature is valid
     */
    bool verify_signature() const;

    /**
     * @brief Serialize transaction to bytes
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserialize transaction from bytes
     */
    static std::optional<Transaction> deserialize(const std::vector<uint8_t>& data);
};

/**
 * @struct Block
 * @brief A block in the blockchain
 */
struct Block {
    uint64_t height{0};                     // Block number (0 = genesis)
    std::string previous_hash;              // Hash of previous block
    std::string merkle_root;                // Merkle root of transactions
    uint64_t timestamp{0};                  // Block creation time
    std::vector<Transaction> transactions;  // Transactions in this block
    uint32_t nonce{0};                      // Proof-of-work nonce
    std::string hash;                       // Block hash

    // Block metadata
    std::string miner;                      // Node that created the block
    uint64_t difficulty{1};                 // Mining difficulty
    std::string model_version;              // Global model version at this block

    /**
     * @brief Calculate block hash
     * @return SHA256 hash of block data
     */
    std::string calculate_hash() const;

    /**
     * @brief Mine the block (find valid nonce)
     * @param difficulty Number of leading zeros required
     * @return true when valid hash found
     */
    bool mine(uint64_t difficulty);

    /**
     * @brief Verify block is valid
     * @return true if valid (hash, transactions, etc.)
     */
    bool verify() const;

    /**
     * @brief Calculate merkle root of transactions
     */
    std::string calculate_merkle_root() const;

    /**
     * @brief Serialize block to bytes
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserialize block from bytes
     */
    static std::optional<Block> deserialize(const std::vector<uint8_t>& data);
};

/**
 * @class Blockchain
 * @brief Main blockchain class
 *
 * Manages the chain of blocks, validates new blocks, tracks balances.
 * This is the "truth" of who has how many tokens.
 *
 * Thread Safety: Uses internal locking, thread-safe for concurrent access.
 */
class Blockchain {
public:
    /**
     * @brief Constructor
     * @param data_dir Directory to store blockchain data
     */
    explicit Blockchain(const std::string& data_dir = "./blockchain");

    /**
     * @brief Destructor
     */
    ~Blockchain();

    // Disable copying
    Blockchain(const Blockchain&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;

    // =========================================================================
    // Blockchain Operations
    // =========================================================================

    /**
     * @brief Initialize blockchain (create genesis block if needed)
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Get current blockchain height
     * @return Number of blocks in chain
     */
    uint64_t get_height() const;

    /**
     * @brief Get the latest block
     * @return Most recent block, or nullopt if chain is empty
     */
    std::optional<Block> get_latest_block() const;

    /**
     * @brief Get a specific block by height
     * @param height Block number (0 = genesis)
     * @return Block if found
     */
    std::optional<Block> get_block(uint64_t height) const;

    /**
     * @brief Get a specific block by hash
     * @param hash Block hash
     * @return Block if found
     */
    std::optional<Block> get_block_by_hash(const std::string& hash) const;

    /**
     * @brief Add a new block to the chain
     * @param block Block to add
     * @return true if block was valid and added
     *
     * This will:
     * 1. Verify block hash
     * 2. Verify all transactions
     * 3. Check prev_hash matches
     * 4. Update balances
     * 5. Save to disk
     */
    bool add_block(const Block& block);

    /**
     * @brief Verify entire blockchain integrity
     * @return true if chain is valid
     *
     * Checks:
     * - All blocks have valid hashes
     * - prev_hash links are correct
     * - All transactions are valid
     * - Balances add up correctly
     */
    bool verify_chain() const;

    // =========================================================================
    // Transaction Operations
    // =========================================================================

    /**
     * @brief Get pending transactions (not yet in a block)
     * @return Vector of pending transactions
     */
    std::vector<Transaction> get_pending_transactions() const;

    /**
     * @brief Add a transaction to the pending pool
     * @param tx Transaction to add
     * @return true if transaction is valid and added
     */
    bool add_transaction(const Transaction& tx);

    /**
     * @brief Get transaction by ID
     * @param tx_id Transaction ID
     * @return Transaction if found
     */
    std::optional<Transaction> get_transaction(const std::string& tx_id) const;

    /**
     * @brief Get all transactions for an address
     * @param address Wallet address
     * @param limit Maximum number to return (0 = all)
     * @return Vector of transactions
     */
    std::vector<Transaction> get_address_transactions(const std::string& address,
                                                     size_t limit = 0) const;

    // =========================================================================
    // Balance Operations
    // =========================================================================

    /**
     * @brief Get balance for a wallet address
     * @param address Wallet address
     * @return Current balance in HYDRA tokens
     */
    double get_balance(const std::string& address) const;

    /**
     * @brief Get balances for all addresses
     * @return Map of address → balance
     */
    std::map<std::string, double> get_all_balances() const;

    /**
     * @brief Get total supply of tokens
     * @return Total HYDRA tokens in existence
     */
    double get_total_supply() const;

    // =========================================================================
    // Mining Operations
    // =========================================================================

    /**
     * @brief Create a new block from pending transactions
     * @param miner_address Miner's wallet address
     * @param model_version Current global model version
     * @return Unmined block (caller must mine it)
     */
    Block create_block(const std::string& miner_address,
                      const std::string& model_version);

    /**
     * @brief Get current mining difficulty
     * @return Number of leading zeros required in hash
     */
    uint64_t get_difficulty() const;

    /**
     * @brief Adjust mining difficulty based on block time
     *
     * If blocks are being mined too fast, increase difficulty.
     * If too slow, decrease difficulty.
     * Target: 1 block every 10 seconds
     */
    void adjust_difficulty();

    // =========================================================================
    // Synchronization
    // =========================================================================

    /**
     * @brief Export blockchain to file
     * @param filepath Where to save
     * @return true if successful
     */
    bool export_chain(const std::string& filepath) const;

    /**
     * @brief Import blockchain from file
     * @param filepath File to load from
     * @return true if successful and valid
     */
    bool import_chain(const std::string& filepath);

    /**
     * @brief Get blocks in a range
     * @param start_height Starting block
     * @param end_height Ending block (inclusive)
     * @return Vector of blocks
     *
     * Used for syncing with peers: "Send me blocks 1000-1100"
     */
    std::vector<Block> get_blocks(uint64_t start_height,
                                  uint64_t end_height) const;

private:
    std::string data_dir_;
    std::vector<Block> chain_;
    std::vector<Transaction> pending_txs_;
    std::map<std::string, double> balances_;
    uint64_t current_difficulty_{1};

    // Internal methods
    void load_from_disk();
    void save_to_disk();
    void update_balances(const Block& block);
    bool verify_transaction(const Transaction& tx) const;
    std::string calculate_merkle_root(const std::vector<Transaction>& txs) const;
};

/**
 * @class Wallet
 * @brief Manages user's cryptographic keys and token balance
 */
class Wallet {
public:
    /**
     * @brief Create or load wallet
     * @param wallet_file Path to wallet file
     */
    explicit Wallet(const std::string& wallet_file = "./wallet.dat");

    /**
     * @brief Get public wallet address
     * @return Address string (hash of public key)
     */
    std::string get_address() const;

    /**
     * @brief Get public key
     */
    std::string get_public_key() const;

    /**
     * @brief Sign data with private key
     * @param data Data to sign
     * @return Signature
     */
    std::string sign(const std::string& data) const;

    /**
     * @brief Verify a signature
     * @param data Original data
     * @param signature Signature to verify
     * @param public_key Public key of signer
     * @return true if signature is valid
     */
    static bool verify(const std::string& data,
                      const std::string& signature,
                      const std::string& public_key);

    /**
     * @brief Create a signed transaction
     * @param to Recipient address
     * @param amount Amount to send
     * @param type Transaction type
     * @return Signed transaction ready to broadcast
     */
    Transaction create_transaction(const std::string& to,
                                  double amount,
                                  Transaction::Type type) const;

    /**
     * @brief Check if wallet is locked (encrypted)
     */
    bool is_locked() const;

    /**
     * @brief Unlock wallet with password
     */
    bool unlock(const std::string& password);

    /**
     * @brief Lock wallet
     */
    void lock();

    /**
     * @brief Change wallet password
     */
    bool change_password(const std::string& old_password,
                        const std::string& new_password);

private:
    std::string wallet_file_;
    std::string public_key_;
    std::string private_key_;
    std::string address_;
    bool is_locked_{true};

    void generate_keys();
    void load_from_file();
    void save_to_file();
};

/**
 * @brief Create the genesis block (first block in chain)
 * @param initial_distribution Map of address → initial token amount
 * @return Genesis block
 *
 * Example:
 * @code
 * std::map<std::string, double> distribution = {
 *     {"alice_address", 100.0},
 *     {"bob_address", 100.0},
 *     {"community_fund", 1000.0}
 * };
 * auto genesis = create_genesis_block(distribution);
 * blockchain.add_block(genesis);
 * @endcode
 */
Block create_genesis_block(const std::map<std::string, double>& initial_distribution);

/**
 * @brief Hash data using SHA256
 * @param data Data to hash
 * @return Hex-encoded hash
 */
std::string sha256(const std::string& data);

/**
 * @brief Hash data using SHA256 (binary data)
 */
std::string sha256(const std::vector<uint8_t>& data);

/**
 * @brief Generate a random node ID
 * @return Random hex string
 */
std::string generate_node_id();

} // namespace hydra::p2p
