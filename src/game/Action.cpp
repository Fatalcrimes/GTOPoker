#include "game/Action.hpp"
#include <sstream>
#include <iomanip>

namespace poker {

Action::Action(ActionType type, double amount) 
    : type_(type), amount_(amount) {
    // Validate action
    if ((type == ActionType::FOLD || type == ActionType::CHECK) && amount != 0.0) {
        throw std::invalid_argument("FOLD and CHECK actions should have amount of 0");
    }
    
    if ((type == ActionType::CALL || type == ActionType::BET || type == ActionType::RAISE) && amount <= 0.0) {
        throw std::invalid_argument("CALL, BET, and RAISE actions must have positive amount");
    }
}

std::string Action::toString() const {
    std::ostringstream oss;
    oss << actionTypeToString(type_);
    
    if (type_ == ActionType::CALL || type_ == ActionType::BET || type_ == ActionType::RAISE) {
        oss << " " << amount_;
    }
    
    return oss.str();
}

bool Action::operator==(const Action& other) const {
    // For FOLD and CHECK, only compare the type
    if ((type_ == ActionType::FOLD || type_ == ActionType::CHECK) &&
        (other.type_ == ActionType::FOLD || other.type_ == ActionType::CHECK)) {
        return type_ == other.type_;
    }
    
    // For other actions, compare both type and amount
    return type_ == other.type_ && amount_ == other.amount_;
}

bool Action::operator!=(const Action& other) const {
    return !(*this == other);
}

std::string actionTypeToString(ActionType type) {
    switch (type) {
        case ActionType::FOLD:  return "FOLD";
        case ActionType::CHECK: return "CHECK";
        case ActionType::CALL:  return "CALL";
        case ActionType::BET:   return "BET";
        case ActionType::RAISE: return "RAISE";
        default:                return "UNKNOWN";
    }
}

void ActionHistory::addAction(Position position, const Action& action) {
    actions_.emplace_back(position, action);
}

std::vector<std::pair<Position, Action>> ActionHistory::getActionsForRound(BettingRound round) const {
    // Ensure we have the right number of round markers
    if (static_cast<size_t>(round) >= roundStartIndices_.size()) {
        return {};
    }
    
    // Calculate the start and end indices for this round
    size_t startIdx = roundStartIndices_[static_cast<size_t>(round)];
    size_t endIdx = (static_cast<size_t>(round) + 1 < roundStartIndices_.size()) 
                   ? roundStartIndices_[static_cast<size_t>(round) + 1] : actions_.size();
    
    // Extract the actions for this round
    std::vector<std::pair<Position, Action>> roundActions;
    for (size_t i = startIdx; i < endIdx; ++i) {
        roundActions.push_back(actions_[i]);
    }
    
    return roundActions;
}

void ActionHistory::clear() {
    actions_.clear();
    roundStartIndices_ = {0};
}

std::string ActionHistory::toString() const {
    std::ostringstream oss;
    
    for (size_t round = 0; round < roundStartIndices_.size(); ++round) {
        if (round > 0) {
            oss << " | ";
        }
        
        // Get the end index for this round
        size_t endIdx = (round + 1 < roundStartIndices_.size()) 
                        ? roundStartIndices_[round + 1] : actions_.size();
        
        // Add actions for this round
        for (int i = roundStartIndices_[round]; i < endIdx; ++i) {
            if (i > roundStartIndices_[round]) {
                oss << ", ";
            }
            
            const auto& [position, action] = actions_[i];
            oss << positionToString(position) << ":" << action.toString();
        }
    }
    
    return oss.str();
}

} // namespace poker