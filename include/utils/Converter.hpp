#pragma once

#include "game/PokerDefs.hpp"
#include "pokerstove/peval/Card.h"
#include "pokerstove/peval/CardSet.h"

namespace poker {

/**
 * Utility class for converting between native Card representations
 * and PokerStove library representations.
 */
class Converter {
public:
    // Convert individual poker::Card to pokerstove::Card
    static pokerstove::Card toPokerStoveCard(const Card& card);
    
    // Convert an array of poker::Card to pokerstove::CardSet
    static pokerstove::CardSet toPokerStoveCardSet(const std::array<Card, NUM_HOLE_CARDS>& cards);
    
    // Convert a vector of poker::Card to pokerstove::CardSet
    static pokerstove::CardSet toPokerStoveCardSet(const std::vector<Card>& cards);
    
    // Convert pokerstove::Card back to poker::Card
    static Card fromPokerStoveCard(const pokerstove::Card& psCard);
    
    // Convert pokerstove::CardSet to vector of poker::Card
    static std::vector<Card> fromPokerStoveCardSet(const pokerstove::CardSet& psCardSet);
};

} // namespace poker