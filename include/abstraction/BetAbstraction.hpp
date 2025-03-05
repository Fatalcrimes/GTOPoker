#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <string>

#include "game/PokerDefs.hpp"
#include "game/Action.hpp"

namespace poker {

/**
 * BetAbstraction reduces the complexity of the game by simplifying the action
 * space, particularly the continuous space of bet sizes.
 */
class BetAbstraction {
public:
    // Abstraction levels
    enum class Level {
        NONE,       // No abstraction (continuous bets)
        MINIMAL,    // Minimal bet sizes (e.g., 0.5x pot, pot, all-in)
        STANDARD,   // Standard sizing (more granular)
        DETAILED    // Detailed sizing
    };
    
    // Constructors
    BetAbstraction(Level level = Level::STANDARD);
    
    // Get abstracted bet sizes for current game state
    std::vector<Action> getAbstractedActions(
        const std::vector<Action>& validActions,
        double potSize,
        double stackSize,
        BettingRound round
    ) const;
    
    // Abstract a specific action
    Action abstractAction(
        const Action& action,
        double potSize,
        double stackSize,
        BettingRound round
    ) const;
    
    // Get abstraction level
    Level getLevel() const { return level_; }
    
    // Get abstraction name
    std::string getName() const;
    
    // Create abstraction based on level
    static std::shared_ptr<BetAbstraction> create(Level level);

private:
    // Define bet sizing for each round and abstraction level
    struct BetSizing {
        std::vector<double> preflopRaiseMultipliers;  // Multipliers relative to BB
        std::vector<double> postflopBetMultipliers;   // Multipliers relative to pot
    };
    
    // Data members
    Level level_;
    BetSizing betSizing_;
    
    // Helper methods
    std::vector<double> getPreflopRaiseSizes(double bigBlind, double maxSize) const;
    std::vector<double> getPostflopBetSizes(double potSize, double maxSize) const;
    
    // Find closest abstracted bet size
    double findClosestBetSize(double targetSize, const std::vector<double>& sizes) const;
};

} // namespace poker