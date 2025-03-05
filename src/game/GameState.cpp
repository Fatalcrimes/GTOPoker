#include "game/GameState.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <chrono>
#include <stdexcept>

namespace poker {

// Forward declare the HandEvaluator
// Enhanced HandEvaluator implementation
// This should replace the simplified version in GameState.cpp

class HandEvaluator {
public:
    // Evaluate hand strength for a player (2 hole cards + community cards)
    HandStrength evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                             const std::vector<Card>& communityCards) const {
        // Find the best 5-card hand from all 7 cards
        std::vector<Card> allCards;
        allCards.reserve(holeCards.size() + communityCards.size());
        allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
        allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());

        // Store all possible 5-card combinations
        std::vector<std::vector<Card>> combinations;
        std::vector<Card> currentCombo(5);
        generateCombinations(allCards, 0, 0, 5, currentCombo, combinations);

        // Evaluate each combination and find the best one
        HandStrength bestStrength;
        bool firstHand = true;

        for (const auto& combo : combinations) {
            HandStrength currentStrength = evaluateFiveCardHand(combo);
            
            if (firstHand || currentStrength > bestStrength) {
                bestStrength = currentStrength;
                firstHand = false;
            }
        }

        return bestStrength;
    }
    
    // Find the best 5-card hand
    std::vector<Card> findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                  const std::vector<Card>& communityCards) const {
        std::vector<Card> allCards;
        allCards.reserve(holeCards.size() + communityCards.size());
        allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
        allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());

        // Store all possible 5-card combinations
        std::vector<std::vector<Card>> combinations;
        std::vector<Card> currentCombo(5);
        generateCombinations(allCards, 0, 0, 5, currentCombo, combinations);

        // Evaluate each combination and find the best one
        HandStrength bestStrength;
        std::vector<Card> bestHand;
        bool firstHand = true;

        for (const auto& combo : combinations) {
            HandStrength currentStrength = evaluateFiveCardHand(combo);
            
            if (firstHand || currentStrength > bestStrength) {
                bestStrength = currentStrength;
                bestHand = combo;
                firstHand = false;
            }
        }

        return bestHand;
    }

private:
    // Generate all k-combinations of n elements
    void generateCombinations(const std::vector<Card>& cards, int start, int index, int k, 
                              std::vector<Card>& current, std::vector<std::vector<Card>>& result) const {
        // If we have selected k elements, add to result
        if (index == k) {
            result.push_back(current);
            return;
        }

        // Try to select from remaining elements
        for (int i = start; i <= cards.size() - k + index; ++i) {
            current[index] = cards[i];
            generateCombinations(cards, i + 1, index + 1, k, current, result);
        }
    }

    // Evaluate a 5-card hand
    HandStrength evaluateFiveCardHand(const std::vector<Card>& cards) const {
        if (cards.size() != 5) {
            throw std::invalid_argument("Hand must contain exactly 5 cards");
        }

        // Check for flush
        bool isFlush = isHandFlush(cards);
        
        // Check for straight
        bool isStraight = isHandStraight(cards);
        
        // Count occurrences of each rank
        std::array<int, 15> rankCounts = {0};
        for (const auto& card : cards) {
            rankCounts[static_cast<int>(card.rank)]++;
        }
        
        // Find ranks by frequency (for pairs, etc.)
        std::vector<std::pair<int, int>> ranksByFreq; // (rank, count)
        for (int i = static_cast<int>(Rank::TWO); i <= static_cast<int>(Rank::ACE); ++i) {
            if (rankCounts[i] > 0) {
                ranksByFreq.emplace_back(i, rankCounts[i]);
            }
        }
        
        // Sort by frequency (high to low), then by rank (high to low)
        std::sort(ranksByFreq.begin(), ranksByFreq.end(), 
                 [](const auto& a, const auto& b) {
                     if (a.second != b.second) return a.second > b.second;
                     return a.first > b.first;
                 });
        
        // Extract kickers (ranks in descending order)
        std::array<uint32_t, 5> kickers = {0};
        for (size_t i = 0; i < ranksByFreq.size() && i < 5; ++i) {
            kickers[i] = ranksByFreq[i].first;
        }

        // Determine hand rank and values
        HandRank handRank;
        uint32_t primaryValue = 0;
        uint32_t secondaryValue = 0;
        
        // Royal flush
        if (isFlush && isStraight && hasRank(cards, Rank::ACE) && hasRank(cards, Rank::KING)) {
            int highestRank = getHighestRank(cards);
            if (highestRank == static_cast<int>(Rank::ACE)) {
                handRank = HandRank::ROYAL_FLUSH;
                primaryValue = static_cast<int>(Rank::ACE);
                return HandStrength{handRank, primaryValue, secondaryValue, kickers};
            }
        }
        
        // Straight flush
        if (isFlush && isStraight) {
            handRank = HandRank::STRAIGHT_FLUSH;
            primaryValue = getHighestRank(cards);
            // Handle A-5 straight flush
            if (hasRank(cards, Rank::ACE) && hasRank(cards, Rank::TWO) && 
                hasRank(cards, Rank::THREE) && hasRank(cards, Rank::FOUR) && 
                hasRank(cards, Rank::FIVE)) {
                primaryValue = static_cast<int>(Rank::FIVE); // A-5 straight is ranked by 5
            }
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Four of a kind
        if (ranksByFreq[0].second == 4) {
            handRank = HandRank::FOUR_OF_A_KIND;
            primaryValue = ranksByFreq[0].first;
            secondaryValue = ranksByFreq[1].first; // The kicker
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Full house
        if (ranksByFreq[0].second == 3 && ranksByFreq[1].second == 2) {
            handRank = HandRank::FULL_HOUSE;
            primaryValue = ranksByFreq[0].first;   // Triplet rank
            secondaryValue = ranksByFreq[1].first; // Pair rank
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Flush
        if (isFlush) {
            handRank = HandRank::FLUSH;
            // Use kickers for tie-breaking (already sorted by rank)
            return HandStrength{handRank, kickers[0], kickers[1], kickers};
        }
        
        // Straight
        if (isStraight) {
            handRank = HandRank::STRAIGHT;
            primaryValue = getHighestRank(cards);
            // Handle A-5 straight
            if (hasRank(cards, Rank::ACE) && hasRank(cards, Rank::TWO) && 
                hasRank(cards, Rank::THREE) && hasRank(cards, Rank::FOUR) && 
                hasRank(cards, Rank::FIVE)) {
                primaryValue = static_cast<int>(Rank::FIVE); // A-5 straight is ranked by 5
            }
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Three of a kind
        if (ranksByFreq[0].second == 3) {
            handRank = HandRank::THREE_OF_A_KIND;
            primaryValue = ranksByFreq[0].first;
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Two pair
        if (ranksByFreq[0].second == 2 && ranksByFreq[1].second == 2) {
            handRank = HandRank::TWO_PAIR;
            primaryValue = ranksByFreq[0].first;   // Higher pair
            secondaryValue = ranksByFreq[1].first; // Lower pair
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // One pair
        if (ranksByFreq[0].second == 2) {
            handRank = HandRank::PAIR;
            primaryValue = ranksByFreq[0].first;
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // High card
        handRank = HandRank::HIGH_CARD;
        primaryValue = ranksByFreq[0].first;
        // Kickers for tie-breaking
        return HandStrength{handRank, primaryValue, secondaryValue, kickers};
    }
    
    // Helper methods for hand evaluation
    
    bool isHandFlush(const std::vector<Card>& cards) const {
        if (cards.empty()) return false;
        
        Suit firstSuit = cards[0].suit;
        for (size_t i = 1; i < cards.size(); ++i) {
            if (cards[i].suit != firstSuit) {
                return false;
            }
        }
        return true;
    }
    
    bool isHandStraight(const std::vector<Card>& cards) const {
        if (cards.size() < 5) return false;
        
        // Extract ranks and sort them
        std::vector<int> ranks;
        for (const auto& card : cards) {
            ranks.push_back(static_cast<int>(card.rank));
        }
        std::sort(ranks.begin(), ranks.end());
        
        // Remove duplicates
        ranks.erase(std::unique(ranks.begin(), ranks.end()), ranks.end());
        
        // Check for standard straight
        if (ranks.size() >= 5) {
            for (size_t i = 0; i <= ranks.size() - 5; ++i) {
                if (ranks[i+4] - ranks[i] == 4) {
                    return true;
                }
            }
        }
        
        // Check for A-5 straight
        if (std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::ACE)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::TWO)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::THREE)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::FOUR)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::FIVE)) != ranks.end()) {
            return true;
        }
        
        return false;
    }
    
    int getHighestRank(const std::vector<Card>& cards) const {
        int highest = 0;
        for (const auto& card : cards) {
            highest = std::max(highest, static_cast<int>(card.rank));
        }
        return highest;
    }
    
    bool hasRank(const std::vector<Card>& cards, Rank rank) const {
        for (const auto& card : cards) {
            if (card.rank == rank) {
                return true;
            }
        }
        return false;
    }
};

}