#pragma once

#include <string>
#include <vector>
#include "game/PokerDefs.hpp"

namespace poker {

enum class ActionType : uint8_t {
    FOLD,
    CHECK,
    CALL,
    BET,
    RAISE
};

class Action {
public:
    // Constructors
    Action() = default;
    Action(ActionType type, double amount = 0.0);
    
    // Getters
    ActionType getType() const { return type_; }
    double getAmount() const { return amount_; }
    
    // String representation
    std::string toString() const;
    
    // Equality operators
    bool operator==(const Action& other) const;
    bool operator!=(const Action& other) const;
    
    // Factory methods for common actions
    static Action fold() { return Action(ActionType::FOLD); }
    static Action check() { return Action(ActionType::CHECK); }
    static Action call(double amount) { return Action(ActionType::CALL, amount); }
    static Action bet(double amount) { return Action(ActionType::BET, amount); }
    static Action raise(double amount) { return Action(ActionType::RAISE, amount); }

private:
    ActionType type_ = ActionType::FOLD;
    double amount_ = 0.0;
};

// Helper functions
std::string actionTypeToString(ActionType type);

// Class to represent action history
class ActionHistory {
public:
    // Add an action to the history
    void addAction(Position position, const Action& action);
    
    // Get all actions in the history
    const std::vector<std::pair<Position, Action>>& getActions() const { return actions_; }
    
    // Get actions for a specific betting round
    std::vector<std::pair<Position, Action>> getActionsForRound(BettingRound round) const;
    
    // Clear history for a new hand
    void clear();
    
    // String representation of history
    std::string toString() const;

private:
    std::vector<std::pair<Position, Action>> actions_;
    std::vector<int> roundStartIndices_ = {0}; // Indices where new betting rounds start
};

} // namespace poker