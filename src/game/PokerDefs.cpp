#include "game/PokerDefs.hpp"
#include <sstream>
#include <array>

namespace poker {

Position nextPosition(Position pos) {
    switch (pos) {
        case Position::BTN: return Position::SB;
        case Position::SB:  return Position::BB;
        case Position::BB:  return Position::BTN;
        default:            return Position::SB; // Shouldn't happen
    }
}

BettingRound nextBettingRound(BettingRound round) {
    switch (round) {
        case BettingRound::PREFLOP: return BettingRound::FLOP;
        case BettingRound::FLOP:    return BettingRound::TURN;
        case BettingRound::TURN:    return BettingRound::RIVER;
        case BettingRound::RIVER:   return BettingRound::SHOWDOWN; // No next round
        default:                     return BettingRound::PREFLOP; // Shouldn't happen
    }
}

std::string positionToString(Position pos) {
    switch (pos) {
        case Position::SB:  return "SB";
        case Position::BB:  return "BB";
        case Position::BTN: return "BTN";
        default:            return "UNKNOWN";
    }
}

std::string bettingRoundToString(BettingRound round) {
    switch (round) {
        case BettingRound::PREFLOP: return "PREFLOP";
        case BettingRound::FLOP:    return "FLOP";
        case BettingRound::TURN:    return "TURN";
        case BettingRound::RIVER:   return "RIVER";
        default:                     return "UNKNOWN";
    }
}
// Stream operators for enum types
std::ostream& operator<<(std::ostream& os, Position pos) {
    os << positionToString(pos);
    return os;
}

std::ostream& operator<<(std::ostream& os, BettingRound round) {
    os << bettingRoundToString(round);
    return os;
}


} // namespace poker