#include "game/PokerDefs.hpp"
#include <sstream>
#include <array>

namespace poker {

std::string Card::toString() const {
    std::string rankStr;
    switch (rank) {
        case Rank::TWO:   rankStr = "2"; break;
        case Rank::THREE: rankStr = "3"; break;
        case Rank::FOUR:  rankStr = "4"; break;
        case Rank::FIVE:  rankStr = "5"; break;
        case Rank::SIX:   rankStr = "6"; break;
        case Rank::SEVEN: rankStr = "7"; break;
        case Rank::EIGHT: rankStr = "8"; break;
        case Rank::NINE:  rankStr = "9"; break;
        case Rank::TEN:   rankStr = "T"; break;
        case Rank::JACK:  rankStr = "J"; break;
        case Rank::QUEEN: rankStr = "Q"; break;
        case Rank::KING:  rankStr = "K"; break;
        case Rank::ACE:   rankStr = "A"; break;
        default:          rankStr = "?"; break;
    }
    
    std::string suitStr;
    switch (suit) {
        case Suit::SPADE:   suitStr = "s"; break;
        case Suit::HEART:   suitStr = "h"; break;
        case Suit::DIAMOND: suitStr = "d"; break;
        case Suit::CLUB:    suitStr = "c"; break;
        default:            suitStr = "?"; break;
    }
    
    return rankStr + suitStr;
}

bool HandStrength::operator<(const HandStrength& other) const {
    // Compare hand ranks first
    if (handRank != other.handRank) {
        return static_cast<int>(handRank) < static_cast<int>(other.handRank);
    }
    
    // Compare primary value
    if (primaryValue != other.primaryValue) {
        return primaryValue < other.primaryValue;
    }
    
    // Compare secondary value
    if (secondaryValue != other.secondaryValue) {
        return secondaryValue < other.secondaryValue;
    }
    
    // Compare kickers
    for (size_t i = 0; i < kickers.size(); ++i) {
        if (kickers[i] != other.kickers[i]) {
            return kickers[i] < other.kickers[i];
        }
    }
    
    // Equal hand strengths
    return false;
}

bool HandStrength::operator>(const HandStrength& other) const {
    return other < *this;
}

bool HandStrength::operator==(const HandStrength& other) const {
    return handRank == other.handRank &&
           primaryValue == other.primaryValue &&
           secondaryValue == other.secondaryValue &&
           kickers == other.kickers;
}

Position nextPosition(Position pos) {
    switch (pos) {
        case Position::SB:  return Position::BB;
        case Position::BB:  return Position::BTN;
        case Position::BTN: return Position::SB;
        default:            return Position::SB; // Shouldn't happen
    }
}

BettingRound nextBettingRound(BettingRound round) {
    switch (round) {
        case BettingRound::PREFLOP: return BettingRound::FLOP;
        case BettingRound::FLOP:    return BettingRound::TURN;
        case BettingRound::TURN:    return BettingRound::RIVER;
        case BettingRound::RIVER:   return BettingRound::RIVER; // No next round
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

std::string handRankToString(HandRank rank) {
    switch (rank) {
        case HandRank::HIGH_CARD:     return "HIGH_CARD";
        case HandRank::PAIR:          return "PAIR";
        case HandRank::TWO_PAIR:      return "TWO_PAIR";
        case HandRank::THREE_OF_A_KIND: return "THREE_OF_A_KIND";
        case HandRank::STRAIGHT:      return "STRAIGHT";
        case HandRank::FLUSH:         return "FLUSH";
        case HandRank::FULL_HOUSE:    return "FULL_HOUSE";
        case HandRank::FOUR_OF_A_KIND: return "FOUR_OF_A_KIND";
        case HandRank::STRAIGHT_FLUSH: return "STRAIGHT_FLUSH";
        case HandRank::ROYAL_FLUSH:   return "ROYAL_FLUSH";
        default:                      return "UNKNOWN";
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

std::ostream& operator<<(std::ostream& os, HandRank rank) {
    os << handRankToString(rank);
    return os;
}

std::ostream& operator<<(std::ostream& os, const Card& card) {
    os << card.toString();
    return os;
}

} // namespace poker