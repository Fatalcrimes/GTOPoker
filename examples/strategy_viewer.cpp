// examples/strategy_viewer.cpp
#include <iostream>
#include <string>
#include <iomanip>
#include "cfr/StrategyTable.hpp"
#include "game/Action.hpp"
#include "utils/Logger.hpp"

using namespace poker;

int main(int argc, char* argv[]) {
    // Initialize logger
    Logger::getInstance().init(Logger::Level::INFO, Logger::Destination::CONSOLE);
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <strategy_file> [--filter <info_set_pattern>]\n";
        return 1;
    }
    
    std::string filename = argv[1];
    std::string filter = "";
    
    // Parse additional arguments
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--filter" && i + 1 < argc) {
            filter = argv[++i];
        }
    }
    
    // Load the strategy
    StrategyTable strategyTable;
    if (!strategyTable.loadFromFile(filename)) {
        std::cerr << "Failed to load strategy from " << filename << std::endl;
        return 1;
    }
    
    // Get all info sets
    auto infoSets = strategyTable.getAllInfoSets();
    std::cout << "Total info sets: " << infoSets.size() << std::endl;
    
    // Print statistics
    int preflopSets = 0, flopSets = 0, turnSets = 0, riverSets = 0;
    for (const auto& infoSet : infoSets) {
        if (infoSet.find("PREFLOP") != std::string::npos) preflopSets++;
        else if (infoSet.find("FLOP") != std::string::npos) flopSets++;
        else if (infoSet.find("TURN") != std::string::npos) turnSets++;
        else if (infoSet.find("RIVER") != std::string::npos) riverSets++;
    }
    
    std::cout << "Preflop info sets: " << preflopSets << std::endl;
    std::cout << "Flop info sets: " << flopSets << std::endl;
    std::cout << "Turn info sets: " << turnSets << std::endl;
    std::cout << "River info sets: " << riverSets << std::endl;
    
    // Print each info set and its strategies (applying filter if provided)
    int displayCount = 0;
    const int MAX_DISPLAY = 50; // Limit the number of displayed info sets
    
    std::cout << "\nInfo Sets:" << std::endl;
    std::cout << "------------------------------------------------\n";
    
    for (const auto& infoSet : infoSets) {
        // Apply filter if provided
        if (!filter.empty() && infoSet.find(filter) == std::string::npos) {
            continue;
        }
        
        if (displayCount++ >= MAX_DISPLAY) {
            std::cout << "... (more info sets available, use --filter to narrow results)\n";
            break;
        }
        
        std::cout << "Info set: " << infoSet << std::endl;
        
        auto strategies = strategyTable.getAverageStrategies(infoSet);
        std::cout << "  Strategies:" << std::endl;
        
        for (const auto& [action, prob] : strategies) {
            std::cout << "    " << std::left << std::setw(15) << action.toString() 
                      << ": " << std::fixed << std::setprecision(6) << prob << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    return 0;
}