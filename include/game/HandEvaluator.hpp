#pragma once

#include <array>
#include <vector>
#include <memory>
#include "game/PokerDefs.hpp"
#include "pokerstove/peval/PokerHandEvaluator.h"

namespace poker {

/**
 * HandEvaluator provides poker hand evaluation and equity calculation
 * using the PokerStove library for accurate results.
 */
class HandEvaluator {
public:
    // Constructor
    HandEvaluator();
    
    // Evaluate hand strength for a player (2 hole cards + community cards)
    HandStrength evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                             const std::vector<Card>& communityCards) const;
    
    // Find the best 5-card hand from all available cards
    std::vector<Card> findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                  const std::vector<Card>& communityCards) const;

    // Calculate equity against random hands
    double calculateEquity(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                          const std::vector<Card>& communityCards,
                          int numTrials = 10000) const;

private:
    std::shared_ptr<pokerstove::PokerHandEvaluator> evaluator_;
    
    // Convert PokerStove evaluation to HandStrength
    HandStrength convertToHandStrength(const pokerstove::PokerHandEvaluation& eval) const;
};

} // namespace poker