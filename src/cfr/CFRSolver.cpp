#include "cfr/CFRSolver.hpp"
#include "utils/Logger.hpp"
#include "utils/Random.hpp"
#include "abstraction/BetAbstraction.hpp"
#include <chrono>
#include <iostream>
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
    betAbstraction_(betAbstraction)
{
    // If no abstractions provided, create default ones
    if (!handAbstraction_) {
        handAbstraction_ = HandAbstraction::create(HandAbstraction::Level::STANDARD);
    }
    
    if (!betAbstraction_) {
        betAbstraction_ = BetAbstraction::create(BetAbstraction::Level::STANDARD);
    }
    
    // Initialize the default progress callback
    progressCallback_ = [](int /*iteration*/, const TrainingStats&) {
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
    
    // OPTIMIZATION: Initialize game state once and reuse it
    auto gameState = initialState_->clone();
    
    // Run the specified number of iterations
    for (int i = 0; i < iterations; ++i) {
        auto iterationStart = std::chrono::high_resolution_clock::now();
        
        // Reset game state instead of creating new one
        gameState->reset();
        gameState->dealHoleCards();
        
        // Initialize reach probabilities
        std::unordered_map<Position, double> reachProbabilities;
        reachProbabilities[Position::SB] = 1.0;
        reachProbabilities[Position::BB] = 1.0;
        reachProbabilities[Position::BTN] = 1.0;
        
        // Run CFR directly instead of through runIteration
        if (useMonteCarloSampling) {
            monteCarloSample(*gameState, reachProbabilities, 0);
        } else {
            cfr(*gameState, reachProbabilities, 0);
        }
        
        auto iterationEnd = std::chrono::high_resolution_clock::now();
        auto iterationTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            iterationEnd - iterationStart).count();
        
        // Update counters with mutex protection
        {
            std::lock_guard<std::mutex> lock(statsMutex_);
            iterationsCompleted_++;
            totalTrainingTime_ += static_cast<double>(iterationTime);
        }
        
        // Report progress
        if ((i + 1) % 10 == 0 || i == iterations - 1) {  // Changed from 100 to 10 for more frequent updates
            LOG_INFO("Completed iteration " + std::to_string(i + 1) + "/" + 
                    std::to_string(iterations) + " (" + std::to_string(iterationTime) + "ms)");
            
            // Calculate and report training stats
            if (progressCallback_) {
                progressCallback_(i + 1, getTrainingStats());
            }
        }
        
        // OPTIMIZATION: Perform memory cleanup periodically
        if ((i + 1) % 20 == 0) {
            LOG_INFO("Performing memory cleanup...");
            size_t beforeSize = regretTable_.size();
            pruneStrategiesAndRegrets();
            size_t afterSize = regretTable_.size();
            LOG_INFO("Memory cleanup: removed " + std::to_string(beforeSize - afterSize) + 
                     " low-value info sets");
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - startTime).count();
    
    LOG_INFO("Training completed in " + std::to_string(totalTime) + "ms");
    LOG_INFO("Processed information sets: " + std::to_string(regretTable_.size()));
}

// You'll need to add this helper method to the CFRSolver class:
void CFRSolver::pruneStrategiesAndRegrets() {
    // Remove info sets with very small regrets to save memory
    const double REGRET_THRESHOLD = 0.01;
    regretTable_.prune(REGRET_THRESHOLD);
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
        monteCarloSample(*gameState, reachProbabilities, 0);
    } else {
        cfr(*gameState, reachProbabilities, 0);
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

// FIXED: Added const qualifier to match header
std::unordered_map<Action, double, RegretTable::ActionHash> 
CFRSolver::getAverageStrategy(const std::string& infoSet) const {
    auto strategies = strategyTable_.getAverageStrategies(infoSet);
    
    // Convert StrategyTable::ActionHash to RegretTable::ActionHash
    std::unordered_map<Action, double, RegretTable::ActionHash> result;
    for (const auto& [action, prob] : strategies) {
        result[action] = prob;
    }
    
    return result;
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
    
    // Thread safety for reading statistics
    {
        std::lock_guard<std::mutex> lock(statsMutex_);
        stats.iterations = iterationsCompleted_;
        
        if (iterationsCompleted_ > 0) {
            stats.avgTimePerIteration = totalTrainingTime_ / iterationsCompleted_;
        } else {
            stats.avgTimePerIteration = 0.0;
        }
    }
    
    stats.infoSetCount = regretTable_.size();
    
    // TODO: Calculate exploitability
    // This is computationally expensive and typically done offline
    stats.exploitability = 0.0;
    
    return stats;
}

void CFRSolver::setProgressCallback(ProgressCallback callback) {
    progressCallback_ = std::move(callback);
}

std::unordered_map<Position, double> 
CFRSolver::cfr(GameState& state, std::unordered_map<Position, double>& reachProbabilities, int depth) {
    const int MAX_RECURSION_DEPTH = 100;

    // Check recursion depth
    if (depth > MAX_RECURSION_DEPTH) {
        LOG_WARNING("Maximum recursion depth exceeded in CFR");
        std::unordered_map<Position, double> emergency_payoffs;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            emergency_payoffs[static_cast<Position>(i)] = 0.0;
        }
        return emergency_payoffs;
    }
    
    // Terminal state check (fast path)
    if (state.isTerminal()) {
        return state.getPayoffs();
    }
    
    // Get current player and info set
    Position currentPosition = state.getCurrentPosition();
    std::string infoSet = getAbstractedInfoSet(state, currentPosition);
    
    // Get valid actions with abstraction
    std::vector<Action> validActions = getAbstractedActions(state);
    if (validActions.empty()) {
        LOG_ERROR("No valid actions for non-terminal state");
        std::unordered_map<Position, double> emergency_payoffs;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            emergency_payoffs[static_cast<Position>(i)] = 0.0;
        }
        return emergency_payoffs;
    }
    
    // Get strategy using positive regrets only
    auto strategy = getStrategy(infoSet, validActions);
    
    // OPTIMIZATION: Only update strategy sum if reach probability is significant
    double reachProb = reachProbabilities[currentPosition];
    if (reachProb > 0.00001) {
        for (const auto& [action, prob] : strategy) {
            if (prob > 0.0) {
                strategyTable_.addToStrategySum(infoSet, action, reachProb * prob);
            }
        }
    }
    
    // Initialize expected utilities
    std::unordered_map<Position, double> expectedUtilities;
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        expectedUtilities[static_cast<Position>(i)] = 0.0;
    }
    
    // OPTIMIZATION: Store action utilities in a vector instead of map
    std::vector<std::unordered_map<Position, double>> actionUtilities(validActions.size());
    
    // OPTIMIZATION: Use preallocated nextReachProbs to avoid repeated allocation
    auto nextReachProbs = reachProbabilities;
    
    // Recurse for each action
    for (size_t i = 0; i < validActions.size(); ++i) {
        const Action& action = validActions[i];
        
        // OPTIMIZATION: Update reach probabilities without creating new map
        double oldReachProb = nextReachProbs[currentPosition];
        nextReachProbs[currentPosition] = reachProb * strategy[action];
        
        // Create a copy of the game state
        auto nextState = state.clone();
        
        // Apply action and handle round transition
        bool roundOver = false;
        try {
            roundOver = nextState->applyAction(action);
        } catch (const std::exception& e) {
            LOG_ERROR("Error applying action: " + std::string(e.what()));
            continue;
        }
        
        if (roundOver && !nextState->isTerminal()) {
            nextState->startNextBettingRound();
        }
        
        // Recursive call
        actionUtilities[i] = cfr(*nextState, nextReachProbs, depth + 1);
        
        // Update expected utilities
        for (const auto& [pos, util] : actionUtilities[i]) {
            expectedUtilities[pos] += strategy[action] * util;
        }
        
        // OPTIMIZATION: Restore original reach probability
        nextReachProbs[currentPosition] = oldReachProb;
    }
    
    // OPTIMIZATION: Only compute regrets if reach probability is significant
    if (reachProb > 0.00001) {
        // Calculate counterfactual probability
        double counterFactProb = 1.0;
        for (const auto& [pos, prob] : reachProbabilities) {
            if (pos != currentPosition) {
                counterFactProb *= prob;
            }
        }
        
        // Calculate regrets
        for (size_t i = 0; i < validActions.size(); ++i) {
            const Action& action = validActions[i];
            
            // OPTIMIZATION: Only process if we have utilities for this action
            if (actionUtilities[i].empty()) continue;
            
            // Calculate regret
            double actionUtil = actionUtilities[i][currentPosition];
            double regret = counterFactProb * (actionUtil - expectedUtilities[currentPosition]);
            
            // CFR+ uses positive regrets only
            if (regret > 0.0) {
                regretTable_.addRegret(infoSet, action, regret);
            }
        }
    }
    
    return expectedUtilities;
}

std::unordered_map<Position, double> 
CFRSolver::monteCarloSample(GameState& state, std::unordered_map<Position, double>& reachProbabilities, int depth = 0) {

    if (depth > MAX_RECURSION_DEPTH) {
        LOG_ERROR("Maximum recursion depth exceeded in monteCarloSample");
        std::unordered_map<Position, double> emergency_payoffs;
        for (int i = 0; i < NUM_PLAYERS; ++i) {
            emergency_payoffs[static_cast<Position>(i)] = 0.0;
        }
        return emergency_payoffs;
    }
    
    // Add debug logging
    LOG_DEBUG("monteCarloSample depth=" + std::to_string(depth) + 
              " round=" + bettingRoundToString(state.getBettingRound()) + 
              " position=" + positionToString(state.getCurrentPosition()));

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
    
    // Make sure the action is in our abstracted action set
    if (sampledAction.getType() == ActionType::BET || sampledAction.getType() == ActionType::RAISE) {
        // Find the closest valid action
        Action closestAction = sampledAction;
        double minDiff = std::numeric_limits<double>::max();
        
        for (const auto& validAction : validActions) {
            if (validAction.getType() == sampledAction.getType()) {
                double diff = std::abs(validAction.getAmount() - sampledAction.getAmount());
                if (diff < minDiff) {
                    minDiff = diff;
                    closestAction = validAction;
                }
            }
        }
        
        // If we found a closer valid action, use it
        if (minDiff < std::numeric_limits<double>::max()) {
            LOG_DEBUG("Adjusting action from " + sampledAction.toString() + " to " + closestAction.toString());
            sampledAction = closestAction;
        }
    }
    
    // Create a copy of the game state
    auto nextState = state.clone();
    
    // Apply the sampled action
    bool roundOver = false;
    try {
        roundOver = nextState->applyAction(sampledAction);
    } catch (const std::invalid_argument& e) {
        // If action is invalid, log the error and try to recover
        LOG_ERROR("Invalid action in monteCarloSample: " + std::string(e.what()));
        
        // Try to choose a valid action as fallback
        if (!validActions.empty()) {
            sampledAction = validActions[0]; // Use first valid action as fallback
            LOG_INFO("Falling back to action: " + sampledAction.toString());
            roundOver = nextState->applyAction(sampledAction);
        } else {
            // Something is seriously wrong if we have no valid actions
            throw std::runtime_error("No valid actions available in non-terminal state");
        }
    }
    
    // If round is over but game is not terminal, start next round
    if (roundOver && !nextState->isTerminal()) {
        nextState->startNextBettingRound();
    }
    
    // Update reach probabilities for recursion
    auto nextReachProbs = reachProbabilities;
    nextReachProbs[currentPosition] *= strategy[sampledAction];
    
    // Recursively calculate utilities
    auto sampledUtil = monteCarloSample(*nextState, nextReachProbs, depth + 1);
    
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
    oss << positionToString(position) << "|" 
        << bettingRoundToString(state.getBettingRound()) << "|" 
        << handBucket << "|"
        << state.getActionHistory().toString();
    
    return oss.str();
}

// In src/cfr/CFRSolver.cpp

void CFRSolver::extractRFIRanges(const std::string& btnOutputFile, 
                                const std::string& sbOutputFile) const {
    LOG_INFO("Extracting RFI ranges...");
    
    // Create mappings for hand buckets to raise frequencies
    std::unordered_map<int, double> btnBucketRaiseFreq;
    std::unordered_map<int, double> sbBucketRaiseFreq;
    
    // Get all info sets
    auto infoSets = strategyTable_.getAllInfoSets();
    
    // For each info set, check if it's a preflop decision with no prior action
    for (const auto& infoSet : infoSets) {
        std::istringstream iss(infoSet);
        std::string position, round, handBucketStr, actions;
        
        // Parse info set format: "<position>|<round>|<hand_bucket>|<action_history>"
        std::getline(iss, position, '|');
        std::getline(iss, round, '|');
        std::getline(iss, handBucketStr, '|');
        std::getline(iss, actions, '|');
        
        // Only consider preflop info sets without prior actions
        if (round == "PREFLOP" && actions.empty()) {
            int handBucket = std::stoi(handBucketStr);
            
            // Get strategy for this info set
            auto strategies = strategyTable_.getAverageStrategies(infoSet);
            
            // Calculate raise frequency (sum of all raise/bet actions)
            double raiseFreq = 0.0;
            for (const auto& [action, prob] : strategies) {
                if (action.getType() == ActionType::BET || action.getType() == ActionType::RAISE) {
                    raiseFreq += prob;
                }
            }
            
            // Store by position
            if (position == "BTN") {
                btnBucketRaiseFreq[handBucket] = raiseFreq;
            } else if (position == "SB") {
                sbBucketRaiseFreq[handBucket] = raiseFreq;
            }
        }
    }
    
    // Open output files
    std::ofstream btnFile(btnOutputFile);
    std::ofstream sbFile(sbOutputFile);
    
    if (!btnFile.is_open() || !sbFile.is_open()) {
        LOG_ERROR("Failed to open output files for RFI ranges");
        return;
    }
    
    // Get abstraction level for debugging info
    int numBuckets = handAbstraction_->getNumBuckets(BettingRound::PREFLOP);
    
    // Write headers
    btnFile << "# Button RFI Range - Using " << numBuckets << " hand buckets" << std::endl;
    sbFile << "# Small Blind RFI Range - Using " << numBuckets << " hand buckets" << std::endl;
    
    // Output each bucket's raise frequency
    for (int bucket = 0; bucket < numBuckets; bucket++) {
        double btnFreq = btnBucketRaiseFreq.count(bucket) ? btnBucketRaiseFreq[bucket] : 0.0;
        double sbFreq = sbBucketRaiseFreq.count(bucket) ? sbBucketRaiseFreq[bucket] : 0.0;
        
        btnFile << "Bucket " << bucket << ": " << (btnFreq * 100.0) << "%" << std::endl;
        sbFile << "Bucket " << bucket << ": " << (sbFreq * 100.0) << "%" << std::endl;
    }
    
    btnFile.close();
    sbFile.close();
    
    LOG_INFO("RFI ranges extracted to " + btnOutputFile + " and " + sbOutputFile);
}

} // namespace poker