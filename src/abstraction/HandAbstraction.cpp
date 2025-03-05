#include "abstraction/HandAbstraction.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <cmath>
#include <random>
#include <memory>

namespace poker {

// Helper for computing hash of BucketKey
bool HandAbstraction::BucketKey::operator==(const BucketKey& other) const {
    return holeCards == other.holeCards && communityCards == other.communityCards;
}

std::size_t HandAbstraction::BucketKeyHash::operator()(const BucketKey& key) const {
    std::size_t hash = 0;
    
    // Hash hole cards
    for (const auto& card : key.holeCards) {
        hash = hash * 31 + static_cast<int>(card.rank) * 4 + static_cast<int>(card.suit);
    }
    
    // Hash community cards
    for (const auto& card : key.communityCards) {
        hash = hash * 31 + static_cast<int>(card.rank) * 4 + static_cast<int>(card.suit);
    }
    
    return hash;
}

HandAbstraction::HandAbstraction(Level level) : level_(level) {
    // Set bucket configuration based on abstraction level
    switch (level) {
        case Level::NONE:
            config_ = {1, 1, 1, 1}; // No abstraction (every hand is unique)
            break;
        case Level::MINIMAL:
            config_ = {10, 50, 50, 50}; // Minimal bucketing
            break;
        case Level::STANDARD:
            config_ = {20, 100, 100, 100}; // Standard bucketing
            break;
        case Level::DETAILED:
            config_ = {50, 200, 200, 200}; // Detailed bucketing
            break;
    }
}

int HandAbstraction::getBucket(
    const std::array<Card, NUM_HOLE_CARDS>& holeCards,
    const std::vector<Card>& communityCards
) const {
    // Create the key for the hand
    BucketKey key{holeCards, communityCards};
    
    // Thread safety for concurrent access
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if this hand has already been bucketed
    auto it = handToBucket_.find(key);
    if (it != handToBucket_.end()) {
        return it->second;
    }
    
    // If not, we need to compute the bucket
    // This depends on the betting round
    BettingRound round;
    if (communityCards.empty()) {
        round = BettingRound::PREFLOP;
    } else if (communityCards.size() == 3) {
        round = BettingRound::FLOP;
    } else if (communityCards.size() == 4) {
        round = BettingRound::TURN;
    } else if (communityCards.size() == 5) {
        round = BettingRound::RIVER;
    } else {
        throw std::invalid_argument("Invalid number of community cards");
    }
    
    // Calculate the bucket
    int bucket = 0;
    
    switch (round) {
        case BettingRound::PREFLOP:
            // For preflop, use a simple ranking based on hand strength
            bucket = calculatePreflopBucket(holeCards);
            break;
        case BettingRound::FLOP:
        case BettingRound::TURN:
        case BettingRound::RIVER:
            // For postflop, use equity calculation
            double equity = calculateHandEquity(holeCards, communityCards);
            bucket = calculatePostflopBucket(equity, round);
            break;
    }
    
    // Cache the result
    handToBucket_[key] = bucket;
    
    return bucket;
}

int HandAbstraction::getNumBuckets(BettingRound round) const {
    switch (round) {
        case BettingRound::PREFLOP:
            return config_.preflopBuckets;
        case BettingRound::FLOP:
            return config_.flopBuckets;
        case BettingRound::TURN:
            return config_.turnBuckets;
        case BettingRound::RIVER:
            return config_.riverBuckets;
        default:
            return 1;
    }
}

void HandAbstraction::precompute() {
    computePreflopBuckets();
    // Note: Postflop buckets are typically computed on-demand
    // due to the large number of possible hands
}

bool HandAbstraction::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Save abstraction level
    file.write(reinterpret_cast<const char*>(&level_), sizeof(level_));
    
    // Save bucket configuration
    file.write(reinterpret_cast<const char*>(&config_), sizeof(config_));
    
    // Save number of hand-to-bucket mappings
    size_t numMappings = handToBucket_.size();
    file.write(reinterpret_cast<const char*>(&numMappings), sizeof(numMappings));
    
    // Save each mapping
    for (const auto& [key, bucket] : handToBucket_) {
        // Save hole cards
        for (const auto& card : key.holeCards) {
            int rankValue = static_cast<int>(card.rank);
            int suitValue = static_cast<int>(card.suit);
            file.write(reinterpret_cast<const char*>(&rankValue), sizeof(rankValue));
            file.write(reinterpret_cast<const char*>(&suitValue), sizeof(suitValue));
        }
        
        // Save number of community cards
        size_t numCommunityCards = key.communityCards.size();
        file.write(reinterpret_cast<const char*>(&numCommunityCards), sizeof(numCommunityCards));
        
        // Save community cards
        for (const auto& card : key.communityCards) {
            int rankValue = static_cast<int>(card.rank);
            int suitValue = static_cast<int>(card.suit);
            file.write(reinterpret_cast<const char*>(&rankValue), sizeof(rankValue));
            file.write(reinterpret_cast<const char*>(&suitValue), sizeof(suitValue));
        }
        
        // Save bucket
        file.write(reinterpret_cast<const char*>(&bucket), sizeof(bucket));
    }
    
    file.close();
    return true;
}

bool HandAbstraction::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    // Load abstraction level
    file.read(reinterpret_cast<char*>(&level_), sizeof(level_));
    
    // Load bucket configuration
    file.read(reinterpret_cast<char*>(&config_), sizeof(config_));
    
    // Clear existing mappings
    handToBucket_.clear();
    
    // Load number of hand-to-bucket mappings
    size_t numMappings;
    file.read(reinterpret_cast<char*>(&numMappings), sizeof(numMappings));
    
    // Load each mapping
    for (size_t i = 0; i < numMappings; ++i) {
        // Load hole cards
        std::array<Card, NUM_HOLE_CARDS> holeCards;
        for (size_t j = 0; j < NUM_HOLE_CARDS; ++j) {
            int rankValue, suitValue;
            file.read(reinterpret_cast<char*>(&rankValue), sizeof(rankValue));
            file.read(reinterpret_cast<char*>(&suitValue), sizeof(suitValue));
            holeCards[j] = Card(static_cast<Rank>(rankValue), static_cast<Suit>(suitValue));
        }
        
        // Load number of community cards
        size_t numCommunityCards;
        file.read(reinterpret_cast<char*>(&numCommunityCards), sizeof(numCommunityCards));
        
        // Load community cards
        std::vector<Card> communityCards(numCommunityCards);
        for (size_t j = 0; j < numCommunityCards; ++j) {
            int rankValue, suitValue;
            file.read(reinterpret_cast<char*>(&rankValue), sizeof(rankValue));
            file.read(reinterpret_cast<char*>(&suitValue), sizeof(suitValue));
            communityCards[j] = Card(static_cast<Rank>(rankValue), static_cast<Suit>(suitValue));
        }
        
        // Load bucket
        int bucket;
        file.read(reinterpret_cast<char*>(&bucket), sizeof(bucket));
        
        // Store the mapping
        BucketKey key{holeCards, communityCards};
        handToBucket_[key] = bucket;
    }
    
    file.close();
    return true;
}

std::string HandAbstraction::getName() const {
    switch (level_) {
        case Level::NONE:
            return "None";
        case Level::MINIMAL:
            return "Minimal";
        case Level::STANDARD:
            return "Standard";
        case Level::DETAILED:
            return "Detailed";
        default:
            return "Unknown";
    }
}

std::shared_ptr<HandAbstraction> HandAbstraction::create(Level level) {
    return std::make_shared<HandAbstraction>(level);
}

// Private implementation methods

void HandAbstraction::computePreflopBuckets() {
    // Create all possible hole card combinations
    std::vector<std::array<Card, NUM_HOLE_CARDS>> allHoleCards;
    
    // Generate all 52 choose 2 combinations
    for (int r1 = static_cast<int>(Rank::TWO); r1 <= static_cast<int>(Rank::ACE); ++r1) {
        for (int s1 = 0; s1 < NUM_SUITS; ++s1) {
            for (int r2 = r1; r2 <= static_cast<int>(Rank::ACE); ++r2) {
                int s2Start = (r1 == r2) ? s1 + 1 : 0;
                for (int s2 = s2Start; s2 < NUM_SUITS; ++s2) {
                    std::array<Card, NUM_HOLE_CARDS> holeCards = {
                        Card(static_cast<Rank>(r1), static_cast<Suit>(s1)),
                        Card(static_cast<Rank>(r2), static_cast<Suit>(s2))
                    };
                    allHoleCards.push_back(holeCards);
                }
            }
        }
    }
    
    // Compute strength for each hole card combination
    std::vector<std::pair<std::array<Card, NUM_HOLE_CARDS>, double>> strengthsAndCards;
    
    for (const auto& holeCards : allHoleCards) {
        double strength = calculatePreflopHandStrength(holeCards);
        strengthsAndCards.emplace_back(holeCards, strength);
    }
    
    // Sort by strength (descending)
    std::sort(strengthsAndCards.begin(), strengthsAndCards.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Assign buckets
    int numBuckets = config_.preflopBuckets;
    int cardsPerBucket = (strengthsAndCards.size() + numBuckets - 1) / numBuckets;
    
    for (size_t i = 0; i < strengthsAndCards.size(); ++i) {
        const auto& [holeCards, _] = strengthsAndCards[i];
        int bucket = static_cast<int>(i / cardsPerBucket);
        
        // Ensure we don't exceed the number of buckets
        bucket = std::min(bucket, numBuckets - 1);
        
        // Store the mapping
        BucketKey key{holeCards, {}};
        handToBucket_[key] = bucket;
    }
}

int HandAbstraction::calculatePreflopBucket(const std::array<Card, NUM_HOLE_CARDS>& holeCards) const {
    // Check if this hand has already been bucketed
    BucketKey key{holeCards, {}};
    auto it = handToBucket_.find(key);
    
    if (it != handToBucket_.end()) {
        return it->second;
    }
    
    // If not, calculate the bucket based on a simple strength metric
    double strength = calculatePreflopHandStrength(holeCards);
    
    // Map strength to bucket
    int bucket = static_cast<int>(strength * config_.preflopBuckets);
    bucket = std::min(bucket, config_.preflopBuckets - 1);
    
    return bucket;
}

double HandAbstraction::calculatePreflopHandStrength(const std::array<Card, NUM_HOLE_CARDS>& holeCards) const {
    // Extract card properties
    Rank rank1 = holeCards[0].rank;
    Rank rank2 = holeCards[1].rank;
    bool suited = holeCards[0].suit == holeCards[1].suit;
    
    // Convert ranks to integer values
    int r1 = static_cast<int>(rank1);
    int r2 = static_cast<int>(rank2);
    
    // Ensure r1 >= r2
    if (r1 < r2) {
        std::swap(r1, r2);
    }
    
    // Simple model for preflop hand strength:
    // - Pairs are strong, especially high pairs
    // - High cards are strong
    // - Suited cards get a bonus
    // - Connected cards (close in rank) get a bonus
    
    double strength = 0.0;
    
    // Base strength from ranks
    double rankStrength = (r1 + r2) / (2.0 * static_cast<int>(Rank::ACE));
    strength += 0.5 * rankStrength;
    
    // Pair bonus
    if (r1 == r2) {
        double pairStrength = r1 / static_cast<double>(Rank::ACE);
        strength += 0.3 * pairStrength;
    }
    
    // Suited bonus
    if (suited) {
        strength += 0.1;
    }
    
    // Connectedness bonus (max for consecutive ranks)
    int gap = r1 - r2;
    double connectednessBonus = std::max(0.0, 0.1 * (1.0 - gap / 12.0));
    strength += connectednessBonus;
    
    // Ensure strength is between 0 and 1
    strength = std::min(1.0, std::max(0.0, strength));
    
    return strength;
}

int HandAbstraction::calculatePostflopBucket(double equity, BettingRound round) const {
    // Map equity (0-1) to bucket
    int numBuckets;
    switch (round) {
        case BettingRound::FLOP:
            numBuckets = config_.flopBuckets;
            break;
        case BettingRound::TURN:
            numBuckets = config_.turnBuckets;
            break;
        case BettingRound::RIVER:
            numBuckets = config_.riverBuckets;
            break;
        default:
            numBuckets = 1;
    }
    
    // Non-linear bucketing - more buckets for higher equity hands
    double adjustedEquity = std::pow(equity, 0.7); // Gives more buckets to higher equity hands
    int bucket = static_cast<int>(adjustedEquity * numBuckets);
    bucket = std::min(bucket, numBuckets - 1);
    
    return bucket;
}

double HandAbstraction::calculateHandEquity(
    const std::array<Card, NUM_HOLE_CARDS>& holeCards,
    const std::vector<Card>& communityCards
) const {
    // This is a simplified equity calculation
    // In a real implementation, this would:
    // 1. Enumerate all possible remaining community cards
    // 2. For each, enumerate opponent hands and calculate win probability
    
    // For demonstration, we'll implement a Monte Carlo approximation
    // with a limited number of samples
    const int NUM_SAMPLES = 1000;
    
    // Create a list of the remaining deck
    std::vector<Card> remainingDeck;
    std::unordered_set<int> usedCardIndices;
    
    // Mark hole cards as used
    for (const auto& card : holeCards) {
        int cardIndex = static_cast<int>(card.rank) * NUM_SUITS + static_cast<int>(card.suit);
        usedCardIndices.insert(cardIndex);
    }
    
    // Mark community cards as used
    for (const auto& card : communityCards) {
        int cardIndex = static_cast<int>(card.rank) * NUM_SUITS + static_cast<int>(card.suit);
        usedCardIndices.insert(cardIndex);
    }
    
    // Fill remaining deck
    for (int r = static_cast<int>(Rank::TWO); r <= static_cast<int>(Rank::ACE); ++r) {
        for (int s = 0; s < NUM_SUITS; ++s) {
            int cardIndex = r * NUM_SUITS + s;
            if (usedCardIndices.find(cardIndex) == usedCardIndices.end()) {
                remainingDeck.push_back(Card(static_cast<Rank>(r), static_cast<Suit>(s)));
            }
        }
    }
    
    // Setup for Monte Carlo simulation
    std::random_device rd;
    std::mt19937 rng(rd());
    
    int totalOpponents = 2; // For 3-player game
    int wins = 0;
    int ties = 0;
    
    // Run the simulation
    for (int sample = 0; sample < NUM_SAMPLES; ++sample) {
        // Shuffle the remaining deck
        std::shuffle(remainingDeck.begin(), remainingDeck.end(), rng);
        
        // Deal cards to complete the board (if needed)
        std::vector<Card> sampleCommunityCards = communityCards;
        size_t cardsToAdd = 5 - sampleCommunityCards.size();
        sampleCommunityCards.insert(sampleCommunityCards.end(), 
                                   remainingDeck.begin(), 
                                   remainingDeck.begin() + cardsToAdd);
        
        // Deal hole cards to opponents
        std::vector<std::array<Card, NUM_HOLE_CARDS>> opponentHoleCards(totalOpponents);
        for (int i = 0; i < totalOpponents; ++i) {
            for (int j = 0; j < NUM_HOLE_CARDS; ++j) {
                opponentHoleCards[i][j] = remainingDeck[cardsToAdd + i * NUM_HOLE_CARDS + j];
            }
        }
        
        // Evaluate hands
        // Use a simple hand evaluator for this example
        // In a real implementation, you'd use the HandEvaluator class
        
        // For simplicity, we'll just compare the highest card in this example
        // This is NOT suitable for real play, just for demonstration
        Rank heroHighestRank = std::max(holeCards[0].rank, holeCards[1].rank);
        
        bool heroWins = true;
        int tieCount = 0;
        
        for (const auto& oppHoleCards : opponentHoleCards) {
            Rank oppHighestRank = std::max(oppHoleCards[0].rank, oppHoleCards[1].rank);
            
            if (oppHighestRank > heroHighestRank) {
                heroWins = false;
                break;
            } else if (oppHighestRank == heroHighestRank) {
                tieCount++;
            }
        }
        
        if (heroWins) {
            if (tieCount > 0) {
                ties++;
            } else {
                wins++;
            }
        }
    }
    
    // Calculate equity
    double equity = (wins + 0.5 * ties) / NUM_SAMPLES;
    
    return equity;
}

} // namespace poker