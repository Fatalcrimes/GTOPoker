#include "game/GameState.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <chrono>
#include <stdexcept>

namespace poker {

// Forward declare the HandEvaluator
class HandEvaluator {
public:
    // Evaluate hand strength for a player (2 hole cards + community cards)
    HandStrength evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                             const std::vector<Card>& communityCards) const;
    
    // Find the best 5-card hand
    std::vector<Card> findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                  const std::vector<Card>& communityCards) const;
};

// Implementation of HandEvaluator
HandStrength HandEvaluator::evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                        const std::vector<Card>& communityCards) const {
    // TODO: Implement a full hand evaluator
    // This is a simplified placeholder implementation
    
    // Combine hole cards and community cards
    std::vector<Card> allCards;
    allCards.reserve(holeCards.size() + communityCards.size());
    allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
    allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());
    
    // Count occurrences of each rank
    std::array<int, 15> rankCounts = {0};
    for (const auto& card : allCards) {
        rankCounts[static_cast<int>(card.rank)]++;
    }
    
    // Check for pairs, three of a kind, etc.
    int numPairs = 0;
    int pairRank = 0;
    int threeOfAKindRank = 0;
    int fourOfAKindRank = 0;
    
    for (int i = static_cast<int>(Rank::TWO); i <= static_cast<int>(Rank::ACE); i++) {
        if (rankCounts[i] == 2) {
            numPairs++;
            pairRank = std::max(pairRank, i);
        } else if (rankCounts[i] == 3) {
            threeOfAKindRank = i;
        } else if (rankCounts[i] == 4) {
            fourOfAKindRank = i;
        }
    }
    
    // Determine hand rank
    HandRank handRank;
    uint32_t primaryValue = 0;
    uint32_t secondaryValue = 0;
    std::array<uint32_t, 5> kickers = {0};
    
    if (fourOfAKindRank > 0) {
        handRank = HandRank::FOUR_OF_A_KIND;
        primaryValue = fourOfAKindRank;
    } else if (threeOfAKindRank > 0 && numPairs > 0) {
        handRank = HandRank::FULL_HOUSE;
        primaryValue = threeOfAKindRank;
        secondaryValue = pairRank;
    } else if (threeOfAKindRank > 0) {
        handRank = HandRank::THREE_OF_A_KIND;
        primaryValue = threeOfAKindRank;
    } else if (numPairs >= 2) {
        handRank = HandRank::TWO_PAIR;
        primaryValue = pairRank;
        // Find the second highest pair
        for (int i = static_cast<int>(Rank::ACE); i >= static_cast<int>(Rank::TWO); i--) {
            if (rankCounts[i] == 2 && i != pairRank) {
                secondaryValue = i;
                break;
            }
        }
    } else if (numPairs == 1) {
        handRank = HandRank::PAIR;
        primaryValue = pairRank;
    } else {
        handRank = HandRank::HIGH_CARD;
        // Find the highest card
        for (int i = static_cast<int>(Rank::ACE); i >= static_cast<int>(Rank::TWO); i--) {
            if (rankCounts[i] > 0) {
                primaryValue = i;
                break;
            }
        }
    }
    
    // TODO: Check for straights, flushes, etc.
    // This would require more complex evaluation
    
    return HandStrength{handRank, primaryValue, secondaryValue, kickers};
}

std::vector<Card> HandEvaluator::findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                             const std::vector<Card>& communityCards) const {
    // TODO: Implement logic to find the best 5-card hand
    // Placeholder implementation
    std::vector<Card> bestHand;
    bestHand.reserve(5);
    
    // Just return the 5 highest cards for now
    std::vector<Card> allCards;
    allCards.reserve(holeCards.size() + communityCards.size());
    allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
    allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());
    
    // Sort cards by rank (descending)
    std::sort(allCards.begin(), allCards.end(), [](const Card& a, const Card& b) {
        return static_cast<int>(a.rank) > static_cast<int>(b.rank);
    });
    
    // Take the 5 highest cards
    for (size_t i = 0; i < 5 && i < allCards.size(); ++i) {
        bestHand.push_back(allCards[i]);
    }
    
    return bestHand;
}

// GameState implementation
GameState::GameState() 
    : currentPosition_(Position::SB), 
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
      usedCards_(other.usedCards_),
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
        usedCards_ = other.usedCards_;
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
        // Hole cards will be dealt later
    }
    
    // Reset game state
    communityCards_.clear();
    usedCards_.clear();
    resetDeck();
    
    currentPosition_ = Position::SB;
    lastAggressor_ = Position::SB;
    bettingRound_ = BettingRound::PREFLOP;
    pot_ = 0.0;
    
    actionHistory_.clear();
    
    // Apply blinds
    applyBlinds();
}

void GameState::dealHoleCards() {
    // Ensure deck is reset and shuffled
    resetDeck();
    
    // Deal two cards to each player
    for (auto& player : players_) {
        for (int i = 0; i < NUM_HOLE_CARDS; ++i) {
            player.holeCards[i] = dealCard();
        }
    }
}

void GameState::dealFlop() {
    if (!communityCards_.empty()) {
        throw std::runtime_error("Community cards already exist");
    }
    
    // Burn a card
    dealCard();
    
    // Deal the flop (3 cards)
    for (int i = 0; i < 3; ++i) {
        communityCards_.push_back(dealCard());
    }
}

void GameState::dealTurn() {
    if (communityCards_.size() != 3) {
        throw std::runtime_error("Need flop before turn");
    }
    
    // Burn a card
    dealCard();
    
    // Deal the turn
    communityCards_.push_back(dealCard());
}

void GameState::dealRiver() {
    if (communityCards_.size() != 4) {
        throw std::runtime_error("Need turn before river");
    }
    
    // Burn a card
    dealCard();
    
    // Deal the river
    communityCards_.push_back(dealCard());
}

bool GameState::applyAction(const Action& action) {
    // Validate the action
    auto validActions = getValidActions();
    if (std::none_of(validActions.begin(), validActions.end(), 
                    [&action](const Action& a) { return a == action; })) {
        throw std::invalid_argument("Invalid action: " + action.toString());
    }
    
    // Get current player
    PlayerState& player = players_[static_cast<size_t>(currentPosition_)];
    
    // Apply the action
    switch (action.getType()) {
        case ActionType::FOLD:
            player.folded = true;
            break;
            
        case ActionType::CHECK:
            // No change to state
            break;
            
        case ActionType::CALL: {
            double highestBet = getHighestBet();
            double callAmount = highestBet - player.currentBet;
            player.stack -= callAmount;
            player.currentBet += callAmount;
            pot_ += callAmount;
            break;
        }
            
        case ActionType::BET: {
            player.stack -= action.getAmount();
            player.currentBet += action.getAmount();
            pot_ += action.getAmount();
            lastAggressor_ = currentPosition_;
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
    
    // Advance to next player or round
    return advanceAction();
}

bool GameState::advanceAction() {
    // Count active (not folded) players
    int activePlayers = 0;
    for (const auto& player : players_) {
        if (!player.folded) {
            activePlayers++;
        }
    }
    
    // If only one player left, round is over
    if (activePlayers <= 1) {
        return true;
    }
    
    // Check if all active players have the same bet
    bool allEqualBets = true;
    double firstBet = 0.0;
    bool foundFirst = false;
    
    for (const auto& player : players_) {
        if (!player.folded) {
            if (!foundFirst) {
                firstBet = player.currentBet;
                foundFirst = true;
            } else if (player.currentBet != firstBet) {
                allEqualBets = false;
                break;
            }
        }
    }
    
    // Find the next position to act
    Position nextPos = currentPosition_;
    do {
        nextPos = nextPosition(nextPos);
    } while (players_[static_cast<size_t>(nextPos)].folded);
    
    // If we're back to the last aggressor (or start of the round) and all bets are equal,
    // the betting round is over
    if (allEqualBets && (nextPos == lastAggressor_ || lastAggressor_ == Position::SB)) {
        return true;
    }
    
    // Otherwise, move to the next player
    currentPosition_ = nextPos;
    return false;
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
    actionHistory_.roundStartIndices_.push_back(actionHistory_.getActions().size());
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
        
        for (const auto& player : players_) {
            if (!player.folded) {
                if (!foundFirst) {
                    firstBet = player.currentBet;
                    foundFirst = true;
                } else if (player.currentBet != firstBet) {
                    return false;  // Not all players have same bet
                }
            }
        }
        
        return true;  // All players have same bet, river round complete
    }
    
    return false;  // Not terminal
}

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
            if (halfPot > 0.0) {
                validActions.push_back(Action::bet(halfPot));
            }
            
            if (fullPot > halfPot) {
                validActions.push_back(Action::bet(fullPot));
            }
            
            if (twoPot > fullPot) {
                validActions.push_back(Action::bet(twoPot));
            }
            
            // All-in bet if not already covered
            if (player.stack > twoPot) {
                validActions.push_back(Action::bet(player.stack));
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

std::string GameState::getInfoSet(Position position) const {
    std::ostringstream oss;
    
    // Add hole cards
    const PlayerState& player = players_[static_cast<size_t>(position)];
    oss << player.holeCards[0].toString() << player.holeCards[1].toString() << "|";
    
    // Add community cards
    for (const auto& card : communityCards_) {
        oss << card.toString();
    }
    oss << "|";
    
    // Add betting round
    oss << bettingRoundToString(bettingRound_) << "|";
    
    // Add action history
    oss << actionHistory_.toString();
    
    return oss.str();
}

std::unordered_map<Position, double> GameState::getPayoffs() const {
    if (!isTerminal()) {
        throw std::runtime_error("Cannot calculate payoffs for non-terminal state");
    }
    
    // Initialize payoffs (negative initial bets)
    std::unordered_map<Position, double> payoffs;
    for (size_t i = 0; i < players_.size(); ++i) {
        Position pos = static_cast<Position>(i);
        payoffs[pos] = -players_[i].currentBet;
    }
    
    // Count active players
    std::vector<Position> activePlayers;
    for (size_t i = 0; i < players_.size(); ++i) {
        if (!players_[i].folded) {
            activePlayers.push_back(static_cast<Position>(i));
        }
    }
    
    // If only one player is active, they win the pot
    if (activePlayers.size() == 1) {
        payoffs[activePlayers[0]] += pot_;
        return payoffs;
    }
    
    // Otherwise, we need to determine the winner based on hand strength
    std::vector<std::pair<Position, HandStrength>> handStrengths;
    
    for (Position pos : activePlayers) {
        const auto& player = players_[static_cast<size_t>(pos)];
        HandStrength strength = handEvaluator_->evaluateHand(player.holeCards, communityCards_);
        handStrengths.emplace_back(pos, strength);
    }
    
    // Sort by hand strength (descending)
    std::sort(handStrengths.begin(), handStrengths.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Find all players with the winning hand strength
    std::vector<Position> winners;
    HandStrength winningStrength = handStrengths[0].second;
    
    for (const auto& [pos, strength] : handStrengths) {
        if (strength == winningStrength) {
            winners.push_back(pos);
        } else {
            break;  // No more winners
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
    deck_.clear();
    
    // Create a full deck
    for (int suit = 0; suit < NUM_SUITS; ++suit) {
        for (int rank = static_cast<int>(Rank::TWO); rank <= static_cast<int>(Rank::ACE); ++rank) {
            deck_.emplace_back(static_cast<Rank>(rank), static_cast<Suit>(suit));
        }
    }
    
    // Shuffle the deck
    std::shuffle(deck_.begin(), deck_.end(), rng_);
}

Card GameState::dealCard() {
    if (deck_.empty()) {
        throw std::runtime_error("No cards left in deck");
    }
    
    Card card = deck_.back();
    deck_.pop_back();
    usedCards_.push_back(card);
    
    return card;
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