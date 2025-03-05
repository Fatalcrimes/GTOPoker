#pragma once

#include <vector>
#include <string>
#include <array>
#include <memory>
#include <random>
#include <unordered_map>

#include "game/PokerDefs.hpp"
#include "game/Action.hpp"

namespace poker {

// Forward declarations
class HandEvaluator;

// Player state
struct PlayerState {
    double stack;
    double currentBet;
    bool folded;
    std::array<Card, NUM_HOLE_CARDS> holeCards;
    
    PlayerState() : stack(STARTING_STACK), currentBet(0.0), folded(false) {}
};

class GameState {
public:
    // Constructors
    GameState();
    GameState(const GameState& other);
    GameState& operator=(const GameState& other);
    ~GameState();
    
    // Game initialization
    void reset();
    void dealHoleCards();
    void dealFlop();
    void dealTurn();
    void dealRiver();
    
    // Game progression
    bool applyAction(const Action& action);
    bool advanceAction();
    void startNextBettingRound();
    
    // State queries
    bool isTerminal() const;
    Position getCurrentPosition() const { return currentPosition_; }
    BettingRound getBettingRound() const { return bettingRound_; }
    double getPot() const { return pot_; }
    
    // Actions
    std::vector<Action> getValidActions() const;
    
    Action findClosestValidAction(const Action& action) const;

    // Information sets
    std::string getInfoSet(Position position) const;
    
    // Payoffs
    std::unordered_map<Position, double> getPayoffs() const;
    
    // Player state access
    const PlayerState& getPlayerState(Position position) const { return players_[static_cast<size_t>(position)]; }
    
    // Community cards access
    const std::vector<Card>& getCommunityCards() const { return communityCards_; }
    
    // Action history
    const ActionHistory& getActionHistory() const { return actionHistory_; }
    
    // Cloning the game state
    std::unique_ptr<GameState> clone() const;
    
    // String representation
    std::string toString() const;

private:
    // Deck management
    void resetDeck();
    Card dealCard();
    
    // Betting
    void applyBlinds();
    double getHighestBet() const;
    
    // State variables
    std::array<PlayerState, NUM_PLAYERS> players_;
    std::vector<Card> communityCards_;
    std::vector<Card> deck_;
    std::vector<Card> usedCards_;
    
    Position currentPosition_ = Position::SB;
    Position lastAggressor_ = Position::SB;
    BettingRound bettingRound_ = BettingRound::PREFLOP;
    
    double pot_ = 0.0;
    ActionHistory actionHistory_;
    
    // Random number generator
    std::mt19937 rng_;
    
    // Hand evaluator
    std::shared_ptr<HandEvaluator> handEvaluator_;
};

} // namespace poker