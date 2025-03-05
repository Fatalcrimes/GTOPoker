#include "utils/Random.hpp"
#include <chrono>
#include <stdexcept>
#include <numeric>

namespace poker {

// Initialize the singleton instance
Random& Random::getInstance() {
    static Random instance;
    return instance;
}

Random::Random() {
    // Initialize with time-based seed by default
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator_.seed(seed);
}

void Random::seed(unsigned int seed) {
    std::lock_guard<std::mutex> lock(mutex_);
    generator_.seed(seed);
}

int Random::getInt(int min, int max) {
    if (min > max) {
        std::swap(min, max);
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::uniform_int_distribution<int> dist(min, max);
    return dist(generator_);
}

double Random::getDouble(double min, double max) {
    if (min > max) {
        std::swap(min, max);
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::uniform_real_distribution<double> dist(min, max);
    return dist(generator_);
}

bool Random::getBool(double probability) {
    if (probability < 0.0 || probability > 1.0) {
        throw std::invalid_argument("Probability must be between 0 and 1");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    std::bernoulli_distribution dist(probability);
    return dist(generator_);
}

std::mt19937& Random::getGenerator() {
    // Note: This is not thread-safe, use with caution
    return generator_;
}

// Non-specialized helper functions for Action
Action sampleActionFromMap(const std::unordered_map<Action, double, std::hash<Action>>& distribution, std::mt19937& generator) {
    if (distribution.empty()) {
        throw std::invalid_argument("Cannot sample from empty distribution");
    }
    
    // Convert map to vectors
    std::vector<Action> actions;
    std::vector<double> probs;
    
    double sum = 0.0;
    for (const auto& [action, prob] : distribution) {
        if (prob > 0.0) {  // Only include non-zero probabilities
            actions.push_back(action);
            probs.push_back(prob);
            sum += prob;
        }
    }
    
    // Handle edge cases
    if (actions.empty()) {
        // All probabilities were zero - pick uniformly from original actions
        for (const auto& [action, _] : distribution) {
            actions.push_back(action);
        }
        
        std::uniform_int_distribution<size_t> dist(0, actions.size() - 1);
        return actions[dist(generator)];
    }
    
    // Normalize probabilities if needed
    if (std::abs(sum - 1.0) > 1e-6) {
        for (auto& p : probs) {
            p /= sum;
        }
    }
    
    // Create distribution and sample
    std::discrete_distribution<size_t> dist(probs.begin(), probs.end());
    size_t index = dist(generator);
    
    return actions[index];
}

// Non-specialized helper functions for Action
Action sampleActionUniform(const std::vector<Action>& elements, std::mt19937& generator) {
    if (elements.empty()) {
        throw std::invalid_argument("Cannot sample from empty vector");
    }
    
    std::uniform_int_distribution<size_t> dist(0, elements.size() - 1);
    return elements[dist(generator)];
}

// Specialization for RegretTable::ActionHash and StrategyTable::ActionHash
Action Random::sample(const std::unordered_map<Action, double, RegretTable::ActionHash>& distribution) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert to a std::hash<Action> map for helper function
    std::unordered_map<Action, double, std::hash<Action>> standardMap;
    for (const auto& [action, prob] : distribution) {
        standardMap[action] = prob;
    }
    
    return sampleActionFromMap(standardMap, generator_);
}

Action Random::sample(const std::unordered_map<Action, double, StrategyTable::ActionHash>& distribution) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert to a std::hash<Action> map for helper function
    std::unordered_map<Action, double, std::hash<Action>> standardMap;
    for (const auto& [action, prob] : distribution) {
        standardMap[action] = prob;
    }
    
    return sampleActionFromMap(standardMap, generator_);
}

} // namespace poker