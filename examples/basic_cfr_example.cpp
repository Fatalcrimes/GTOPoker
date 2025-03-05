#include <iostream>
#include <memory>
#include <string>
#include <chrono>

#include "game/GameState.hpp"
#include "cfr/CFRSolver.hpp"
#include "abstraction/HandAbstraction.hpp"
#include "abstraction/BetAbstraction.hpp"
#include "utils/Logger.hpp"

using namespace poker;

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger::getInstance().init(Logger::Level::INFO, Logger::Destination::BOTH, "cfr_example.log");
    
    LOG_INFO("Starting 3-player CFRM-based Poker Bot example");
    
    // Parse command line arguments
    int iterations = 1000;
    std::string saveFile = "strategy.bin";
    bool useMonteCarloSampling = false;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--iterations" && i + 1 < argc) {
            iterations = std::stoi(argv[++i]);
        } else if (arg == "--save" && i + 1 < argc) {
            saveFile = argv[++i];
        } else if (arg == "--monte-carlo") {
            useMonteCarloSampling = true;
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [options]\n"
                      << "Options:\n"
                      << "  --iterations N    Number of CFR iterations (default: 1000)\n"
                      << "  --save FILE       Save strategy to file (default: strategy.bin)\n"
                      << "  --monte-carlo     Use Monte Carlo sampling for faster convergence\n"
                      << "  --help            Show this help message\n";
            return 0;
        }
    }
    
    LOG_INFO("Configuration:");
    LOG_INFO("  Iterations: " + std::to_string(iterations));
    LOG_INFO("  Save file: " + saveFile);
    LOG_INFO("  Monte Carlo sampling: " + std::string(useMonteCarloSampling ? "enabled" : "disabled"));
    
    try {
        // Create abstraction objects
        auto handAbstraction = HandAbstraction::create(HandAbstraction::Level::STANDARD);
        auto betAbstraction = BetAbstraction::create(BetAbstraction::Level::STANDARD);
        
        LOG_INFO("Precomputing hand abstractions...");
        handAbstraction->precompute();
        
        // Create initial game state
        auto initialState = std::make_unique<GameState>();
        
        // Create the CFR solver
        LOG_INFO("Initializing CFR solver...");
        CFRSolver solver(std::move(initialState), handAbstraction, betAbstraction);
        
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