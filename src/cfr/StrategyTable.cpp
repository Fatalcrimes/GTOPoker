#include "cfr/StrategyTable.hpp"
#include "utils/Serialization.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace poker {

StrategyTable::StrategyTable() {
    // Nothing to initialize
}

void StrategyTable::setStrategy(const std::string& infoSet, const Action& action, double probability) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Set the strategy probability
    currentStrategy_[infoSet][action] = probability;
}

double StrategyTable::getStrategy(const std::string& infoSet, const Action& action) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists
    auto infoSetIt = currentStrategy_.find(infoSet);
    if (infoSetIt == currentStrategy_.end()) {
        return 0.0;
    }
    
    // Check if the action exists in this info set
    const auto& actionStrategies = infoSetIt->second;
    auto actionIt = actionStrategies.find(action);
    if (actionIt == actionStrategies.end()) {
        return 0.0;
    }
    
    return actionIt->second;
}

std::unordered_map<Action, double, ActionHash> 
StrategyTable::getStrategies(const std::string& infoSet) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists
    auto it = currentStrategy_.find(infoSet);
    if (it == currentStrategy_.end()) {
        return {};
    }
    
    return it->second;
}

void StrategyTable::addToStrategySum(const std::string& infoSet, const Action& action, double probability) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Add the probability to the strategy sum
    strategySum_[infoSet][action] += probability;
}

double StrategyTable::getAverageStrategy(const std::string& infoSet, const Action& action) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists in strategy sum
    auto infoSetIt = strategySum_.find(infoSet);
    if (infoSetIt == strategySum_.end()) {
        return 0.0;
    }
    
    // Check if the action exists in this info set
    const auto& actionSums = infoSetIt->second;
    auto actionIt = actionSums.find(action);
    if (actionIt == actionSums.end()) {
        return 0.0;
    }
    
    // Calculate the sum of all probabilities for this info set
    double sum = 0.0;
    for (const auto& pair : actionSums) {
        sum += pair.second;
    }
    
    // Calculate the average strategy
    if (sum > 0.0) {
        return actionIt->second / sum;
    } else {
        // If sum is 0, return uniform strategy
        return 1.0 / actionSums.size();
    }
}

std::unordered_map<Action, double, ActionHash> 
StrategyTable::getAverageStrategies(const std::string& infoSet) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Check if the info set exists in strategy sum
    auto infoSetIt = strategySum_.find(infoSet);
    if (infoSetIt == strategySum_.end()) {
        return {};
    }
    
    const auto& actionSums = infoSetIt->second;
    
    // Calculate the sum of all probabilities for this info set
    double sum = 0.0;
    for (const auto& pair : actionSums) {
        sum += pair.second;
    }
    
    // Calculate average strategies for all actions
    std::unordered_map<Action, double, ActionHash> averageStrategies;
    
    if (sum > 0.0) {
        // Normalize by the sum
        for (const auto& pair : actionSums) {
            averageStrategies[pair.first] = pair.second / sum;
        }
    } else {
        // If sum is 0, return uniform strategy
        double uniformProb = 1.0 / actionSums.size();
        for (const auto& pair : actionSums) {
            averageStrategies[pair.first] = uniformProb;
        }
    }
    
    return averageStrategies;
}

bool StrategyTable::hasInfoSet(const std::string& infoSet) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    return currentStrategy_.find(infoSet) != currentStrategy_.end();
}

void StrategyTable::clear() {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    currentStrategy_.clear();
    strategySum_.clear();
}

size_t StrategyTable::size() const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    return currentStrategy_.size();
}

bool StrategyTable::saveToFile(const std::string& filename) const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Save both current strategy and strategy sum
    std::string currentStrategyFile = filename + ".current";
    std::string strategySumFile = filename + ".sum";
    
    bool currentSaved = Serialization::saveToFile(currentStrategy_, currentStrategyFile);
    bool sumSaved = Serialization::saveToFile(strategySum_, strategySumFile);
    
    return currentSaved && sumSaved;
}

bool StrategyTable::loadFromFile(const std::string& filename) {
    // Write lock for thread safety
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Load both current strategy and strategy sum
    std::string currentStrategyFile = filename + ".current";
    std::string strategySumFile = filename + ".sum";
    
    bool currentLoaded = Serialization::loadFromFile(currentStrategy_, currentStrategyFile);
    bool sumLoaded = Serialization::loadFromFile(strategySum_, strategySumFile);
    
    return currentLoaded && sumLoaded;
}

std::vector<std::string> StrategyTable::getAllInfoSets() const {
    // Read lock for thread safety
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<std::string> infoSets;
    infoSets.reserve(currentStrategy_.size());
    
    for (const auto& pair : currentStrategy_) {
        infoSets.push_back(pair.first);
    }
    
    return infoSets;
}

// ActionHash implementation
std::size_t StrategyTable::ActionHash::operator()(const Action& action) const {
    // Combine the action type and amount into a single hash value
    std::size_t h1 = std::hash<int>()(static_cast<int>(action.getType()));
    std::size_t h2 = std::hash<double>()(action.getAmount());
    
    // Combine hashes
    return h1 ^ (h2 << 1);
}

} // namespace poker