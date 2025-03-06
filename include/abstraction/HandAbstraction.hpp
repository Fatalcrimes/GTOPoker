#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <memory>
#include <mutex>

#include "game/PokerDefs.hpp"

namespace poker {

/**
 * HandAbstraction reduces the complexity of the game by grouping similar hands
 * into buckets, effectively reducing the state space of the game.
 */
class HandAbstraction {
public:
    // Abstraction levels
    enum class Level {
        NONE,       // No abstraction
        MINIMAL,    // Basic abstraction (few buckets)
        STANDARD,   // Balanced abstraction
        DETAILED    // Fine-grained abstraction
    };
    
    // Constructors
    HandAbstraction(Level level = Level::STANDARD);
    
    // Get bucket index for a hand in a specific round
    int getBucket(
        const std::array<Card, NUM_HOLE_CARDS>& holeCards,
        const std::vector<Card>& communityCards
    ) const;
    
    std::string getBucketHandRange(int bucket, BettingRound round) const;
    std::string convertToHandString(const std::array<Card, NUM_HOLE_CARDS>& holeCards) const;
    std::string compressHandRange(const std::vector<std::string>& hands) const;

    // Get number of buckets for a specific round
    int getNumBuckets(BettingRound round) const;
    
    // Precompute abstractions (can be time-consuming)
    void precompute();
    
    // Save/load precomputed abstractions
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    // Get abstraction level
    Level getLevel() const { return level_; }
    
    // Get abstraction name
    std::string getName() const;
    
    // Create abstraction based on level
    static std::shared_ptr<HandAbstraction> create(Level level);

private:
    struct BucketKey {
        std::array<Card, NUM_HOLE_CARDS> holeCards;
        std::vector<Card> communityCards;
        
        bool operator==(const BucketKey& other) const;
    };
    
    struct BucketKeyHash {
        std::size_t operator()(const BucketKey& key) const;
    };
    
    // Bucket configuration
    struct BucketConfig {
        int preflopBuckets;
        int flopBuckets;
        int turnBuckets;
        int riverBuckets;
    };
    
    // Data members
    Level level_;
    BucketConfig config_;
    
    // Make these mutable so const methods can modify them for caching
    mutable std::mutex mutex_;
    mutable std::unordered_map<BucketKey, int, BucketKeyHash> handToBucket_;
    
    // Precomputation helpers
    void computePreflopBuckets();
    
    // Helper methods for bucket calculation
    int calculatePreflopBucket(const std::array<Card, NUM_HOLE_CARDS>& holeCards) const;
    int calculatePostflopBucket(double equity, BettingRound round) const;
    double calculatePreflopHandStrength(const std::array<Card, NUM_HOLE_CARDS>& holeCards) const;
    
    // Equity calculation for postflop
    double calculateHandEquity(
        const std::array<Card, NUM_HOLE_CARDS>& holeCards,
        const std::vector<Card>& communityCards
    ) const;
};

} // namespace poker