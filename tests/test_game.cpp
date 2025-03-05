#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "game/GameState.hpp"
#include "game/Action.hpp"
#include "game/PokerDefs.hpp"

using namespace poker;

// Simple testing framework
#define TEST(name) void name()
#define ASSERT(condition) assert(condition)
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_NE(a, b) assert((a) != (b))
#define ASSERT_TRUE(a) assert(a)
#define ASSERT_FALSE(a) assert(!(a))
#define RUN_TEST(name) std::cout << "Running " << #name << "... "; name(); std::cout << "PASSED" << std::endl

// Tests for Card class
TEST(test_card) {
    Card c1(Rank::ACE, Suit::SPADE);
    Card c2(Rank::ACE, Suit::SPADE);
    Card c3(Rank::KING, Suit::SPADE);
    Card c4(Rank::ACE, Suit::HEART);
    
    ASSERT_EQ(c1, c2);
    ASSERT_NE(c1, c3);
    ASSERT_NE(c1, c4);
    
    ASSERT_TRUE(c3 < c1);  // King < Ace
    ASSERT_FALSE(c1 < c3);
    
    ASSERT_EQ(c1.toString(), "AS");
    ASSERT_EQ(c3.toString(), "KS");
    ASSERT_EQ(c4.toString(), "AH");
}

// Tests for Position enum
TEST(test_position) {
    ASSERT_EQ(nextPosition(Position::SB), Position::BB);
    ASSERT_EQ(nextPosition(Position::BB), Position::BTN);
    ASSERT_EQ(nextPosition(Position::BTN), Position::SB);
    
    ASSERT_EQ(positionToString(Position::SB), "SB");
    ASSERT_EQ(positionToString(Position::BB), "BB");
    ASSERT_EQ(positionToString(Position::BTN), "BTN");
}

// Tests for BettingRound enum
TEST(test_betting_round) {
    ASSERT_EQ(nextBettingRound(BettingRound::PREFLOP), BettingRound::FLOP);
    ASSERT_EQ(nextBettingRound(BettingRound::FLOP), BettingRound::TURN);
    ASSERT_EQ(nextBettingRound(BettingRound::TURN), BettingRound::RIVER);
    ASSERT_EQ(nextBettingRound(BettingRound::RIVER), BettingRound::RIVER);  // No next round after river
    
    ASSERT_EQ(bettingRoundToString(BettingRound::PREFLOP), "PREFLOP");
    ASSERT_EQ(bettingRoundToString(BettingRound::FLOP), "FLOP");
    ASSERT_EQ(bettingRoundToString(BettingRound::TURN), "TURN");
    ASSERT_EQ(bettingRoundToString(BettingRound::RIVER), "RIVER");
}

// Tests for Action class
TEST(test_action) {
    Action a1 = Action::fold();
    Action a2 = Action::check();
    Action a3 = Action::call(1.0);
    Action a4 = Action::bet(2.0);
    Action a5 = Action::raise(3.0);
    
    ASSERT_EQ(a1.getType(), ActionType::FOLD);
    ASSERT_EQ(a2.getType(), ActionType::CHECK);
    ASSERT_EQ(a3.getType(), ActionType::CALL);
    ASSERT_EQ(a4.getType(), ActionType::BET);
    ASSERT_EQ(a5.getType(), ActionType::RAISE);
    
    ASSERT_EQ(a1.getAmount(), 0.0);
    ASSERT_EQ(a2.getAmount(), 0.0);
    ASSERT_EQ(a3.getAmount(), 1.0);
    ASSERT_EQ(a4.getAmount(), 2.0);
    ASSERT_EQ(a5.getAmount(), 3.0);
    
    ASSERT_EQ(a1.toString(), "FOLD");
    ASSERT_EQ(a2.toString(), "CHECK");
    ASSERT_EQ(a3.toString(), "CALL 1");
    ASSERT_EQ(a4.toString(), "BET 2");
    ASSERT_EQ(a5.toString(), "RAISE 3");
    
    // Test equality operators
    Action a6 = Action::fold();
    Action a7 = Action::bet(2.0);
    
    ASSERT_EQ(a1, a6);
    ASSERT_EQ(a4, a7);
    ASSERT_NE(a1, a2);
    ASSERT_NE(a3, a4);
}

// Tests for ActionHistory class
TEST(test_action_history) {
    ActionHistory history;
    
    history.addAction(Position::SB, Action::bet(2.0));
    history.addAction(Position::BB, Action::call(2.0));
    history.addAction(Position::BTN, Action::raise(6.0));
    
    const auto& actions = history.getActions();
    ASSERT_EQ(actions.size(), 3);
    ASSERT_EQ(actions[0].first, Position::SB);
    ASSERT_EQ(actions[0].second, Action::bet(2.0));
    ASSERT_EQ(actions[1].first, Position::BB);
    ASSERT_EQ(actions[1].second, Action::call(2.0));
    ASSERT_EQ(actions[2].first, Position::BTN);
    ASSERT_EQ(actions[2].second, Action::raise(6.0));
    
    history.clear();
    ASSERT_EQ(history.getActions().size(), 0);
}

// Tests for GameState class
TEST(test_game_state) {
    GameState state;
    
    // Initial state should be preflop with SB to act
    ASSERT_EQ(state.getBettingRound(), BettingRound::PREFLOP);
    ASSERT_EQ(state.getCurrentPosition(), Position::SB);
    
    // Pot should have blinds
    ASSERT_EQ(state.getPot(), SMALL_BLIND + BIG_BLIND);
    
    // Test dealing
    state.dealHoleCards();
    for (Position pos : {Position::SB, Position::BB, Position::BTN}) {
        const auto& playerState = state.getPlayerState(pos);
        ASSERT_EQ(playerState.holeCards.size(), NUM_HOLE_CARDS);
    }
    
    // Test valid actions
    auto validActions = state.getValidActions();
    ASSERT_FALSE(validActions.empty());
    
    // Test action application
    Action action = Action::fold();
    bool roundOver = state.applyAction(action);
    
    // After SB folds, BB should be next to act
    if (!roundOver) {
        ASSERT_EQ(state.getCurrentPosition(), Position::BB);
    }
    
    // Apply more actions to advance to flop
    state.applyAction(Action::check());
    state.applyAction(Action::check());
    
    // Start flop
    state.startNextBettingRound();
    ASSERT_EQ(state.getBettingRound(), BettingRound::FLOP);
    ASSERT_EQ(state.getCommunityCards().size(), 3);
    
    // Test terminal state detection
    ASSERT_FALSE(state.isTerminal());
    
    // Test cloning
    auto stateClone = state.clone();
    ASSERT_EQ(stateClone->getBettingRound(), BettingRound::FLOP);
    ASSERT_EQ(stateClone->getCommunityCards().size(), 3);
}

int main() {
    std::cout << "Running game tests...\n";
    
    RUN_TEST(test_card);
    RUN_TEST(test_position);
    RUN_TEST(test_betting_round);
    RUN_TEST(test_action);
    RUN_TEST(test_action_history);
    RUN_TEST(test_game_state);
    
    std::cout << "All tests passed!\n";
    return 0;
}