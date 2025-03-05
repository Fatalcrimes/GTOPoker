#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <atomic>

#include "game/GameState.hpp"
#include "cfr/RegretTable.hpp"
#include "cfr/StrategyTable.hpp"
#include "abstraction/HandAbstraction.hpp"
#include "abstraction/BetAbstraction.hpp"

namespace poker {

/**
 * CFRSolver implements the Counterfactual Regret Minimization algorithm
 * for finding approximate Nash equilibrium strategies in imperfect information games.
 */
class CFRSolver {
public:
    // Constructor
    CFRSolver(
        std::unique_ptr<GameState> initialState,
        std::shared_ptr<HandAbstraction> handAbstraction = nullptr,
        std::shared_ptr<BetAbstraction> betAbstraction = nullptr
    );
    
    // Destructor
    ~CFRSolver();
    
    // Run CFR for specified number of iterations
    void train(int iterations, bool useMonteCarloSampling = false);
    
    // Run single CFR iteration
    void runIteration(bool useMonteCarloSampling = false);
    
    // Get current strategy
    std::unordered_map<Action, double, RegretTable::ActionHash> 
    getStrategy(const std::string& infoSet, const std::vector<Action>& validActions);
    
    // Get average strategy - FIXED: Added const qualifier
    std::unordered_map<Action, double, RegretTable::ActionHash> 
    getAverageStrategy(const std::string& infoSet) const;
    
    // Save learned strategy
    bool saveStrategy(const std::string& filename) const;
    
    // Load strategy
    bool loadStrategy(const std::string& filename);
    
    // Get training statistics
    struct TrainingStats {
        int iterations;
        double exploitability;
        size_t infoSetCount;
        double avgTimePerIteration;
    };
    TrainingStats getTrainingStats() const;
    
    // Progress callback
    using ProgressCallback = std::function<void(int iteration, const TrainingStats&)>;
    void setProgressCallback(ProgressCallback callback);

private:
    // CFR+ implementation with regret matching and averaging
    std::unordered_map<Position, double> 
    cfr(GameState& state, std::unordered_map<Position, double>& reachProbabilities);
    
    // Monte Carlo CFR implementation for faster convergence
    std::unordered_map<Position, double> 
    monteCarloSample(GameState& state, std::unordered_map<Position, double>& reachProbabilities);
    
    // Get abstracted information set
    std::string getAbstractedInfoSet(const GameState& state, Position position) const;
    
    // Get abstracted actions
    std::vector<Action> getAbstractedActions(const GameState& state) const;
    
    // Update strategy based on current regrets
    void updateStrategy(const std::string& infoSet, const std::vector<Action>& validActions);
    
    // Data members
    std::unique_ptr<GameState> initialState_;
    std::shared_ptr<HandAbstraction> handAbstraction_;
    std::shared_ptr<BetAbstraction> betAbstraction_;
    
    RegretTable regretTable_;
    StrategyTable strategyTable_;
    
    // Training statistics - no need for atomic since we protect with mutex
    int iterationsCompleted_{0};
    double totalTrainingTime_{0.0};
    ProgressCallback progressCallback_;
    mutable std::mutex statsMutex_;  // Add mutex for thread safety
};

} // namespace poker