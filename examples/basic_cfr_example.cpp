#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include "game/GameState.hpp"
#include "cfr/CFRSolver.hpp"
#include "abstraction/HandAbstraction.hpp"
#include "abstraction/BetAbstraction.hpp"
#include "utils/Logger.hpp"
#include "utils/Random.hpp"  // Added missing include

using namespace poker;

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger::getInstance().init(Logger::Level::INFO, Logger::Destination::BOTH, "cfr_example.log");
    
    LOG_INFO("Starting 3-player CFRM-based Poker Bot example");
    
    // Parse command line arguments
    int iterations = 50000;
    std::string loadFile = "";
    std::string saveFile = "strategy.dat";
    bool useMonteCarloSampling = true;
    bool runTest = true;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--iterations" && i + 1 < argc) {
            iterations = std::stoi(argv[++i]);
        } else if (arg == "--load" && i + 1 < argc) {
            loadFile = argv[++i];
        } else if (arg == "--save" && i + 1 < argc) {
            saveFile = argv[++i];
        } else if (arg == "--monte-carlo") {
            useMonteCarloSampling = true;
        } else if (arg == "--no-test") {
            runTest = false;
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --iterations N    Number of CFR iterations (default: 1000)\n"
                      << "  --load FILE       Load strategy from file\n"
                      << "  --save FILE       Save strategy to file (default: strategy.dat)\n"
                      << "  --monte-carlo     Use Monte Carlo sampling for faster convergence\n"
                      << "  --no-test         Skip test hand playthrough\n"
                      << "  --help            Show this help message\n";
            return 0;
        }
    }
    
    try {
        // Create abstraction objects
        auto handAbstraction = HandAbstraction::create(HandAbstraction::Level::DETAILED);
        auto betAbstraction = BetAbstraction::create(BetAbstraction::Level::MINIMAL);
        
        // Precompute abstractions
        LOG_INFO("Precomputing hand abstractions...");
        handAbstraction->precompute();
        
        // Create initial game state
        auto initialState = std::make_unique<GameState>();
        
        // Create the CFR solver
        LOG_INFO("Initializing CFR solver...");
        CFRSolver solver(std::move(initialState), handAbstraction, betAbstraction);
        
        // Load strategy if specified
        if (!loadFile.empty()) {
            LOG_INFO("Loading strategy from " + loadFile);
            if (solver.loadStrategy(loadFile)) {
                LOG_INFO("Strategy loaded successfully");
            } else {
                LOG_ERROR("Failed to load strategy");
            }
        }
        
        // Train if iterations > 0
        if (iterations > 0) {
            // Set up a progress callback
            solver.setProgressCallback([](int iteration, const CFRSolver::TrainingStats& stats) {
                std::cout << "Iteration " << iteration 
                        << " complete. Info sets: " << stats.infoSetCount
                        << ", Avg time: " << stats.avgTimePerIteration << " ms"
                        << std::endl;
                
                if (iteration % 100 == 0) {
                    LOG_INFO("Iteration " + std::to_string(iteration) + 
                            " complete. Info sets: " + std::to_string(stats.infoSetCount));
                }
            });
            
            // Train the solver
            LOG_INFO("Starting CFR training for " + std::to_string(iterations) + " iterations...");
            auto startTime = std::chrono::high_resolution_clock::now();
            
            solver.train(iterations, useMonteCarloSampling);
            
            LOG_INFO("Extracting RFI ranges from trained strategy");
solver.extractRFIRanges("data/strategies/btn_rfi_range.txt", "data/strategies/sb_rfi_range.txt");

            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
            
            LOG_INFO("Training completed in " + std::to_string(duration) + " seconds");
            
            // Save the learned strategy
            LOG_INFO("Saving strategy to " + saveFile);
            if (solver.saveStrategy(saveFile)) {
                LOG_INFO("Strategy saved successfully");
            } else {
                LOG_ERROR("Failed to save strategy");
            }
        }
        
        // Run a test hand if requested
        if (runTest) {
            // Create a new game state for testing
            auto testState = std::make_unique<GameState>();
            testState->dealHoleCards();
            
            std::cout << "Test hand: " << testState->toString() << std::endl;
            
            // Get the current position's info set
            Position currentPos = testState->getCurrentPosition();
            std::string infoSet = testState->getInfoSet(currentPos);
            
            // Get strategy for this info set
            std::cout << "Info set: " << infoSet << std::endl;
            
            // Get valid actions
            auto validActions = testState->getValidActions();
            
            std::cout << "Valid actions: " << std::endl;
            for (const auto& action : validActions) {
                std::cout << "  " << action.toString() << std::endl;
            }
            
            // Use getAverageStrategy which is now properly marked const
            auto strategy = solver.getAverageStrategy(infoSet);
            
            std::cout << "Strategy: " << std::endl;
            for (const auto& [action, prob] : strategy) {
                std::cout << "  " << action.toString() << ": " << prob << std::endl;
            }
        }
        
        // Get final training stats
        auto stats = solver.getTrainingStats();
        LOG_INFO("Final statistics:");
        LOG_INFO("  Iterations: " + std::to_string(stats.iterations));
        LOG_INFO("  Info sets: " + std::to_string(stats.infoSetCount));
        LOG_INFO("  Exploitability: " + std::to_string(stats.exploitability));
        LOG_INFO("  Avg time per iteration: " + std::to_string(stats.avgTimePerIteration) + " ms");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Exception: " + std::string(e.what()));
        return 1;
    }
    
    LOG_INFO("Example completed successfully");
    return 0;
}