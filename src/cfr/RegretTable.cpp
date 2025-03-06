#include "cfr/RegretTable.hpp"
#include "utils/Serialization.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace poker {

RegretTable::RegretTable() {
    // Nothing to initialize
}

void RegretTable::addRegret(const std::string& infoSet, const Action& action, double regret) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Add the regret to the existing value
    regrets_[infoSet][action] += regret;
    
    // CFR+ modification: ensure regrets are non-negative
    // This helps with convergence speed
    if (regrets_[infoSet][action] < 0) {
        regrets_[infoSet][action] = 0;
    }
}

double RegretTable::getRegret(const std::string& infoSet, const Action& action) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists
    auto infoSetIt = regrets_.find(infoSet);
    if (infoSetIt == regrets_.end()) {
        return 0.0;
    }
    
    // Check if the action exists in this info set
    const auto& actionRegrets = infoSetIt->second;
    auto actionIt = actionRegrets.find(action);
    if (actionIt == actionRegrets.end()) {
        return 0.0;
    }
    
    return actionIt->second;
}

std::unordered_map<Action, double, RegretTable::ActionHash> 
RegretTable::getRegrets(const std::string& infoSet) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists
    auto it = regrets_.find(infoSet);
    if (it == regrets_.end()) {
        return {};
    }
    
    return it->second;
}

bool RegretTable::hasInfoSet(const std::string& infoSet) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    return regrets_.find(infoSet) != regrets_.end();
}

void RegretTable::clear() {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    regrets_.clear();
}

size_t RegretTable::size() const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    return regrets_.size();
}

bool RegretTable::saveToFile(const std::string& filename) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Use the Serialization utility to save the regrets
    return Serialization::saveToFile<double, ActionHash>(regrets_, filename);
}

bool RegretTable::loadFromFile(const std::string& filename) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Use the Serialization utility to load the regrets
    return Serialization::loadFromFile<double, ActionHash>(regrets_, filename);
}

std::vector<std::string> RegretTable::getAllInfoSets() const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::string> infoSets;
    infoSets.reserve(regrets_.size());
    
    for (const auto& pair : regrets_) {
        infoSets.push_back(pair.first);
    }
    
    return infoSets;
}

void RegretTable::prune(double threshold) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Keep track of entries to remove
    std::vector<std::string> infoSetsToRemove;
    
    // Find info sets where all regrets are below threshold
    for (const auto& [infoSet, actionRegrets] : regrets_) {
        bool allSmall = true;
        double maxRegret = 0.0;
        
        for (const auto& [action, regret] : actionRegrets) {
            maxRegret = std::max(maxRegret, std::abs(regret));
            if (std::abs(regret) > threshold) {
                allSmall = false;
                break;
            }
        }
        
        // Only remove if all regrets are small and this isn't a frequently visited state
        if (allSmall && maxRegret < threshold) {
            infoSetsToRemove.push_back(infoSet);
        }
    }
    
    // Remove the identified info sets
    for (const auto& infoSet : infoSetsToRemove) {
        regrets_.erase(infoSet);
    }
}

// ActionHash implementation
std::size_t RegretTable::ActionHash::operator()(const Action& action) const {
    // Combine the action type and amount into a single hash value
    std::size_t h1 = std::hash<int>()(static_cast<int>(action.getType()));
    std::size_t h2 = std::hash<double>()(action.getAmount());
    
    // Combine hashes
    return h1 ^ (h2 << 1);
}

} // namespace poker