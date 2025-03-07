#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <iostream>

namespace poker {

// Game parameters
constexpr double BIG_BLIND = 1.0;
constexpr double SMALL_BLIND = 0.5;
constexpr double STARTING_STACK = 25.0; // 25 BB


// Player positions for 3-player game
enum class Position : uint8_t {
    SB,  // Small Blind
    BB,  // Big Blind
    BTN  // Button
};

// Betting rounds
enum class BettingRound : uint8_t {
    PREFLOP,
    FLOP,
    TURN,
    RIVER
};

// Define number of players
constexpr int NUM_PLAYERS = 3;

// Stream operators for enum types
std::ostream& operator<<(std::ostream& os, Position pos);
std::ostream& operator<<(std::ostream& os, BettingRound round);
} // namespace poker