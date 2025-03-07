#pragma once

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <random>
#include <unordered_map>

#include <game/PokerDefs.hpp>
#include <game/Action.hpp>
#include <pokerstove/peval/CardSet.h>
#include <pokerstove/penum/SimpleDeck.hpp>

namespace poker {

// Forward declarations
class HandEvaluator;

// Player state
struct PlayerState {
    Position pos;
    double stack;
    double currentBet;
    bool folded;
    bool checked;
    pokerstove::CardSet holeCards;
    
    PlayerState() : stack(STARTING_STACK), currentBet(0.0), folded(false), checked(false), holeCards() {}
};

class GameState {
public:
    // Constructors
    GameState();
    GameState(const GameState& other);
    GameState& operator=(const GameState& other);
    ~GameState();
    
/*
This resets the game state including player chips, hole cards, deck, and action.
*/
    void reset();
/*
Deals the 2 Hole cards to each player.
*/    
    void dealHoleCards();
/*
Puts 3 cards in the community cardset and removes them from the deck
*/
    void dealFlop();
/*
Puts 1 card in the community set and removes it from the deck
*/
    void dealTurn();
/*
Puts 1 card in the community set and removes it from the deck
*/
    void dealRiver();
/*
Showdown for final result
*/
    void GameState::showdown();
/*
The applies some action either fold, raise, or call.
*/  
    void applyAction(const Action& action);
/*
Checks if there are any actions left to be made else go to the next betting round.
*/
    bool checkActions();
/*
Starts the next betting round after all actions are done
*/
    void startNextBettingRound();
    
    // State queries
    bool isTerminal() const;
    Position getCurrentPosition() const { return currentPosition_; }
    BettingRound getBettingRound() const { return bettingRound_; }
    double getPot() const { return pot_; }
    
    // Actions
    std::vector<Action> getValidActions() const;
    
    // Payoffs
    std::unordered_map<Position, double> getPayoffs() const;
    
    // Player state access
    const PlayerState& getPlayerState(Position position) const { return players_[static_cast<size_t>(position)]; }
    
    // Community cards access
    const pokerstove::CardSet& getCommunityCards() const { return communityCards_; }
    
    // Action history
    const ActionHistory& getActionHistory() const { return actionHistory_; }
    
    // Cloning the game state
    std::unique_ptr<GameState> clone() const;
    
    // String representation
    std::string toString() const;

private:
    // Deck management
    void resetDeck();
    
    // Betting
    void applyBlinds();
    double getHighestBet() const;
    
    // State variables
    std::array<PlayerState, NUM_PLAYERS> players_;
    pokerstove::CardSet communityCards_;
    pokerstove::SimpleDeck deck_;
    
    Position currentPosition_ = Position::BTN;
    Position lastAggressor_ = Position::SB;
    BettingRound bettingRound_ = BettingRound::PREFLOP;
    
    double pot_ = 0.0;
    ActionHistory actionHistory_;
    
    // Random number generator
    std::mt19937 rng_;
    
    // Hand evaluator
    pokerstove::HoldemHandEvaluator handEvaluator;
};

} // namespace poker