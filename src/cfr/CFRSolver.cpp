#include "cfr/CFRSolver.hpp"
#include "utils/Logger.hpp"
#include "utils/Random.hpp"
#include <chrono>
#include <thread>
#include <numeric>
#include <algorithm>
#include <functional>
#include <iomanip>

namespace poker {

CFRSolver::CFRSolver(
    std::unique_ptr<GameState> initialState,
    std::shared_ptr<HandAbstraction> handAbstraction,
    std::shared_ptr<BetAbstraction> betAbstraction
) : initialState_(std::move(initialState)),
    handAbstraction_(handAbstraction),
    betAbstraction_(betAbstraction),
    iterationsCompleted_(0),
    totalTrainingTime_(0) {
    
    // If no abstractions provided, create default ones
    if (!handAbstraction_) {
        handAbstraction_ = HandAbstraction::create(HandAbstraction::Level::STANDARD);
    }
    
    if (!betAbstraction_) {
        betAbstraction_ = BetAbstraction::create(BetAbstraction::Level::STANDARD);
    }
    
    // Initialize the default progress callback
    progressCallback_ = [](int iteration, const TrainingStats&) {
        // Default progress callback does nothing
    };
}

CFRSolver::~CFRSolver() {
    // Nothing to clean up explicitly
}

void CFRSolver::train(int iterations, bool useMonteCarloSampling) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    LOG_INFO("Starting CFRM training for " + std::to_string(iterations) + " iterations");
    LOG_INFO("Hand abstraction: " + handAbstraction_->getName());
    LOG_INFO("Bet abstraction: " + betAbstraction_->getName());
    LOG_INFO("Using Monte Carlo sampling: " + std::string(useMonteCarloSampling ? "Yes" : "No"));
    
    // Run the specified number of iterations
    for (int i = 0; i < iterations; ++i) {
        auto iterationStart = std::chrono::high_resolution_clock::now();
        
        // Run a single iteration
        runIteration(useMonteCarloSampling);
        
        // Update counters
        iterationsCompleted_++;
        
        auto iterationEnd = std::chrono::high_resolution_clock::now();
        auto iterationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            iterationEnd - iterationStart).count();
        
        totalTrainingTime_ += iterationTime;
        
        // Report progress
        if ((i + 1) % 100 == 0 || i == iterations - 1) {
            LOG_INFO("Completed iteration " + std::to_string(i + 1) + "/" + 
                    std::to_string(iterations) + " (" + std::to_string(iterationTime) + "ms)");
            
            // Calculate and report training stats
            if (progressCallback_) {
                progressCallback_(i + 1, getTrainingStats());
            }
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    LOG_INFO("Training completed in " + std::to_string(totalTime) + "ms");
    LOG_INFO("Processed information sets: " + std::to_string(regretTable_.size()));
}

void CFRSolver::runIteration(bool useMonteCarloSampling) {
    // Create a fresh game state for each iteration
    auto gameState = initialState_->clone();
    
    // Deal hole cards
    gameState->dealHoleCards();
    
    // Initialize reach probabilities (1.0 for each player)
    std::unordered_map<Position, double> reachProbabilities;
    reachProbabilities[Position::SB] = 1.0;
    reachProbabilities[Position::BB] = 1.0;
    reachProbabilities[Position::BTN] = 1.0;
    
    // Run CFR recursion
    if (useMonteCarloSampling) {
        monteCarloSample(*gameState, reachProbabilities);
    } else {
        cfr(*gameState, reachProbabilities);
    }
}

std::unordered_map<Action, double, RegretTable::ActionHash> 
CFRSolver::getStrategy(const std::string& infoSet, const std::vector<Action>& validActions) {
    // Get all regrets for this info set
    auto regrets = regretTable_.getRegrets(infoSet);
    
    // Sum positive regrets
    double regretSum = 0.0;
    for (const auto& action : validActions) {
        double actionRegret = regrets[action];
        if (actionRegret > 0.0) {
            regretSum += actionRegret;
        }
    }
    
    // Calculate strategy based on regret-matching
    std::unordered_map<Action, double, RegretTable::ActionHash> strategy;
    
    if (regretSum > 0.0) {
        // Normalize by the sum of positive regrets
        for (const auto& action : validActions) {
            double actionRegret = regrets[action];
            if (actionRegret > 0.0) {
                strategy[action] = actionRegret / regretSum;
            } else {
                strategy[action] = 0.0;
            }
        }
    } else {
        // If all regrets are non-positive, use uniform strategy
        double uniformProb = 1.0 / validActions.size();
        for (const auto& action : validActions) {
            strategy[action] = uniformProb;
        }
    }
    
    return strategy;
}

std::unordered_map<Action, double, RegretTable::ActionHash> 
CFRSolver::getAverageStrategy(const std::string& infoSet) {
    return strategyTable_.getAverageStrategies(infoSet);
}

bool CFRSolver::saveStrategy(const std::string& filename) const {
    LOG_INFO("Saving strategy to: " + filename);
    
    // Save the average strategy
    bool success = strategyTable_.saveToFile(filename);
    
    if (success) {
        LOG_INFO("Strategy saved successfully");
    } else {
        LOG_ERROR("Failed to save strategy");
    }
    
    return success;
}

bool CFRSolver::loadStrategy(const std::string& filename) {
    LOG_INFO("Loading strategy from: " + filename);
    
    // Load the average strategy
    bool success = strategyTable_.loadFromFile(filename);
    
    if (success) {
        LOG_INFO("Strategy loaded successfully");
    } else {
        LOG_ERROR("Failed to load strategy");
    }
    
    return success;
}

CFRSolver::TrainingStats CFRSolver::getTrainingStats() const {
    TrainingStats stats;
    
    stats.iterations = iterationsCompleted_;
    stats.infoSetCount = regretTable_.size();
    
    if (iterationsCompleted_ > 0) {
        stats.avgTimePerIteration = static_cast<double>(totalTrainingTime_) / iterationsCompleted_;
    } else {
        stats.avgTimePerIteration = 0.0;
    }
    
    // TODO: Calculate exploitability
    // This is computationally expensive and typically done offline
    stats.exploitability = 0.0;
    
    return stats;
}

void CFRSolver::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = std::move(callback);
}

std::unordered_map<Position, double> 
CFRSolver::cfr(GameState& state, std::unordered_map<Position, double>& reachProbabilities) {
    // If we're at a terminal state, return the payoffs
    if (state.isTerminal()) {
        return state.getPayoffs();
    }
    
    // Get current player and their info set
    Position currentPosition = state.getCurrentPosition();
    std::string infoSet = getAbstractedInfoSet(state, currentPosition);
    
    // Get valid actions for current player
    std::vector<Action> validActions = getAbstractedActions(state);
    
    // Get current strategy for this info set
    auto strategy = getStrategy(infoSet, validActions);
    
    // Update strategy table
    for (const auto& [action, prob] : strategy) {
        if (prob > 0.0) {
            strategyTable_.setStrategy(infoSet, action, prob);
        }
    }
    
    // Add contribution to average strategy weighted by reach probability
    for (const auto& [action, prob] : strategy) {
        if (prob > 0.0) {
            strategyTable_.addToStrategySum(infoSet, action, reachProbabilities[currentPosition] * prob);
        }
    }
    
    // Initialize utilities for each action
    std::unordered_map<Action, std::unordered_map<Position, double>> actionUtilities;
    
    // Initialize expected utility for the current player
    std::unordered_map<Position, double> expectedUtility;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        expectedUtility[static_cast<Position>(i)] = 0.0;
    }
    
    // Recurse for each action
    for (const auto& action : validActions) {
        // Create a copy of the game state
        auto nextState = state.clone();
        
        // Apply the action
        bool roundOver = nextState->applyAction(action);
        
        // If round is over but game is not terminal, start next round
        if (roundOver && !nextState->isTerminal()) {
            nextState->startNextBettingRound();
        }
        
        // Update reach probabilities for recursion
        auto nextReachProbs = reachProbabilities;
        nextReachProbs[currentPosition] *= strategy[action];
        
        // Recursively calculate utilities
        auto actionUtil = cfr(*nextState, nextReachProbs);
        
        // Store utilities for this action
        actionUtilities[action] = actionUtil;
        
        // Update expected utility
        for (const auto& [pos, util] : actionUtil) {
            expectedUtility[pos] += strategy[action] * util;
        }
    }
    
    // Calculate counterfactual regrets
    for (const auto& action : validActions) {
        // Calculate counterfactual probability
        double counterfactualProb = 1.0;
        for (const auto& [pos, prob] : reachProbabilities) {
            if (pos != currentPosition) {
                counterfactualProb *= prob;
            }
        }
        
        // Calculate regret as counterfactual utility difference
        double regret = counterfactualProb * (actionUtilities[action][currentPosition] - 
                                             expectedUtility[currentPosition]);
        
        // Add to regret table
        regretTable_.addRegret(infoSet, action, regret);
    }
    
    return expectedUtility;
}

std::unordered_map<Position, double> 
CFRSolver::monteCarloSample(GameState& state, std::unordered_map<Position, double>& reachProbabilities) {
    // If we're at a terminal state, return the payoffs
    if (state.isTerminal()) {
        return state.getPayoffs();
    }
    
    // Get current player and their info set
    Position currentPosition = state.getCurrentPosition();
    std::string infoSet = getAbstractedInfoSet(state, currentPosition);
    
    // Get valid actions for current player
    std::vector<Action> validActions = getAbstractedActions(state);
    
    // Get current strategy for this info set
    auto strategy = getStrategy(infoSet, validActions);
    
    // Update strategy table
    for (const auto& [action, prob] : strategy) {
        if (prob > 0.0) {
            strategyTable_.setStrategy(infoSet, action, prob);
        }
    }
    
    // Add contribution to average strategy weighted by reach probability
    for (const auto& [action, prob] : strategy) {
        if (prob > 0.0) {
            strategyTable_.addToStrategySum(infoSet, action, reachProbabilities[currentPosition] * prob);
        }
    }
    
    // For Monte Carlo sampling, we'll sample one action according to the strategy
    // instead of recursing on all actions
    
    // Sample action based on current strategy
    auto& random = Random::getInstance();
    Action sampledAction = random.sample(strategy);
    
    // Create a copy of the game state
    auto nextState = state.clone();
    
    // Apply the sampled action
    bool roundOver = nextState->applyAction(sampledAction);
    
    // If round is over but game is not terminal, start next round
    if (roundOver && !nextState->isTerminal()) {
        nextState->startNextBettingRound();
    }
    
    // Update reach probabilities for recursion
    auto nextReachProbs = reachProbabilities;
    nextReachProbs[currentPosition] *= strategy[sampledAction];
    
    // Recursively calculate utilities
    auto sampledUtil = monteCarloSample(*nextState, nextReachProbs);
    
    // Initialize expected utility
    std::unordered_map<Position, double> expectedUtility = sampledUtil;
    
    // For the current player, calculate and store counterfactual regrets
    // Calculate counterfactual probability
    double counterfactualProb = 1.0;
    for (const auto& [pos, prob] : reachProbabilities) {
        if (pos != currentPosition) {
            counterfactualProb *= prob;
        }
    }
    
    // For MC-CFR, we only calculate regrets for the sampled action
    // We need to scale the regret by 1/probability to get an unbiased estimator
    if (strategy[sampledAction] > 0.0) {
        double scaledCounterfactualProb = counterfactualProb / strategy[sampledAction];
        
        // Store regret for sampled action (no need to calculate for other actions)
        regretTable_.addRegret(infoSet, sampledAction, scaledCounterfactualProb * sampledUtil[currentPosition]);
    }
    
    return expectedUtility;
}

std::string CFRSolver::getAbstractedInfoSet(const GameState& state, Position position) const {
    // Get the raw info set
    std::string rawInfoSet = state.getInfoSet(position);
    
    // If no hand abstraction is used, return the raw info set
    if (!handAbstraction_) {
        return rawInfoSet;
    }
    
    // Otherwise, apply hand abstraction
    const PlayerState& player = state.getPlayerState(position);
    int handBucket = handAbstraction_->getBucket(player.holeCards, state.getCommunityCards());
    
    // Format: <position>|<round>|<hand_bucket>|<action_history>
    std::ostringstream oss;
    oss << position << "|" 
        << state.getBettingRound() << "|" 
        << handBucket << "|"
        << state.getActionHistory().toString();
    
    return oss.str();
}

std::vector<Action> CFRSolver::getAbstractedActions(const GameState& state) const {
    // Get valid actions for the current game state
    std::vector<Action> validActions = state.getValidActions();
    
    // If no bet abstraction is used, return all valid actions
    if (!betAbstraction_) {
        return validActions;
    }
    
    // Otherwise, apply bet abstraction
    Position currentPosition = state.getCurrentPosition();
    const PlayerState& player = state.getPlayerState(currentPosition);
    
    return betAbstraction_->getAbstractedActions(
        validActions,
        state.getPot(),
        player.stack,
        state.getBettingRound()
    );
}

} // namespace poker