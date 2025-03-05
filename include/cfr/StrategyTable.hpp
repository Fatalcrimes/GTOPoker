#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include "game/Action.hpp"

namespace poker {

/**
 * StrategyTable stores and manages strategy probabilities for information sets.
 * It maintains both current strategy and strategy sum for average strategy calculation.
 */
class StrategyTable {
public:
    // Custom hash function for Actions
    struct ActionHash {
        std::size_t operator()(const Action& action) const;
    };
    
    // Constructor
    StrategyTable();
    
    // Set strategy for an action at an information set
    void setStrategy(const std::string& infoSet, const Action& action, double probability);
    
    // Get strategy probability for an action at an information set
    double getStrategy(const std::string& infoSet, const Action& action) const;
    
    // Get all strategy probabilities for an information set
    std::unordered_map<Action, double, ActionHash> getStrategies(const std::string& infoSet) const;
    
    // Add to strategy sum (for computing average strategy)
    void addToStrategySum(const std::string& infoSet, const Action& action, double probability);
    
    // Get average strategy for an action at an information set
    double getAverageStrategy(const std::string& infoSet, const Action& action) const;
    
    // Get all average strategy probabilities for an information set
    std::unordered_map<Action, double, ActionHash> getAverageStrategies(const std::string& infoSet) const;
    
    // Check if an information set exists in the table
    bool hasInfoSet(const std::string& infoSet) const;
    
    // Clear all strategies
    void clear();
    
    // Size of the strategy table (number of info sets)
    size_t size() const;
    
    // Serialize/deserialize strategies
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    // Get all info sets
    std::vector<std::string> getAllInfoSets() const;

private:
    // Type definitions for nested maps
    using ActionStrategyMap = std::unordered_map<Action, double, ActionHash>;
    using InfoSetStrategyMap = std::unordered_map<std::string, ActionStrategyMap>;
    
    // Data members
    InfoSetStrategyMap currentStrategy_;
    InfoSetStrategyMap strategySum_;
    
    // Thread safety
    mutable std::shared_mutex mutex_;
};

} // namespace poker