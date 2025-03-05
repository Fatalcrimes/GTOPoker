#include "abstraction/BetAbstraction.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include "utils/Logger.hpp"

namespace poker {

BetAbstraction::BetAbstraction(Level level) : level_(level) {
    // Initialize bet sizing based on abstraction level
    switch (level) {
        case Level::NONE:
            // No abstraction - continuous bet sizes
            betSizing_ = {
                {}, // Empty for continuous sizing
                {}
            };
            break;
            
        case Level::MINIMAL:
            // Minimal abstraction - few bet sizes
            betSizing_ = {
                {2.5, 3.5, -1}, // Preflop: 2.5BB, 3.5BB, all-in
                {0.5, 1.0, -1}  // Postflop: half-pot, pot, all-in
            };
            break;
            
        case Level::STANDARD:
            // Standard abstraction - moderate number of bet sizes
            betSizing_ = {
                {2.0, 2.5, 3.0, 4.0, -1}, // Preflop: 2BB, 2.5BB, 3BB, 4BB, all-in
                {0.33, 0.5, 0.75, 1.0, 1.5, -1} // Postflop: 1/3 pot, 1/2 pot, 3/4 pot, pot, 1.5x pot, all-in
            };
            break;
            
        case Level::DETAILED:
            // Detailed abstraction - many bet sizes
            betSizing_ = {
                {2.0, 2.25, 2.5, 2.75, 3.0, 3.5, 4.0, 5.0, -1}, // Preflop: many sizes
                {0.25, 0.33, 0.5, 0.66, 0.75, 1.0, 1.25, 1.5, 2.0, -1} // Postflop: many sizes
            };
            break;
    }
}

std::vector<Action> BetAbstraction::getAbstractedActions(
    const std::vector<Action>& validActions,
    double potSize,
    double stackSize,
    BettingRound round
) const {
    // If no abstraction, return original actions
    if (level_ == Level::NONE) {
        return validActions;
    }
    
    std::vector<Action> abstractedActions;
    
    // Always include fold, check, call actions
    for (const auto& action : validActions) {
        if (action.getType() == ActionType::FOLD || 
            action.getType() == ActionType::CHECK || 
            action.getType() == ActionType::CALL) {
            abstractedActions.push_back(action);
        }
    }
    
    // For bets and raises, select a subset based on abstraction level
    std::vector<Action> betActions;
    std::vector<Action> raiseActions;
    
    for (const auto& action : validActions) {
        if (action.getType() == ActionType::BET) {
            betActions.push_back(action);
        } else if (action.getType() == ActionType::RAISE) {
            raiseActions.push_back(action);
        }
    }
    
    // Sort bet/raise actions by amount
    auto sortByAmount = [](const Action& a, const Action& b) {
        return a.getAmount() < b.getAmount();
    };
    
    std::sort(betActions.begin(), betActions.end(), sortByAmount);
    std::sort(raiseActions.begin(), raiseActions.end(), sortByAmount);
    
    // Select a subset of bet/raise actions based on abstraction level
    int numBetActions = betActions.size();
    int numRaiseActions = raiseActions.size();
    
    // Determine how many actions to include based on level
    int maxBetActions = 0;
    switch (level_) {
        case Level::MINIMAL:  maxBetActions = 2; break; // Small number
        case Level::STANDARD: maxBetActions = 3; break; // Medium number
        case Level::DETAILED: maxBetActions = 5; break; // Larger number
    }
    
    // Select evenly spaced bet actions
    if (!betActions.empty()) {
        // Always include min bet and max bet
        abstractedActions.push_back(betActions.front());
        
        if (betActions.size() > 1) {
            abstractedActions.push_back(betActions.back());
        }
        
        // Add intermediate bets if we have more than 2 actions
        if (numBetActions > 2 && maxBetActions > 2) {
            int step = (numBetActions - 1) / (maxBetActions - 1);
            if (step > 0) {
                for (int i = step; i < numBetActions - 1; i += step) {
                    abstractedActions.push_back(betActions[i]);
                    if (abstractedActions.size() >= maxBetActions) break;
                }
            }
        }
    }
    
    // Do the same for raise actions
    if (!raiseActions.empty()) {
        // Always include min raise and max raise
        abstractedActions.push_back(raiseActions.front());
        
        if (raiseActions.size() > 1) {
            abstractedActions.push_back(raiseActions.back());
        }
        
        // Add intermediate raises if we have more than 2 actions
        if (numRaiseActions > 2 && maxBetActions > 2) {
            int step = (numRaiseActions - 1) / (maxBetActions - 1);
            if (step > 0) {
                for (int i = step; i < numRaiseActions - 1; i += step) {
                    abstractedActions.push_back(raiseActions[i]);
                    if (abstractedActions.size() >= 2 * maxBetActions) break; // Limit total actions
                }
            }
        }
    }
    
    // Add debug logging
    debugLogActions(validActions, abstractedActions);
    
    return abstractedActions;
}

Action BetAbstraction::abstractAction(
    const Action& action,
    double potSize,
    double stackSize,
    BettingRound round
) const {
    // For fold, check, call - no abstraction needed
    if (action.getType() == ActionType::FOLD ||
        action.getType() == ActionType::CHECK ||
        action.getType() == ActionType::CALL) {
        return action;
    }
    
    // For bets and raises, find the closest abstracted size
    bool isPreflop = round == BettingRound::PREFLOP;
    
    if (action.getType() == ActionType::BET) {
        // No abstraction needed for Level::NONE
        if (level_ == Level::NONE) {
            return action;
        }
        
        // Get abstracted bet sizes
        std::vector<double> abstractedSizes = getAbstractedBetSizes(potSize, stackSize, isPreflop);
        
        // Find closest
        if (!abstractedSizes.empty()) {
            double closestSize = findClosestBetSize(action.getAmount(), abstractedSizes);
            return Action::bet(closestSize);
        }
    }
    
    if (action.getType() == ActionType::RAISE) {
        // No abstraction needed for Level::NONE
        if (level_ == Level::NONE) {
            return action;
        }
        
        // Assume callAmount is 0 for simplicity (will be replaced with actual call amount in practice)
        double callAmount = 0.0;
        
        // Get abstracted raise sizes
        std::vector<double> abstractedSizes = getAbstractedRaiseSizes(potSize, callAmount, stackSize, isPreflop);
        
        // Find closest
        if (!abstractedSizes.empty()) {
            double closestSize = findClosestBetSize(action.getAmount(), abstractedSizes);
            return Action::raise(closestSize);
        }
    }
    
    // If we couldn't abstract, return the original action
    return action;
}

std::string BetAbstraction::getName() const {
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

std::shared_ptr<BetAbstraction> BetAbstraction::create(Level level) {
    return std::make_shared<BetAbstraction>(level);
}

std::vector<double> BetAbstraction::getAbstractedBetSizes(
    double potSize,
    double maxSize,
    bool isPreflop
) const {
    std::vector<double> sizes;
    
    const auto& multipliers = isPreflop ? 
                            betSizing_.preflopRaiseMultipliers : 
                            betSizing_.postflopBetMultipliers;
    
    // For preflop, multipliers are relative to big blind
    // For postflop, multipliers are relative to pot size
    double reference = isPreflop ? BIG_BLIND : potSize;
    
    for (double mult : multipliers) {
        if (mult < 0) {
            // Special case: all-in
            sizes.push_back(maxSize);
        } else {
            // Calculate bet size and round to 2 decimal places for consistency
            double rawSize = reference * mult;
            double roundedSize = std::round(rawSize * 100) / 100.0;
            sizes.push_back(std::min(roundedSize, maxSize));
        }
    }
    
    // Remove duplicates and sort
    std::sort(sizes.begin(), sizes.end());
    sizes.erase(std::unique(sizes.begin(), sizes.end(), 
        [](double a, double b) { return std::abs(a - b) < 0.01; }), 
        sizes.end());
    
    return sizes;
}

std::vector<double> BetAbstraction::getAbstractedRaiseSizes(
    double potSize,
    double callAmount,
    double maxSize,
    bool isPreflop
) const {
    // For raises, we need to account for the call amount
    std::vector<double> betSizes = getAbstractedBetSizes(potSize, maxSize, isPreflop);
    std::vector<double> raiseSizes;
    
    for (double size : betSizes) {
        if (size > callAmount) {
            raiseSizes.push_back(size);
        }
    }
    
    return raiseSizes;
}

double BetAbstraction::findClosestBetSize(
    double targetSize,
    const std::vector<double>& sizes
) const {
    if (sizes.empty()) {
        return targetSize;  // No abstraction available
    }
    
    // Find the closest size (using linear search for simplicity)
    double closestSize = sizes[0];
    double minDiff = std::abs(targetSize - closestSize);
    
    for (size_t i = 1; i < sizes.size(); ++i) {
        double diff = std::abs(targetSize - sizes[i]);
        if (diff < minDiff) {
            minDiff = diff;
            closestSize = sizes[i];
        }
    }
    
    return closestSize;
}

void BetAbstraction::debugLogActions(const std::vector<Action>& originalActions, 
                                    const std::vector<Action>& abstractedActions) const {
    // Log the original actions
    std::ostringstream originalOss;
    originalOss << "Original actions: ";
    for (const auto& action : originalActions) {
        originalOss << action.toString() << ", ";
    }
    LOG_DEBUG(originalOss.str());
    
    // Log the abstracted actions
    std::ostringstream abstractedOss;
    abstractedOss << "Abstracted actions: ";
    for (const auto& action : abstractedActions) {
        abstractedOss << action.toString() << ", ";
    }
    LOG_DEBUG(abstractedOss.str());
}

} // namespace poker