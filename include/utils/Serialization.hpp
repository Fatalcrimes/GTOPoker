#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <memory>

#include "game/Action.hpp"
#include "game/PokerDefs.hpp"

namespace poker {

/**
 * Serialization utilities for saving and loading game data.
 */
class Serialization {
public:
    // Save strategy data to file
    template <typename T>
    static bool saveToFile(const std::unordered_map<std::string, std::unordered_map<Action, T>>& data, 
                          const std::string& filename);
    
    // Load strategy data from file
    template <typename T>
    static bool loadFromFile(std::unordered_map<std::string, std::unordered_map<Action, T>>& data, 
                            const std::string& filename);
    
    // Serialize/deserialize card
    static std::string serializeCard(const Card& card);
    static Card deserializeCard(const std::string& str);
    
    // Serialize/deserialize action
    static std::string serializeAction(const Action& action);
    static Action deserializeAction(const std::string& str);
    
    // Serialize/deserialize position
    static std::string serializePosition(Position position);
    static Position deserializePosition(const std::string& str);
    
    // Serialize/deserialize betting round
    static std::string serializeBettingRound(BettingRound round);
    static BettingRound deserializeBettingRound(const std::string& str);
    
    // Serialize/deserialize hand strength
    static std::string serializeHandStrength(const HandStrength& strength);
    static HandStrength deserializeHandStrength(const std::string& str);
    
    // Helper methods for binary serialization
    static void writeBinary(std::ofstream& ofs, const std::string& str);
    static void writeBinary(std::ofstream& ofs, int value);
    static void writeBinary(std::ofstream& ofs, double value);
    
    static std::string readBinaryString(std::ifstream& ifs);
    static int readBinaryInt(std::ifstream& ifs);
    static double readBinaryDouble(std::ifstream& ifs);
    
    // Check if file exists
    static bool fileExists(const std::string& filename);
    
    // Create directory if it doesn't exist
    static bool createDirectory(const std::string& path);
};

// Template implementation for saving strategy data
template <typename T>
bool Serialization::saveToFile(const std::unordered_map<std::string, std::unordered_map<Action, T>>& data, 
                              const std::string& filename) {
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) {
        return false;
    }
    
    // Write number of info sets
    int numInfoSets = static_cast<int>(data.size());
    writeBinary(ofs, numInfoSets);
    
    // Write each info set
    for (const auto& infoSetPair : data) {
        const std::string& infoSet = infoSetPair.first;
        const auto& actionMap = infoSetPair.second;
        
        // Write info set string
        writeBinary(ofs, infoSet);
        
        // Write number of actions
        int numActions = static_cast<int>(actionMap.size());
        writeBinary(ofs, numActions);
        
        // Write each action and its value
        for (const auto& actionPair : actionMap) {
            const Action& action = actionPair.first;
            T value = actionPair.second;
            
            // Write action
            std::string actionStr = serializeAction(action);
            writeBinary(ofs, actionStr);
            
            // Write value
            writeBinary(ofs, static_cast<double>(value));
        }
    }
    
    ofs.close();
    return true;
}

// Template implementation for loading strategy data
template <typename T>
bool Serialization::loadFromFile(std::unordered_map<std::string, std::unordered_map<Action, T>>& data, 
                                const std::string& filename) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        return false;
    }
    
    data.clear();
    
    // Read number of info sets
    int numInfoSets = readBinaryInt(ifs);
    
    // Read each info set
    for (int i = 0; i < numInfoSets; ++i) {
        // Read info set string
        std::string infoSet = readBinaryString(ifs);
        
        // Read number of actions
        int numActions = readBinaryInt(ifs);
        
        // Read each action and its value
        std::unordered_map<Action, T> actionMap;
        for (int j = 0; j < numActions; ++j) {
            // Read action
            std::string actionStr = readBinaryString(ifs);
            Action action = deserializeAction(actionStr);
            
            // Read value
            double value = readBinaryDouble(ifs);
            
            // Store in map
            actionMap[action] = static_cast<T>(value);
        }
        
        // Store in data map
        data[infoSet] = std::move(actionMap);
    }
    
    ifs.close();
    return true;
}

} // namespace poker