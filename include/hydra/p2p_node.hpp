/**
 * @file p2p_node.hpp
 * @brief Decentralized P2P Node - Each PC is both worker and coordinator
 *
 * Every node in the HydraAI network runs this. There is no central server.
 * Nodes discover each other via DHT, agree on model updates via consensus,
 * and track tokens via blockchain.
 */

#pragma once

#include "blockchain.hpp"
#include "peer_manager.hpp"
#include "consensus.hpp"
#include "model.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

namespace hydra::p2p {

/**
 * @struct NodeConfig
 * @brief Configuration for a P2P node
 */
struct NodeConfig {
    std::string user_id;                    // User's unique identifier
    uint16_t listen_port{7777};             // Port to listen on
    std::vector<std::string> bootstrap_nodes; // Initial peers to connect to
    std::string data_dir{"./hydra_data"};  // Where to store blockchain, etc.

    // Network settings
    size_t max_peers{20};                   // Maximum number of peers
    size_t min_peers{8};                    // Minimum peers to maintain
    int peer_timeout_sec{60};               // Timeout for inactive peers

    // Blockchain settings
    int block_time_sec{10};                 // Target time between blocks
    double mining_difficulty{1.0};          // Proof-of-work difficulty

    // Training settings
    size_t batch_size{32};                  // Training batch size
    double learning_rate{0.001};            // Local training learning rate
    int training_epochs{3};                 // Epochs per training session
};

/**
 * @struct PeerInfo
 * @brief Information about a connected peer
 */
struct PeerInfo {
    std::string peer_id;                    // Peer's node ID
    std::string address;                    // IP:port
    int latency_ms{0};                      // Network latency
    std::string status;                     // "synced", "training", "validating"
    int reputation{0};                      // Reputation score (0-100)
    uint64_t last_seen{0};                  // Timestamp of last activity

    // Capabilities
    bool has_gpu{false};
    int cpu_cores{0};
    uint64_t memory_mb{0};
};

/**
 * @struct NetworkStats
 * @brief Statistics about the P2P network
 */
struct NetworkStats {
    size_t connected_peers{0};              // Current peer connections
    size_t estimated_network_size{0};       // Estimated total nodes
    uint64_t blockchain_height{0};          // Current block number
    std::string sync_status;                // "synced", "syncing", "disconnected"
    double network_hashrate{0.0};           // Total network training power
};

/**
 * @class P2PNode
 * @brief Main class for a decentralized HydraAI node
 *
 * This is the heart of HydraAI's decentralization. Every PC runs this node,
 * which handles:
 * - Peer discovery and connection management
 * - Blockchain consensus and validation
 * - Neural network training and verification
 * - Token wallet and transactions
 *
 * Thread Safety: This class uses internal locking and is thread-safe.
 *
 * Example usage:
 * @code
 * hydra::p2p::NodeConfig config;
 * config.user_id = "alice123";
 * config.bootstrap_nodes = {"node1.hydraai.org:7777", "node2.hydraai.org:7777"};
 *
 * hydra::p2p::P2PNode node(config);
 * node.start();
 *
 * // Node is now part of the P2P network!
 * // It will automatically:
 * // - Connect to peers
 * // - Sync blockchain
 * // - Participate in training
 * // - Earn tokens
 *
 * // Check balance
 * auto balance = node.get_token_balance();
 * std::cout << "Balance: " << balance << " HYDRA\n";
 *
 * // Query the model
 * auto response = node.query_model("What is machine learning?");
 * @endcode
 */
class P2PNode {
public:
    /**
     * @brief Constructor
     * @param config Node configuration
     */
    explicit P2PNode(const NodeConfig& config);

    /**
     * @brief Destructor - cleanly shuts down node
     */
    ~P2PNode();

    // Disable copying (node represents unique network identity)
    P2PNode(const P2PNode&) = delete;
    P2PNode& operator=(const P2PNode&) = delete;

    // =========================================================================
    // Node Lifecycle
    // =========================================================================

    /**
     * @brief Start the P2P node
     *
     * This will:
     * 1. Load or create wallet
     * 2. Load blockchain from disk
     * 3. Connect to bootstrap nodes
     * 4. Start peer discovery
     * 5. Begin training participation
     *
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop the node gracefully
     *
     * Disconnects from peers, saves state, stops training.
     */
    void stop();

    /**
     * @brief Check if node is running
     */
    bool is_running() const { return is_running_; }

    // =========================================================================
    // Network Operations
    // =========================================================================

    /**
     * @brief Get list of connected peers
     * @return Vector of peer information
     */
    std::vector<PeerInfo> get_peers() const;

    /**
     * @brief Get network statistics
     * @return Current network stats
     */
    NetworkStats get_network_stats() const;

    /**
     * @brief Manually connect to a peer
     * @param address Peer address (host:port)
     * @return true if connection initiated
     */
    bool connect_to_peer(const std::string& address);

    /**
     * @brief Disconnect from a peer
     * @param peer_id Peer to disconnect from
     */
    void disconnect_peer(const std::string& peer_id);

    // =========================================================================
    // Blockchain / Wallet Operations
    // =========================================================================

    /**
     * @brief Get current token balance
     * @return Balance in HYDRA tokens
     */
    double get_token_balance() const;

    /**
     * @brief Get wallet address
     * @return Public wallet address
     */
    std::string get_wallet_address() const;

    /**
     * @brief Get blockchain height
     * @return Current block number
     */
    uint64_t get_blockchain_height() const;

    /**
     * @brief Get recent transactions
     * @param limit Maximum number to return
     * @return Vector of recent transactions
     */
    std::vector<Transaction> get_recent_transactions(size_t limit = 10) const;

    /**
     * @brief Send tokens to another user
     * @param recipient Recipient's wallet address
     * @param amount Amount to send
     * @return Transaction ID if successful, nullopt otherwise
     */
    std::optional<std::string> send_tokens(const std::string& recipient,
                                          double amount);

    // =========================================================================
    // Training Operations
    // =========================================================================

    /**
     * @brief Start training (if not already training)
     *
     * The node will:
     * 1. Get training tasks from network
     * 2. Train model locally
     * 3. Propose updates to peers
     * 4. Earn tokens for valid work
     *
     * @return true if training started
     */
    bool start_training();

    /**
     * @brief Pause training
     */
    void pause_training();

    /**
     * @brief Resume training
     */
    void resume_training();

    /**
     * @brief Check if currently training
     */
    bool is_training() const { return is_training_; }

    /**
     * @brief Get training status
     * @return Status string and progress (0.0 to 1.0)
     */
    std::pair<std::string, double> get_training_status() const;

    /**
     * @brief Get training statistics
     */
    struct TrainingStats {
        int tasks_completed{0};
        int tasks_validated{0};
        double tokens_earned{0.0};
        double current_loss{0.0};
        std::vector<double> loss_history;
    };

    TrainingStats get_training_stats() const;

    // =========================================================================
    // Model Query Operations
    // =========================================================================

    /**
     * @brief Query the trained model
     * @param prompt Input prompt/question
     * @return Model's response, or nullopt if insufficient tokens
     *
     * This costs tokens (default 0.5 HYDRA per query)
     */
    std::optional<std::string> query_model(const std::string& prompt);

    /**
     * @brief Get current global model version
     * @return Model version string
     */
    std::string get_model_version() const;

    // =========================================================================
    // Consensus / Validation
    // =========================================================================

    /**
     * @brief Validate a peer's training proposal
     * @param proposal Proposed model update
     * @return true if valid, false if invalid
     *
     * This earns a small amount of tokens for validation work
     */
    bool validate_proposal(const ModelUpdateProposal& proposal);

    /**
     * @brief Vote on a proposal
     * @param proposal_id Proposal to vote on
     * @param accept true to accept, false to reject
     * @return true if vote submitted successfully
     */
    bool vote_on_proposal(const std::string& proposal_id, bool accept);

    // =========================================================================
    // Event Callbacks
    // =========================================================================

    /**
     * @brief Set callback for new peer connections
     * @param callback Function to call when peer connects
     */
    void on_peer_connected(std::function<void(const PeerInfo&)> callback);

    /**
     * @brief Set callback for peer disconnections
     */
    void on_peer_disconnected(std::function<void(const std::string& peer_id)> callback);

    /**
     * @brief Set callback for new blocks
     */
    void on_new_block(std::function<void(const Block&)> callback);

    /**
     * @brief Set callback for tokens earned
     */
    void on_tokens_earned(std::function<void(double amount)> callback);

    /**
     * @brief Set callback for model updates
     */
    void on_model_updated(std::function<void(const std::string& version)> callback);

    /**
     * @brief Set callback for log messages
     * @param callback Function(level, message) - level is "info", "warning", "error"
     */
    void on_log_message(std::function<void(const std::string& level,
                                           const std::string& message)> callback);

private:
    NodeConfig config_;
    bool is_running_{false};
    bool is_training_{false};

    // Core components (forward declared, defined in .cpp)
    class Impl;
    std::unique_ptr<Impl> impl_;

    // Internal methods
    void main_loop();                       // Main node event loop
    void peer_discovery_loop();             // DHT peer discovery
    void blockchain_sync_loop();            // Blockchain synchronization
    void training_loop();                   // Training and proposal submission
    void validation_loop();                 // Validate others' proposals
    void gossip_loop();                     // Gossip protocol for state sync
};

/**
 * @struct ModelUpdateProposal
 * @brief A proposed update to the global model
 */
struct ModelUpdateProposal {
    std::string proposal_id;                // Unique proposal ID
    std::string proposer;                   // Node that proposed it
    std::string model_version;              // Current model version
    std::string gradient_hash;              // Hash of gradient data
    std::vector<uint8_t> gradient_data;     // Serialized gradient

    // Proof of training
    double loss_before{0.0};
    double loss_after{0.0};
    int samples_trained{0};
    std::string dataset_hash;               // Which data was used

    // Voting
    int votes_accept{0};
    int votes_reject{0};
    uint64_t voting_deadline{0};            // Timestamp

    // Verification
    bool verified{false};
    std::vector<std::string> verifiers;     // Nodes that verified
};

/**
 * @class ProposalValidator
 * @brief Validates training proposals from other nodes
 */
class ProposalValidator {
public:
    /**
     * @brief Validate a training proposal
     * @param proposal The proposal to validate
     * @return true if valid, false if suspicious/invalid
     *
     * Validation checks:
     * 1. Download same training data
     * 2. Apply gradient
     * 3. Verify loss decreased
     * 4. Check gradient magnitudes are reasonable
     * 5. Verify computational cost matches claims
     */
    bool validate(const ModelUpdateProposal& proposal);

private:
    // Validation checks
    bool check_gradient_validity(const std::vector<uint8_t>& gradient);
    bool check_loss_improvement(double before, double after);
    bool check_computational_cost(int samples, uint64_t time_ms);
};

} // namespace hydra::p2p
