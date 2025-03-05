#include "abstraction/BetAbstraction.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

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
    
    // Group actions by type
    bool hasFold = false;
    bool hasCheck = false;
    bool hasCall = false;
    double callAmount = 0.0;
    std::vector<double> betAmounts;
    std::vector<double> raiseAmounts;
    
    for (const auto& action : validActions) {
        switch (action.getType()) {
            case ActionType::FOLD:
                hasFold = true;
                break;
            case ActionType::CHECK:
                hasCheck = true;
                break;
            case ActionType::CALL:
                hasCall = true;
                callAmount = action.getAmount();
                break;
            case ActionType::BET:
                betAmounts.push_back(action.getAmount());
                break;
            case ActionType::RAISE:
                raiseAmounts.push_back(action.getAmount());
                break;
        }
    }
    
    // Add fold, check, call actions as-is
    if (hasFold) {
        abstractedActions.push_back(Action::fold());
    }
    
    if (hasCheck) {
        abstractedActions.push_back(Action::check());
    }
    
    if (hasCall) {
        abstractedActions.push_back(Action::call(callAmount));
    }
    
    // Abstract bet sizes
    if (!betAmounts.empty()) {
        bool isPreflop = round == BettingRound::PREFLOP;
        const auto& multipliers = isPreflop ? 
                                betSizing_.preflopRaiseMultipliers : 
                                betSizing_.postflopBetMultipliers;
        
        if (multipliers.empty()) {
            // No abstraction, use original bets
            for (double amount : betAmounts) {
                abstractedActions.push_back(Action::bet(amount));
            }
        } else {
            // Abstracted bets
            std::vector<double> abstractedBets = getAbstractedBetSizes(potSize, stackSize, isPreflop);
            
            for (double amount : abstractedBets) {
                if (amount <= stackSize) {
                    abstractedActions.push_back(Action::bet(amount));
                }
            }
        }
    }
    
    // Abstract raise sizes
    if (!raiseAmounts.empty()) {
        bool isPreflop = round == BettingRound::PREFLOP;
        const auto& multipliers = isPreflop ? 
                                betSizing_.preflopRaiseMultipliers : 
                                betSizing_.postflopBetMultipliers;
        
        if (multipliers.empty()) {
            // No abstraction, use original raises
            for (double amount : raiseAmounts) {
                abstractedActions.push_back(Action::raise(amount));
            }
        } else {
            // Abstracted raises
            std::vector<double> abstractedRaises = getAbstractedRaiseSizes(potSize, callAmount, stackSize, isPreflop);
            
            for (double amount : abstractedRaises) {
                if (amount <= stackSize && amount > callAmount) {
                    abstractedActions.push_back(Action::raise(amount));
                }
            }
        }
    }
    
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
            sizes.push_back(std::min(reference * mult, maxSize));
        }
    }
    
    // Remove duplicates and sort
    std::sort(sizes.begin(), sizes.end());
    sizes.erase(std::unique(sizes.begin(), sizes.end()), sizes.end());
    
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

} // namespace poker