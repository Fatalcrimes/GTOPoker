#include "utils/Converter.hpp"

namespace poker {

pokerstove::Card Converter::toPokerStoveCard(const Card& card) {
    // Map your rank enum to pokerstove's char representation
    char rankChar;
    switch (card.rank) {
        case Rank::TWO:   rankChar = '2'; break;
        case Rank::THREE: rankChar = '3'; break;
        case Rank::FOUR:  rankChar = '4'; break;
        case Rank::FIVE:  rankChar = '5'; break;
        case Rank::SIX:   rankChar = '6'; break;
        case Rank::SEVEN: rankChar = '7'; break;
        case Rank::EIGHT: rankChar = '8'; break;
        case Rank::NINE:  rankChar = '9'; break;
        case Rank::TEN:   rankChar = 'T'; break;
        case Rank::JACK:  rankChar = 'J'; break;
        case Rank::QUEEN: rankChar = 'Q'; break;
        case Rank::KING:  rankChar = 'K'; break;
        case Rank::ACE:   rankChar = 'A'; break;
        default:          rankChar = '?'; break;
    }
    
    // Map your suit enum to pokerstove's char representation
    char suitChar;
    switch (card.suit) {
        case Suit::SPADE:   suitChar = 's'; break;
        case Suit::HEART:   suitChar = 'h'; break;
        case Suit::DIAMOND: suitChar = 'd'; break;
        case Suit::CLUB:    suitChar = 'c'; break;
        default:            suitChar = '?'; break;
    }
    
    // Create pokerstove card using string constructor
    std::string cardStr;
    cardStr += rankChar;
    cardStr += suitChar;
    return pokerstove::Card(cardStr);
}

pokerstove::CardSet Converter::toPokerStoveCardSet(const std::array<Card, NUM_HOLE_CARDS>& cards) {
    pokerstove::CardSet cardSet;
    for (const auto& card : cards) {
        cardSet.insert(toPokerStoveCard(card));
    }
    return cardSet;
}

pokerstove::CardSet Converter::toPokerStoveCardSet(const std::vector<Card>& cards) {
    pokerstove::CardSet cardSet;
    for (const auto& card : cards) {
        cardSet.insert(toPokerStoveCard(card));
    }
    return cardSet;
}

Card Converter::fromPokerStoveCard(const pokerstove::Card& psCard) {
    // Get the rank and suit
    int psRank = psCard.rank();
    int psSuit = psCard.suit();
    
    // Map pokerstove rank to your Rank enum
    Rank rank;
    switch (psRank) {
        case pokerstove::TWO:   rank = Rank::TWO; break;
        case pokerstove::THREE: rank = Rank::THREE; break;
        case pokerstove::FOUR:  rank = Rank::FOUR; break;
        case pokerstove::FIVE:  rank = Rank::FIVE; break;
        case pokerstove::SIX:   rank = Rank::SIX; break;
        case pokerstove::SEVEN: rank = Rank::SEVEN; break;
        case pokerstove::EIGHT: rank = Rank::EIGHT; break;
        case pokerstove::NINE:  rank = Rank::NINE; break;
        case pokerstove::TEN:   rank = Rank::TEN; break;
        case pokerstove::JACK:  rank = Rank::JACK; break;
        case pokerstove::QUEEN: rank = Rank::QUEEN; break;
        case pokerstove::KING:  rank = Rank::KING; break;
        case pokerstove::ACE:   rank = Rank::ACE; break;
        default:                rank = Rank::TWO; break;
    }
    
    // Map pokerstove suit to your Suit enum
    Suit suit;
    switch (psSuit) {
        case pokerstove::SPADES:   suit = Suit::SPADE; break;
        case pokerstove::HEARTS:   suit = Suit::HEART; break;
        case pokerstove::DIAMONDS: suit = Suit::DIAMOND; break;
        case pokerstove::CLUBS:    suit = Suit::CLUB; break;
        default:                   suit = Suit::SPADE; break;
    }
    
    return Card(rank, suit);
}

std::vector<Card> Converter::fromPokerStoveCardSet(const pokerstove::CardSet& psCardSet) {
    std::vector<Card> result;
    
    // Convert all cards in the set
    for (int i = 0; i < 52; i++) {
        if (psCardSet.cardInSet(i)) {
            result.push_back(fromPokerStoveCard(pokerstove::Card(i)));
        }
    }
    
    return result;
}

} // namespace poker