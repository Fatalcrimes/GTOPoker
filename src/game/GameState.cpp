#include <game/GameState.hpp>
#include <utils/Logger.hpp>
#include <algorithm>
#include <random>
#include <sstream>
#include <chrono>
#include <stdexcept>
#include <pokerstove/peval/HoldemHandEvaluator.h>
#include <pokerstove/peval/CardSet.h>

namespace poker {


// GameState implementation
GameState::GameState() 
    : currentPosition_(Position::BTN), 
      lastAggressor_(Position::SB),
      bettingRound_(BettingRound::PREFLOP),
      pot_(0.0),
      handEvaluator_(std::make_shared<HandEvaluator>()) {
    
    // Initialize random number generator with time-based seed
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng_.seed(seed);
    
    // Initialize the game
    reset();
}

GameState::GameState(const GameState& other)
    : players_(other.players_),
      communityCards_(other.communityCards_),
      deck_(other.deck_),
      currentPosition_(other.currentPosition_),
      lastAggressor_(other.lastAggressor_),
      bettingRound_(other.bettingRound_),
      pot_(other.pot_),
      actionHistory_(other.actionHistory_),
      rng_(other.rng_),
      handEvaluator_(other.handEvaluator_) {
}

GameState& GameState::operator=(const GameState& other) {
    if (this != &other) {
        players_ = other.players_;
        communityCards_ = other.communityCards_;
        deck_ = other.deck_;
        currentPosition_ = other.currentPosition_;
        lastAggressor_ = other.lastAggressor_;
        bettingRound_ = other.bettingRound_;
        pot_ = other.pot_;
        actionHistory_ = other.actionHistory_;
        rng_ = other.rng_;
        handEvaluator_ = other.handEvaluator_;
    }
    return *this;
}

GameState::~GameState() = default;

void GameState::reset() {
    // Reset player states
    for (auto& player : players_) {
        player.stack = STARTING_STACK;
        player.currentBet = 0.0;
        player.folded = false;
        player.holeCards.clear();
    }
    
    resetDeck();
    
    currentPosition_ = Position::BTN;
    lastAggressor_ = Position::SB;
    bettingRound_ = BettingRound::PREFLOP;
    pot_ = 0.0;
    
    actionHistory_.clear();
    
    // Apply blinds
    applyBlinds();
}

void GameState::dealHoleCards() {
    for (auto& player : players_) {
        for (int i = 0; i < 2; ++i) {
            player.holeCards.insert(deck_.deal(1));
        }
    }
}

void GameState::dealFlop() {
    // Deal the flop (3 cards)
    for (int i = 0; i < 3; ++i) {
        communityCards_.insert(deck_.deal(1));
    }
}

void GameState::dealTurn() {
    communityCards_.insert(deck_.deal(1));
}

void GameState::dealRiver() {
    communityCards_.insert(deck_.deal(1));
}
//#implement
void GameState::showdown() {

}

void GameState::applyAction(const Action& requestedAction) {
    
    Action action = requestedAction;
    
    // Get current player
    PlayerState& player = players_[static_cast<size_t>(currentPosition_)];
    
    // Apply the action
    switch (action.getType()) {
        case ActionType::FOLD:
            player.folded = true;
            break;
            
        case ActionType::CHECK:
            player.checked = true;
            break;
            
        case ActionType::CALL: {
            double highestBet = getHighestBet();
            double callAmount = highestBet - player.currentBet;
            player.stack -= callAmount;
            player.currentBet += callAmount;
            pot_ += callAmount;
            break;
        }
            
        case ActionType::RAISE: {
            player.stack -= action.getAmount();
            player.currentBet += action.getAmount();
            pot_ += action.getAmount();
            lastAggressor_ = currentPosition_;
            break;
        }
    }
    
    // Record the action
    actionHistory_.addAction(currentPosition_, action);
    
    nextPosition(currentPosition_);
}

void GameState::startNextBettingRound() {
    // Reset bets
    for (auto& player : players_) {
        player.currentBet = 0.0;
    }
    
    // Advance the betting round
    bettingRound_ = nextBettingRound(bettingRound_);
    
    // Deal cards for the new round
    switch (bettingRound_) {
        case BettingRound::FLOP:
            dealFlop();
            break;
        case BettingRound::TURN:
            dealTurn();
            break;
        case BettingRound::RIVER:
            dealRiver();
            break;
        case BettingRound::SHOWDOWN:
            showdown();
            break;
        default:
            // Should not happen
            break;
    }
    
    // Reset last aggressor
    lastAggressor_ = Position::SB;
    
    // Set first player to act (usually SB, but adjust if folded)
    currentPosition_ = Position::SB;
    while (players_[static_cast<size_t>(currentPosition_)].folded) {
        currentPosition_ = nextPosition(currentPosition_);
    }
    
    // Update action history for new round
    actionHistory_.startNewRound();
}

bool GameState::isTerminal() const {
    // Count active players
    int activePlayers = 0;
    for (const auto& player : players_) {
        if (!player.folded) {
            activePlayers++;
        }
    }
    
    // If only one player left, game is terminal
    if (activePlayers <= 1) {
        return true;
    }
    
    // If we've completed the river round, game is terminal
    if (bettingRound_ == BettingRound::RIVER) {
        // Check if all active players have the same bet
        double firstBet = 0.0;
        bool foundFirst = false;
        bool allEqualBets = true;
        
        for (const auto& player : players_) {
            if (!player.folded) {
                if (!foundFirst) {
                    firstBet = player.currentBet;
                    foundFirst = true;
                } else if (std::abs(player.currentBet - firstBet) != 0) {
                    allEqualBets = false;
                    break;
                }
            }
        }
        
        return allEqualBets;  // Terminal if all players have same bet on river
    }
    
    return false;  // Not terminal
}
/*
REVIEW THE LOGIC!!!
*/
std::vector<Action> GameState::getValidActions() const {
    std::vector<Action> validActions;
    
    // Get current player
    const PlayerState& player = players_[static_cast<size_t>(currentPosition_)];
    
    // Get highest bet among all players
    double highestBet = getHighestBet();
    
    // Amount needed to call
    double callAmount = highestBet - player.currentBet;
    
    // Can always fold if there's a bet to call
    if (callAmount > 0.0) {
        validActions.push_back(Action::fold());
    }
    
    // Can check if no bet to call
    if (callAmount == 0.0) {
        validActions.push_back(Action::check());
    }
    
    // Can call if there's a bet and player has enough chips
    if (callAmount > 0.0 && callAmount <= player.stack) {
        validActions.push_back(Action::call(callAmount));
    }
    
    // Can bet/raise if player has chips
    if (player.stack > 0.0) {
        if (callAmount == 0.0) {
            // No current bet, can make a bet
            
            // Standard bet sizes: 0.5 pot, pot, 2x pot
            double halfPot = std::min(pot_ * 0.5, player.stack);
            double fullPot = std::min(pot_, player.stack);
            double twoPot = std::min(pot_ * 2.0, player.stack);
            
            // Add bet actions (avoid duplicates)
            if (halfPot >= 1.0) {
                validActions.push_back(Action::raise(halfPot));
            }
            
            if (fullPot > halfPot) {
                validActions.push_back(Action::raise(fullPot));
            }
            
            if (twoPot > fullPot) {
                validActions.push_back(Action::raise(twoPot));
            }
            
            // All-in bet if not already covered
            if (player.stack > twoPot) {
                validActions.push_back(Action::raise(player.stack));
            }
        } else {
            // There's a bet to call, can raise
            
            // Minimum raise is 2x the previous bet
            double minRaise = std::min(callAmount * 2.0, player.stack);
            
            // Standard raise sizes: min raise, 3x, 5x, all-in
            double threeX = std::min(highestBet * 3.0, player.stack);
            double fiveX = std::min(highestBet * 5.0, player.stack);
            
            // Add raise actions (avoid duplicates and ensure they exceed call amount)
            if (minRaise > callAmount) {
                validActions.push_back(Action::raise(minRaise));
            }
            
            if (threeX > minRaise) {
                validActions.push_back(Action::raise(threeX));
            }
            
            if (fiveX > threeX) {
                validActions.push_back(Action::raise(fiveX));
            }
            
            // All-in raise if not already covered
            if (player.stack > fiveX) {
                validActions.push_back(Action::raise(player.stack));
            }
        }
    }
    
    return validActions;
}

std::unordered_map<Position, double> GameState::getPayoffs() const {

    // Initialize payoffs with negative bets
    std::unordered_map<Position, double> payoffs;
    for (size_t i = 0; i < players_.size(); ++i) {
        Position pos = static_cast<Position>(i);
        payoffs[pos] = -players_[i].currentBet;
    }

    // Identify active players
    std::vector<Position> activePlayers;
    for (size_t i = 0; i < players_.size(); ++i) {
        if (!players_[i].folded) {
            activePlayers.push_back(static_cast<Position>(i));
        }
    }

    // Single active player wins the pot
    if (activePlayers.size() == 1) {
        payoffs[activePlayers[0]] += pot_;
        return payoffs;
    }

    // Multiple active players: evaluate hands with PokerStove
    std::vector<std::pair<Position, pokerstove::PokerHandEvaluation>> handStrengths;
    for (Position pos : activePlayers) {
        const PlayerState& player = players_[static_cast<size_t>(pos)];
        pokerstove::PokerHandEvaluation strength = handEvaluator.evaluateHand(player.holeCards, communityCards_);
        handStrengths.emplace_back(pos, strength);
    }

    // Sort by hand strength (descending)
    std::sort(handStrengths.begin(), handStrengths.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Identify winners (handle ties)
    std::vector<Position> winners;
    pokerstove::PokerHandEvaluation winningStrength = handStrengths[0].second;
    for (const auto& [pos, strength] : handStrengths) {
        if (strength == winningStrength) {
            winners.push_back(pos);
        } else {
            break; // No more winners due to descending order
        }
    }

    // Distribute pot among winners
    double winAmount = pot_ / winners.size();
    for (Position winner : winners) {
        payoffs[winner] += winAmount;
    }

    return payoffs;
}

std::unique_ptr<GameState> GameState::clone() const {
    return std::make_unique<GameState>(*this);
}

std::string GameState::toString() const {
    std::ostringstream oss;
    
    // Game state overview
    oss << "Round: " << bettingRoundToString(bettingRound_) << "\n";
    oss << "Pot: " << pot_ << "\n";
    oss << "Current position: " << positionToString(currentPosition_) << "\n\n";
    
    // Community cards
    oss << "Community cards: ";
    for (const auto& card : communityCards_) {
        oss << card.toString() << " ";
    }
    oss << "\n\n";
    
    // Player states
    for (size_t i = 0; i < players_.size(); ++i) {
        Position pos = static_cast<Position>(i);
        const auto& player = players_[i];
        
        oss << positionToString(pos) << ": ";
        oss << "Stack=" << player.stack << ", ";
        oss << "Bet=" << player.currentBet << ", ";
        oss << "Folded=" << (player.folded ? "true" : "false") << ", ";
        oss << "Cards=[" << player.holeCards[0].toString() << " " 
                         << player.holeCards[1].toString() << "]\n";
    }
    oss << "\n";
    
    // Action history
    oss << "Action history: " << actionHistory_.toString() << "\n";
    
    return oss.str();
}

void GameState::resetDeck() {
    communityCards_.clear();
    deck_.reset();
    deck_.shuffle();
}

void GameState::applyBlinds() {
    // Apply small blind
    PlayerState& sbPlayer = players_[static_cast<size_t>(Position::SB)];
    sbPlayer.stack -= SMALL_BLIND;
    sbPlayer.currentBet = SMALL_BLIND;
    
    // Apply big blind
    PlayerState& bbPlayer = players_[static_cast<size_t>(Position::BB)];
    bbPlayer.stack -= BIG_BLIND;
    bbPlayer.currentBet = BIG_BLIND;
    
    // Update pot
    pot_ = SMALL_BLIND + BIG_BLIND;
}

double GameState::getHighestBet() const {
    double highestBet = 0.0;
    
    for (const auto& player : players_) {
        highestBet = std::max(highestBet, player.currentBet);
    }
    
    return highestBet;
}

} // namespace poker