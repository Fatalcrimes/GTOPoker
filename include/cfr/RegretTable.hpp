#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <shared_mutex>

#include "game/Action.hpp"

namespace poker {

/**
 * RegretTable stores and manages regrets for information sets.
 * This is a key component of the CFR algorithm.
 */
class RegretTable {
public:
    // Constructor
    RegretTable();
    
    // Add regret for an action at an information set
    void addRegret(const std::string& infoSet, const Action& action, double regret);
    
    // Get regret for an action at an information set
    double getRegret(const std::string& infoSet, const Action& action) const;
    
    // Get all regrets for an information set
    std::unordered_map<Action, double, ActionHash> getRegrets(const std::string& infoSet) const;
    
    // Check if an information set exists in the table
    bool hasInfoSet(const std::string& infoSet) const;
    
    // Clear all regrets
    void clear();
    
    // Size of the regret table (number of info sets)
    size_t size() const;
    
    // Serialize/deserialize regrets
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    
    // Get all info sets
    std::vector<std::string> getAllInfoSets() const;

private:
    // Custom hash function for Actions
    struct ActionHash {
        std::size_t operator()(const Action& action) const;
    };
    
    // Type definitions for nested maps
    using ActionRegretMap = std::unordered_map<Action, double, ActionHash>;
    using InfoSetRegretMap = std::unordered_map<std::string, ActionRegretMap>;
    
    // Regrets data
    InfoSetRegretMap regrets_;
    
    // Thread safety
    mutable std::shared_mutex mutex_;
};

} // namespace poker