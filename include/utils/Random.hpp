#pragma once

#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <mutex>
#include "game/Action.hpp"
#include "game/PokerDefs.hpp"
#include "cfr/RegretTable.hpp"
#include "cfr/StrategyTable.hpp"

namespace poker {

/**
 * Random provides thread-safe random number generation utilities.
 */
class Random {
public:
    // Get singleton instance
    static Random& getInstance();
    
    // Seed the random number generator
    void seed(unsigned int seed);
    
    // Get a random integer in range [min, max]
    int getInt(int min, int max);
    
    // Get a random double in range [min, max)
    double getDouble(double min, double max);
    
    // Get a random boolean with specified probability
    bool getBool(double probability = 0.5);
    
    // Shuffle a vector
    template <typename T>
    void shuffle(std::vector<T>& vec);
    
    // Sample from discrete distribution with custom hash functions
    Action sample(const std::unordered_map<Action, double, RegretTable::ActionHash>& distribution);
    Action sample(const std::unordered_map<Action, double, StrategyTable::ActionHash>& distribution);
    
    // Sample from discrete distribution
    template <typename T>
    T sample(const std::unordered_map<T, double>& distribution);
    
    // Sample from vector of weights
    template <typename T>
    T sample(const std::vector<T>& elements, const std::vector<double>& weights);
    
    // Sample uniformly from vector
    template <typename T>
    T sampleUniform(const std::vector<T>& elements);
    
    // Get underlying generator (use with caution, not thread-safe)
    std::mt19937& getGenerator();
    
    // Destructor
    ~Random() = default;

private:
    // Private constructor for singleton
    Random();
    
    // Data members
    std::mt19937 generator_;
    std::mutex mutex_;
    
    // Prevent copying
    Random(const Random&) = delete;
    Random& operator=(const Random&) = delete;
};

// Template implementations

template <typename T>
void Random::shuffle(std::vector<T>& vec) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::shuffle(vec.begin(), vec.end(), generator_);
}

template <typename T>
T Random::sample(const std::unordered_map<T, double>& distribution) {
    if (distribution.empty()) {
        throw std::invalid_argument("Cannot sample from empty distribution");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Convert map to vectors
    std::vector<T> elements;
    std::vector<double> weights;
    
    elements.reserve(distribution.size());
    weights.reserve(distribution.size());
    
    for (const auto& pair : distribution) {
        elements.push_back(pair.first);
        weights.push_back(pair.second);
    }
    
    // Create discrete distribution
    std::discrete_distribution<size_t> disc_dist(weights.begin(), weights.end());
    
    // Sample and return
    size_t index = disc_dist(generator_);
    return elements[index];
}

template <typename T>
T Random::sample(const std::vector<T>& elements, const std::vector<double>& weights) {
    if (elements.size() != weights.size() || elements.empty()) {
        throw std::invalid_argument("Elements and weights must have same non-zero size");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create discrete distribution
    std::discrete_distribution<size_t> disc_dist(weights.begin(), weights.end());
    
    // Sample and return
    size_t index = disc_dist(generator_);
    return elements[index];
}

template <typename T>
T Random::sampleUniform(const std::vector<T>& elements) {
    if (elements.empty()) {
        throw std::invalid_argument("Cannot sample from empty vector");
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Uniform distribution over indices
    std::uniform_int_distribution<size_t> uniform_dist(0, elements.size() - 1);
    
    // Sample and return
    size_t index = uniform_dist(generator_);
    return elements[index];
}

} // namespace poker