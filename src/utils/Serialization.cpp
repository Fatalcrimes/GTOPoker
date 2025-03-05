#include "utils/Serialization.hpp"
#include "utils/Logger.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/stat.h>

namespace poker {

// Serialization of Card
std::string Serialization::serializeCard(const Card& card) {
    std::ostringstream oss;
    oss << static_cast<int>(card.rank) << ","
        << static_cast<int>(card.suit);
    return oss.str();
}

Card Serialization::deserializeCard(const std::string& str) {
    std::istringstream iss(str);
    std::string rankStr, suitStr;
    
    std::getline(iss, rankStr, ',');
    std::getline(iss, suitStr, ',');
    
    int rank = std::stoi(rankStr);
    int suit = std::stoi(suitStr);
    
    return Card(static_cast<Rank>(rank), static_cast<Suit>(suit));
}

// Serialization of Action
std::string Serialization::serializeAction(const Action& action) {
    std::ostringstream oss;
    oss << static_cast<int>(action.getType()) << ","
        << action.getAmount();
    return oss.str();
}

Action Serialization::deserializeAction(const std::string& str) {
    std::istringstream iss(str);
    std::string typeStr, amountStr;
    
    std::getline(iss, typeStr, ',');
    std::getline(iss, amountStr, ',');
    
    int type = std::stoi(typeStr);
    double amount = std::stod(amountStr);
    
    return Action(static_cast<ActionType>(type), amount);
}

// Serialization of Position
std::string Serialization::serializePosition(Position position) {
    return std::to_string(static_cast<int>(position));
}

Position Serialization::deserializePosition(const std::string& str) {
    int pos = std::stoi(str);
    return static_cast<Position>(pos);
}

// Serialization of BettingRound
std::string Serialization::serializeBettingRound(BettingRound round) {
    return std::to_string(static_cast<int>(round));
}

BettingRound Serialization::deserializeBettingRound(const std::string& str) {
    int round = std::stoi(str);
    return static_cast<BettingRound>(round);
}

// Serialization of HandStrength
std::string Serialization::serializeHandStrength(const HandStrength& strength) {
    std::ostringstream oss;
    oss << static_cast<int>(strength.handRank) << ","
        << strength.primaryValue << ","
        << strength.secondaryValue;
    
    for (const auto& kicker : strength.kickers) {
        oss << "," << kicker;
    }
    
    return oss.str();
}

HandStrength Serialization::deserializeHandStrength(const std::string& str) {
    std::istringstream iss(str);
    std::string rankStr, primaryStr, secondaryStr;
    
    std::getline(iss, rankStr, ',');
    std::getline(iss, primaryStr, ',');
    std::getline(iss, secondaryStr, ',');
    
    int rank = std::stoi(rankStr);
    uint32_t primary = std::stoul(primaryStr);
    uint32_t secondary = std::stoul(secondaryStr);
    
    std::array<uint32_t, 5> kickers = {0};
    
    for (int i = 0; i < 5 && !iss.eof(); ++i) {
        std::string kickerStr;
        std::getline(iss, kickerStr, ',');
        if (!kickerStr.empty()) {
            kickers[i] = std::stoul(kickerStr);
        }
    }
    
    return HandStrength{
        static_cast<HandRank>(rank),
        primary,
        secondary,
        kickers
    };
}

// Binary serialization helpers
void Serialization::writeBinary(std::ofstream& ofs, const std::string& str) {
    // Write string length
    int length = static_cast<int>(str.length());
    ofs.write(reinterpret_cast<const char*>(&length), sizeof(length));
    
    // Write string data
    ofs.write(str.c_str(), length);
}

void Serialization::writeBinary(std::ofstream& ofs, int value) {
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void Serialization::writeBinary(std::ofstream& ofs, double value) {
    ofs.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

std::string Serialization::readBinaryString(std::ifstream& ifs) {
    // Read string length
    int length;
    ifs.read(reinterpret_cast<char*>(&length), sizeof(length));
    
    // Read string data
    std::string str(length, ' ');
    ifs.read(&str[0], length);
    
    return str;
}

int Serialization::readBinaryInt(std::ifstream& ifs) {
    int value;
    ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

double Serialization::readBinaryDouble(std::ifstream& ifs) {
    double value;
    ifs.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

// File utilities
bool Serialization::fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

bool Serialization::createDirectory(const std::string& path) {
    try {
        return std::filesystem::create_directories(path);
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("Failed to create directory " + path + ": " + e.what());
        return false;
    }
}

} // namespace poker