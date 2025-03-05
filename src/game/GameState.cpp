#include "game/GameState.hpp"
#include <algorithm>
#include <random>
#include <sstream>
#include <chrono>
#include <stdexcept>

namespace poker {

// Forward declare the HandEvaluator
// Enhanced HandEvaluator implementation
// This should replace the simplified version in GameState.cpp

class HandEvaluator {
public:
    // Evaluate hand strength for a player (2 hole cards + community cards)
    HandStrength evaluateHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                             const std::vector<Card>& communityCards) const {
        // Find the best 5-card hand from all 7 cards
        std::vector<Card> allCards;
        allCards.reserve(holeCards.size() + communityCards.size());
        allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
        allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());

        // Store all possible 5-card combinations
        std::vector<std::vector<Card>> combinations;
        std::vector<Card> currentCombo(5);
        generateCombinations(allCards, 0, 0, 5, currentCombo, combinations);

        // Evaluate each combination and find the best one
        HandStrength bestStrength;
        bool firstHand = true;

        for (const auto& combo : combinations) {
            HandStrength currentStrength = evaluateFiveCardHand(combo);
            
            if (firstHand || currentStrength > bestStrength) {
                bestStrength = currentStrength;
                firstHand = false;
            }
        }

        return bestStrength;
    }
    
    // Find the best 5-card hand
    std::vector<Card> findBestHand(const std::array<Card, NUM_HOLE_CARDS>& holeCards,
                                  const std::vector<Card>& communityCards) const {
        std::vector<Card> allCards;
        allCards.reserve(holeCards.size() + communityCards.size());
        allCards.insert(allCards.end(), holeCards.begin(), holeCards.end());
        allCards.insert(allCards.end(), communityCards.begin(), communityCards.end());

        // Store all possible 5-card combinations
        std::vector<std::vector<Card>> combinations;
        std::vector<Card> currentCombo(5);
        generateCombinations(allCards, 0, 0, 5, currentCombo, combinations);

        // Evaluate each combination and find the best one
        HandStrength bestStrength;
        std::vector<Card> bestHand;
        bool firstHand = true;

        for (const auto& combo : combinations) {
            HandStrength currentStrength = evaluateFiveCardHand(combo);
            
            if (firstHand || currentStrength > bestStrength) {
                bestStrength = currentStrength;
                bestHand = combo;
                firstHand = false;
            }
        }

        return bestHand;
    }

private:
    // Generate all k-combinations of n elements
    void generateCombinations(const std::vector<Card>& cards, int start, int index, int k, 
                              std::vector<Card>& current, std::vector<std::vector<Card>>& result) const {
        // If we have selected k elements, add to result
        if (index == k) {
            result.push_back(current);
            return;
        }

        // Try to select from remaining elements
        for (int i = start; i <= (int)cards.size() - k + index; ++i) {
            current[index] = cards[i];
            generateCombinations(cards, i + 1, index + 1, k, current, result);
        }
    }

    // Evaluate a 5-card hand
    HandStrength evaluateFiveCardHand(const std::vector<Card>& cards) const {
        if (cards.size() != 5) {
            throw std::invalid_argument("Hand must contain exactly 5 cards");
        }

        // Check for flush
        bool isFlush = isHandFlush(cards);
        
        // Check for straight
        bool isStraight = isHandStraight(cards);
        
        // Count occurrences of each rank
        std::array<int, 15> rankCounts = {0};
        for (const auto& card : cards) {
            rankCounts[static_cast<int>(card.rank)]++;
        }
        
        // Find ranks by frequency (for pairs, etc.)
        std::vector<std::pair<int, int>> ranksByFreq; // (rank, count)
        for (int i = static_cast<int>(Rank::TWO); i <= static_cast<int>(Rank::ACE); ++i) {
            if (rankCounts[i] > 0) {
                ranksByFreq.emplace_back(i, rankCounts[i]);
            }
        }
        
        // Sort by frequency (high to low), then by rank (high to low)
        std::sort(ranksByFreq.begin(), ranksByFreq.end(), 
                 [](const auto& a, const auto& b) {
                     if (a.second != b.second) return a.second > b.second;
                     return a.first > b.first;
                 });
        
        // Extract kickers (ranks in descending order)
        std::array<uint32_t, 5> kickers = {0};
        for (size_t i = 0; i < ranksByFreq.size() && i < 5; ++i) {
            kickers[i] = ranksByFreq[i].first;
        }

        // Determine hand rank and values
        HandRank handRank;
        uint32_t primaryValue = 0;
        uint32_t secondaryValue = 0;
        
        // Royal flush
        if (isFlush && isStraight && hasRank(cards, Rank::ACE) && hasRank(cards, Rank::KING)) {
            int highestRank = getHighestRank(cards);
            if (highestRank == static_cast<int>(Rank::ACE)) {
                handRank = HandRank::ROYAL_FLUSH;
                primaryValue = static_cast<int>(Rank::ACE);
                return HandStrength{handRank, primaryValue, secondaryValue, kickers};
            }
        }
        
        // Straight flush
        if (isFlush && isStraight) {
            handRank = HandRank::STRAIGHT_FLUSH;
            primaryValue = getHighestRank(cards);
            // Handle A-5 straight flush
            if (hasRank(cards, Rank::ACE) && hasRank(cards, Rank::TWO) && 
                hasRank(cards, Rank::THREE) && hasRank(cards, Rank::FOUR) && 
                hasRank(cards, Rank::FIVE)) {
                primaryValue = static_cast<int>(Rank::FIVE); // A-5 straight is ranked by 5
            }
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Four of a kind
        if (ranksByFreq[0].second == 4) {
            handRank = HandRank::FOUR_OF_A_KIND;
            primaryValue = ranksByFreq[0].first;
            secondaryValue = ranksByFreq[1].first; // The kicker
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Full house
        if (ranksByFreq[0].second == 3 && ranksByFreq[1].second == 2) {
            handRank = HandRank::FULL_HOUSE;
            primaryValue = ranksByFreq[0].first;   // Triplet rank
            secondaryValue = ranksByFreq[1].first; // Pair rank
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Flush
        if (isFlush) {
            handRank = HandRank::FLUSH;
            // Use kickers for tie-breaking (already sorted by rank)
            return HandStrength{handRank, kickers[0], kickers[1], kickers};
        }
        
        // Straight
        if (isStraight) {
            handRank = HandRank::STRAIGHT;
            primaryValue = getHighestRank(cards);
            // Handle A-5 straight
            if (hasRank(cards, Rank::ACE) && hasRank(cards, Rank::TWO) && 
                hasRank(cards, Rank::THREE) && hasRank(cards, Rank::FOUR) && 
                hasRank(cards, Rank::FIVE)) {
                primaryValue = static_cast<int>(Rank::FIVE); // A-5 straight is ranked by 5
            }
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Three of a kind
        if (ranksByFreq[0].second == 3) {
            handRank = HandRank::THREE_OF_A_KIND;
            primaryValue = ranksByFreq[0].first;
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // Two pair
        if (ranksByFreq[0].second == 2 && ranksByFreq[1].second == 2) {
            handRank = HandRank::TWO_PAIR;
            primaryValue = ranksByFreq[0].first;   // Higher pair
            secondaryValue = ranksByFreq[1].first; // Lower pair
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // One pair
        if (ranksByFreq[0].second == 2) {
            handRank = HandRank::PAIR;
            primaryValue = ranksByFreq[0].first;
            // Kickers for tie-breaking
            return HandStrength{handRank, primaryValue, secondaryValue, kickers};
        }
        
        // High card
        handRank = HandRank::HIGH_CARD;
        primaryValue = ranksByFreq[0].first;
        // Kickers for tie-breaking
        return HandStrength{handRank, primaryValue, secondaryValue, kickers};
    }
    
    // Helper methods for hand evaluation
    
    bool isHandFlush(const std::vector<Card>& cards) const {
        if (cards.empty()) return false;
        
        Suit firstSuit = cards[0].suit;
        for (size_t i = 1; i < cards.size(); ++i) {
            if (cards[i].suit != firstSuit) {
                return false;
            }
        }
        return true;
    }
    
    bool isHandStraight(const std::vector<Card>& cards) const {
        if (cards.size() < 5) return false;
        
        // Extract ranks and sort them
        std::vector<int> ranks;
        for (const auto& card : cards) {
            ranks.push_back(static_cast<int>(card.rank));
        }
        std::sort(ranks.begin(), ranks.end());
        
        // Remove duplicates
        ranks.erase(std::unique(ranks.begin(), ranks.end()), ranks.end());
        
        // Check for standard straight
        if (ranks.size() >= 5) {
            for (size_t i = 0; i <= ranks.size() - 5; ++i) {
                if (ranks[i+4] - ranks[i] == 4) {
                    return true;
                }
            }
        }
        
        // Check for A-5 straight
        if (std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::ACE)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::TWO)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::THREE)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::FOUR)) != ranks.end() &&
            std::find(ranks.begin(), ranks.end(), static_cast<int>(Rank::FIVE)) != ranks.end()) {
            return true;
        }
        
        return false;
    }
    
    int getHighestRank(const std::vector<Card>& cards) const {
        int highest = 0;
        for (const auto& card : cards) {
            highest = std::max(highest, static_cast<int>(card.rank));
        }
        return highest;
    }
    
    bool hasRank(const std::vector<Card>& cards, Rank rank) const {
        for (const auto& card : cards) {
            if (card.rank == rank) {
                return true;
            }
        }
        return false;
    }
};

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

bool GameState::applyAction(const Action& requestedAction) {
    // Find the closest valid action
    Action action = findClosestValidAction(requestedAction);
    
    // Check if this action is valid
    auto validActions = getValidActions();
    bool isValid = false;
    
    for (const auto& validAction : validActions) {
        if (validAction == action) {
            isValid = true;
            break;
        }
    }
    
    // If not valid, report error with details
    if (!isValid) {
        std::ostringstream oss;
        oss << "Invalid action: " << requestedAction.toString();
        if (requestedAction.toString() != action.toString()) {
            oss << " (adjusted to " << action.toString() << ")";
        }
        oss << ". Valid actions are: ";
        for (const auto& validAction : validActions) {
            oss << validAction.toString() << ", ";
        }
        throw std::invalid_argument(oss.str());
    }
    
    // Rest of the method remains the same...
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

Action GameState::findClosestValidAction(const Action& action) const {
    auto validActions = getValidActions();
    
    // First, try to find an exact match
    for (const auto& validAction : validActions) {
        if (validAction == action) {
            return validAction;
        }
    }
    
    // If not found, find the closest by amount (for BET/RAISE/CALL)
    if (action.getType() == ActionType::BET || 
        action.getType() == ActionType::RAISE || 
        action.getType() == ActionType::CALL) {
        
        Action closestAction = action;
        double minDiff = std::numeric_limits<double>::max();
        
        for (const auto& validAction : validActions) {
            if (validAction.getType() == action.getType()) {
                double diff = std::abs(validAction.getAmount() - action.getAmount());
                if (diff < minDiff) {
                    minDiff = diff;
                    closestAction = validAction;
                }
            }
        }
        
        // If we found a close match with small difference, return it
        const double EPSILON = 0.01; // 1 cent difference is acceptable
        if (minDiff < EPSILON) {
            return closestAction;
        }
    }
    
    // For non-bet actions or if no close match, return the original
    return action;
}

} // namespace poker