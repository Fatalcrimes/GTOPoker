#include "game/HandEvaluator.hpp"
#include "utils/Converter.hpp"
#include "utils/Random.hpp"
#include "utils/Logger.hpp"
#include "pokerstove/peval/CardSet.h"
#include "pokerstove/peval/HoldemHandEvaluator.h"
#include <algorithm>

namespace poker {

HandEvaluator::HandEvaluator() {
    // Initialize PokerStove evaluator for Hold'em
    evaluator_ = pokerstove::PokerHandEvaluator::alloc("h");
    if (!evaluator_) {
        LOG_ERROR("Failed to initialize PokerStove evaluator");
        throw std::runtime_error("Failed to initialize PokerStove evaluator");
    }
}

HandStrength HandEvaluator::evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                         const std::vector<Card>& communityCards) const {
    try {
        // Convert cards to PokerStove format
        pokerstove::CardSet holeCardSet = Converter::toPokerStoveCardSet(holeCards);
        pokerstove::CardSet boardCardSet = Converter::toPokerStoveCardSet(communityCards);
        
        // Evaluate the hand
        pokerstove::PokerHandEvaluation eval = evaluator_->evaluate(holeCardSet, boardCardSet);
        
        // Convert to our HandStrength structure
        return convertToHandStrength(eval);
    } catch (const std::exception& e) {
        LOG_ERROR("Error in hand evaluation: " + std::string(e.what()));
        // Return a default HandStrength in case of error
        return HandStrength{HandRank::HIGH_CARD, 0, 0, {0}};
    }
}

std::vector<Card> HandEvaluator::findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                             const std::vector<Card>& communityCards) const {
    try {
        // Convert cards to PokerStove format
        pokerstove::CardSet holeCardSet = Converter::toPokerStoveCardSet(holeCards);
        pokerstove::CardSet boardCardSet = Converter::toPokerStoveCardSet(communityCards);
        
        // Get the best 5-card combination
        pokerstove::CardSet handCards = evaluator_->bestHandCards(holeCardSet, boardCardSet);
        
        // Convert back to our Card format
        return Converter::fromPokerStoveCardSet(handCards);
    } catch (const std::exception& e) {
        LOG_ERROR("Error finding best hand: " + std::string(e.what()));
        
        // In case of error, return the original cards (up to 5)
        std::vector<Card> result;
        for (const auto& card : holeCards) {
            result.push_back(card);
        }
        for (const auto& card : communityCards) {
            if (result.size() < 5) {
                result.push_back(card);
            } else {
                break;
            }
        }
        return result;
    }
}

double HandEvaluator::calculateEquity(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                     const std::vector<Card>& communityCards,
                                     int numTrials) const {
    try {
        // Convert cards to PokerStove format
        pokerstove::CardSet holeCardSet = Converter::toPokerStoveCardSet(holeCards);
        pokerstove::CardSet boardCardSet = Converter::toPokerStoveCardSet(communityCards);
        
        // Create a deck excluding the cards we know
        pokerstove::CardSet deck;
        deck.fill(); // Fill with all 52 cards
        deck.remove(holeCardSet); // Remove hero's hole cards
        deck.remove(boardCardSet); // Remove community cards
        
        auto& random = Random::getInstance();
        
        int wins = 0;
        int ties = 0;
        
        // We'll use Monte Carlo simulation for equity calculation
        for (int i = 0; i < numTrials; i++) {
            // Create a copy of the deck for this trial
            pokerstove::CardSet remainingDeck = deck;
            
            // Create random opponent hands (2 opponents for 3-player game)
            std::vector<pokerstove::CardSet> oppHands;
            
            for (int p = 0; p < 2; p++) {
                pokerstove::CardSet oppHand;
                
                // Deal 2 cards to opponent
                for (int c = 0; c < NUM_HOLE_CARDS; c++) {
                    // Find a random card from remaining deck
                    std::vector<Card> availableCards = Converter::fromPokerStoveCardSet(remainingDeck);
                    if (availableCards.empty()) {
                        LOG_ERROR("No cards left in deck during equity calculation");
                        continue;
                    }
                    
                    // Select random card
                    int randIndex = random.getInt(0, availableCards.size() - 1);
                    Card randomCard = availableCards[randIndex];
                    
                    // Add to opponent hand and remove from deck
                    pokerstove::Card psCard = Converter::toPokerStoveCard(randomCard);
                    oppHand.insert(psCard);
                    remainingDeck.remove(psCard);
                }
                
                oppHands.push_back(oppHand);
            }
            
            // Complete the board if needed
            pokerstove::CardSet completeBoard = boardCardSet;
            int cardsToAdd = 5 - completeBoard.size();
            
            for (int c = 0; c < cardsToAdd; c++) {
                // Find a random card from remaining deck
                std::vector<Card> availableCards = Converter::fromPokerStoveCardSet(remainingDeck);
                if (availableCards.empty()) {
                    LOG_ERROR("No cards left for board completion during equity calculation");
                    continue;
                }
                
                // Select random card
                int randIndex = random.getInt(0, availableCards.size() - 1);
                Card randomCard = availableCards[randIndex];
                
                // Add to board and remove from deck
                pokerstove::Card psCard = Converter::toPokerStoveCard(randomCard);
                completeBoard.insert(psCard);
                remainingDeck.remove(psCard);
            }
            
            // Evaluate hands
            pokerstove::PokerHandEvaluation heroEval = evaluator_->evaluate(holeCardSet, completeBoard);
            
            bool win = true;
            int tieCount = 0;
            
            for (const auto& oppHand : oppHands) {
                pokerstove::PokerHandEvaluation oppEval = evaluator_->evaluate(oppHand, completeBoard);
                
                if (oppEval > heroEval) {
                    win = false;
                    break;
                } else if (oppEval == heroEval) {
                    tieCount++;
                }
            }
            
            if (win) {
                if (tieCount > 0) {
                    ties++;
                } else {
                    wins++;
                }
            }
        }
        
        // Calculate equity
        return (wins + 0.5 * ties) / numTrials;
    } catch (const std::exception& e) {
        LOG_ERROR("Error calculating equity: " + std::string(e.what()));
        return 0.5; // Return 50% equity as a fallback
    }
}

HandStrength HandEvaluator::convertToHandStrength(const pokerstove::PokerHandEvaluation& eval) const {
    HandStrength result;
    
    // Map PokerStove hand type to our HandRank
    switch (eval.type()) {
        case pokerstove::HIGH_CARD:       result.handRank = HandRank::HIGH_CARD; break;
        case pokerstove::ONE_PAIR:        result.handRank = HandRank::PAIR; break;
        case pokerstove::TWO_PAIR:        result.handRank = HandRank::TWO_PAIR; break;
        case pokerstove::THREE_OF_A_KIND: result.handRank = HandRank::THREE_OF_A_KIND; break;
        case pokerstove::STRAIGHT:        result.handRank = HandRank::STRAIGHT; break;
        case pokerstove::FLUSH:           result.handRank = HandRank::FLUSH; break;
        case pokerstove::FULL_HOUSE:      result.handRank = HandRank::FULL_HOUSE; break;
        case pokerstove::FOUR_OF_A_KIND:  result.handRank = HandRank::FOUR_OF_A_KIND; break;
        case pokerstove::STRAIGHT_FLUSH:
            // Check for royal flush (ace-high straight flush)
            if (eval.highCard() == pokerstove::ACE) {
                result.handRank = HandRank::ROYAL_FLUSH;
            } else {
                result.handRank = HandRank::STRAIGHT_FLUSH;
            }
            break;
        default: result.handRank = HandRank::HIGH_CARD; break;
    }
    
    // Extract primary and secondary values from the evaluation
    result.primaryValue = eval.majorRank();
    result.secondaryValue = eval.minorRank();
    
    // Extract kickers
    std::array<uint32_t, 5> kickers = {0};
    pokerstove::CardSet kickerCards = eval.getKickerCards();
    
    int i = 0;
    for (int cardIndex = 51; cardIndex >= 0 && i < 5; cardIndex--) {
        if (kickerCards.cardInSet(cardIndex)) {
            pokerstove::Card card(cardIndex);
            kickers[i++] = card.rank();
        }
    }
    
    result.kickers = kickers;
    
    return result;
}

} // namespace poker