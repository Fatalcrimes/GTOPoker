#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace poker {

// Game parameters
constexpr double BIG_BLIND = 1.0;
constexpr double SMALL_BLIND = 0.5;
constexpr double STARTING_STACK = 25.0; // 25 BB

// Card definitions
enum class Suit : uint8_t {
    SPADE,
    HEART,
    DIAMOND,
    CLUB
};

enum class Rank : uint8_t {
    TWO = 2,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE
};

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

// Hand rankings
enum class HandRank : uint8_t {
    HIGH_CARD,
    PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
};

// Card representation
struct Card {
    Rank rank;
    Suit suit;
    
    Card() = default;
    Card(Rank r, Suit s) : rank(r), suit(s) {}
    
    bool operator==(const Card& other) const {
        return rank == other.rank && suit == other.suit;
    }
    
    bool operator<(const Card& other) const {
        if (rank != other.rank) {
            return static_cast<uint8_t>(rank) < static_cast<uint8_t>(other.rank);
        }
        return static_cast<uint8_t>(suit) < static_cast<uint8_t>(other.suit);
    }
    
    std::string toString() const;
};

// Define a full deck of cards
constexpr int DECK_SIZE = 52;
constexpr int NUM_RANKS = 13;
constexpr int NUM_SUITS = 4;

// Define number of players
constexpr int NUM_PLAYERS = 3;

// Maximum number of betting rounds
constexpr int MAX_BETTING_ROUNDS = 4;

// Maximum number of community cards
constexpr int MAX_COMMUNITY_CARDS = 5;

// Number of hole cards per player
constexpr int NUM_HOLE_CARDS = 2;

// Hand strength representation
struct HandStrength {
    HandRank handRank;
    uint32_t primaryValue;
    uint32_t secondaryValue;
    std::array<uint32_t, 5> kickers;
    
    bool operator<(const HandStrength& other) const;
    bool operator>(const HandStrength& other) const;
    bool operator==(const HandStrength& other) const;
};

// Helper functions
Position nextPosition(Position pos);
BettingRound nextBettingRound(BettingRound round);
std::string positionToString(Position pos);
std::string bettingRoundToString(BettingRound round);
std::string handRankToString(HandRank rank);

} // namespace poker