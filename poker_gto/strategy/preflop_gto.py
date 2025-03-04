import random
import json
import os
from deuces import Card, Evaluator, Deck

class PokerRange:
    def __init__(self, range_str=None, hands=None):
        self.range_str = range_str if range_str else ""
        self.pairs = []
        self.suited = []
        self.offsuit = []
        
        # Initialize directly from a list of hands
        if hands:
            for hand in hands:
                rank1, rank2, suited = hand
                if rank1 == rank2:
                    if rank1 not in self.pairs:
                        self.pairs.append(rank1)
                elif suited:
                    if (rank1, rank2) not in self.suited:
                        self.suited.append((rank1, rank2))
                else:
                    if (rank1, rank2) not in self.offsuit:
                        self.offsuit.append((rank1, rank2))
        elif range_str:  # Only parse if range_str is not empty.
            self.parse_range()

    def parse_range(self):
        """Parse a range string into pairs, suited, and offsuit hands."""
        parts = self.range_str.split(',')
        for part in parts:
            part = part.strip()
            if not part:  # Skip empty parts.
                continue
            if '+' in part:
                if part[0] == part[1]:
                    # For pairs like "88+"
                    start = PokerRange.rank_to_num(part[0])
                    self.pairs.extend(range(start, 15))
                elif part[1] == 's':
                    # Suited range like "ATs+"
                    rank1 = PokerRange.rank_to_num(part[0])
                    rank2 = PokerRange.rank_to_num(part[2])
                    # This is a simplified expansion; you might refine it later.
                    self.suited.extend([(r1, r2) for r1 in range(rank1, 15) for r2 in range(rank2, r1)])
                elif part[1] == 'o':
                    # Offsuit range like "AQo+"
                    rank1 = PokerRange.rank_to_num(part[0])
                    rank2 = PokerRange.rank_to_num(part[2])
                    self.offsuit.extend([(r1, r2) for r1 in range(rank1, 15) for r2 in range(rank2, r1)])
            else:
                # No '+' sign in the part.
                if len(part) == 2 and part[0] == part[1]:
                    self.pairs.append(PokerRange.rank_to_num(part[0]))
                elif len(part) >= 3 and part[2] == 's':
                    self.suited.append((PokerRange.rank_to_num(part[0]), PokerRange.rank_to_num(part[1])))
                elif len(part) >= 3 and part[2] == 'o':
                    self.offsuit.append((PokerRange.rank_to_num(part[0]), PokerRange.rank_to_num(part[1])))

    @staticmethod
    def rank_to_num(rank):
        """Convert card rank to numeric value (2-14)."""
        ranks = {'2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7, '8': 8, '9': 9, 'T': 10, 'J': 11, 'Q': 12, 'K': 13, 'A': 14}
        return ranks[rank]

    @staticmethod
    def num_to_rank(num):
        """Convert numeric rank back to string."""
        ranks = {2: '2', 3: '3', 4: '4', 5: '5', 6: '6', 7: '7', 8: '8', 9: '9', 10: 'T', 11: 'J', 12: 'Q', 13: 'K', 14: 'A'}
        return ranks[num]

    def is_in_range(self, hand):
        """
        Check if a hand (tuple: (rank1, rank2, suited)) is in this range.
        For pairs, we check the numeric value.
        For non-pairs, we look in suited or offsuit lists.
        """
        rank1, rank2, suited = hand
        if rank1 == rank2:
            return rank1 in self.pairs
        elif suited:
            return (rank1, rank2) in self.suited
        else:
            return (rank1, rank2) in self.offsuit
            
    def to_dict(self):
        """Convert the range to a dictionary for JSON serialization."""
        return {
            'pairs': self.pairs,
            'suited': self.suited,
            'offsuit': self.offsuit
        }
        
    @classmethod
    def from_dict(cls, range_dict):
        """Create a PokerRange object from a dictionary."""
        poker_range = cls()
        poker_range.pairs = range_dict.get('pairs', [])
        poker_range.suited = range_dict.get('suited', [])
        poker_range.offsuit = range_dict.get('offsuit', [])
        return poker_range
        
    def get_all_hands(self):
        """Get all hands in this range as a list of tuples (rank1, rank2, suited)."""
        hands = []
        
        # Add pairs
        for rank in self.pairs:
            hands.append((rank, rank, False))  # Suited doesn't matter for pairs
            
        # Add suited hands
        for rank1, rank2 in self.suited:
            hands.append((rank1, rank2, True))
            
        # Add offsuit hands
        for rank1, rank2 in self.offsuit:
            hands.append((rank1, rank2, False))
            
        return hands
        
    def get_hand_percentage(self):
        """Calculate what percentage of all possible hands this range represents."""
        total_hands = len(self.get_all_hands())
        # Total possible hands: 13 pairs + 13*12/2 suited + 13*12/2 offsuit = 169
        return (total_hands / 169) * 100

class GTOPreflopStrategy:
    def __init__(self, stack_size=100):
        self.stack_size = stack_size  # Starting stack in big blinds.
        self.positions = ['UTG', 'UTG+1', 'MP', 'MP+1', 'CO', 'BTN', 'SB', 'BB']
        # Default open-raising ranges per position (will be optimized later)
        self.open_raise_ranges = {
            'UTG': PokerRange("88+, ATs+, KJs+, QJs, JTs, T9s, 98s, AQo+, KQo"),
            'UTG+1': PokerRange("77+, A9s+, KTs+, QTs+, J9s+, T8s+, 98s, AJo+, KJo+"),
            'MP': PokerRange("66+, A8s+, K9s+, Q9s+, J9s+, T8s+, 98s, 87s, ATo+, KJo+, QJo"),
            'MP+1': PokerRange("55+, A7s+, K8s+, Q8s+, J8s+, T7s+, 97s+, 86s+, 76s, A9o+, KTo+, QTo+, JTo"),
            'CO': PokerRange("44+, A2s+, K6s+, Q7s+, J7s+, T7s+, 97s+, 86s+, 75s+, 65s, A8o+, K9o+, Q9o+, J9o+, T9o"),
            'BTN': PokerRange("22+, A2s+, K2s+, Q5s+, J6s+, T6s+, 95s+, 85s+, 74s+, 64s+, 54s, A2o+, K8o+, Q8o+, J8o+, T8o+, 98o"),
            'SB': PokerRange("22+, A2s+, K2s+, Q5s+, J6s+, T6s+, 95s+, 85s+, 74s+, 64s+, 54s, A2o+, K8o+, Q8o+, J8o+, T8o+, 98o"),
            'BB': None  # The big blind does not open-raise.
        }
        self.raise_size = min(2.5, self.stack_size)  # Open-raise size in BB.
        self.three_bet_size = min(7.5, self.stack_size)  # 3-bet size in BB.
        # Initialize the deuces evaluator
        self.evaluator = Evaluator()
        
        # Store for optimal ranges
        self.optimal_ranges = {}
        self.hand_ev_cache = {}  # Cache for hand EV calculations

    def get_open_raise_range(self, position):
        """Return the open-raising range for the given position."""
        # First check if we have an optimal range
        if position in self.optimal_ranges:
            return self.optimal_ranges[position]
        # Fall back to default range
        return self.open_raise_ranges.get(position, None)

    def calculate_equity(self, hand, opponent_range, num_simulations=1000):
        """
        Calculate the equity of a hand against an opponent's range using Monte Carlo simulation and deuces.
        
        Parameters:
        hand: tuple (rank1, rank2, suited) with rank1 >= rank2
        opponent_range: PokerRange object representing opponent's possible hands
        num_simulations: number of simulations to run
        
        Returns:
        float: Equity (probability of winning)
        """
        if not opponent_range or not opponent_range.pairs and not opponent_range.suited and not opponent_range.offsuit:
            return 0.5  # Default equity if no opponent range is provided
        
        wins = 0
        ties = 0
        valid_simulations = 0
        
        # Convert our hand tuple to deuces cards
        my_hand = self._hand_tuple_to_deuces_cards(hand)
        
        while valid_simulations < num_simulations:
            # Create a new deck for each simulation
            deck = Deck()
            
            # Remove our cards from the deck
            for card in my_hand:
                deck.cards.remove(card)
            
            # Generate a random opponent hand from their range
            opponent_hand_tuple = self._generate_hand_from_range(opponent_range)
            opponent_hand = self._hand_tuple_to_deuces_cards(opponent_hand_tuple)
            
            # Check if any of the opponent's cards are already in our hand
            if any(card in my_hand for card in opponent_hand):
                continue  # Skip this iteration and try again
            
            # Remove opponent cards from the deck
            for card in opponent_hand:
                if card in deck.cards:
                    deck.cards.remove(card)
            
            # Draw the board (5 community cards)
            board = deck.draw(5)
            
            # Evaluate both hands to determine the winner
            my_score = self.evaluator.evaluate(board, my_hand)
            opponent_score = self.evaluator.evaluate(board, opponent_hand)
            
            # In deuces, lower score is better
            if my_score < opponent_score:
                wins += 1
            elif my_score == opponent_score:
                ties += 0.5  # Count ties as half a win
            
            valid_simulations += 1
        
        # Calculate equity
        return (wins + ties) / valid_simulations if valid_simulations > 0 else 0.5

    def _hand_tuple_to_deuces_cards(self, hand):
        """
        Convert a hand tuple (rank1, rank2, suited) to a list of deuces Card objects.
        """
        rank1, rank2, suited = hand
        
        # Convert ranks to string representation for deuces
        r1 = PokerRange.num_to_rank(rank1)
        r2 = PokerRange.num_to_rank(rank2)
        
        # Choose suits based on whether the hand is suited
        if suited and rank1 != rank2:
            # Both cards have the same suit (e.g., hearts)
            return [Card.new(r1 + 'h'), Card.new(r2 + 'h')]
        else:
            # Different suits for each card
            return [Card.new(r1 + 'h'), Card.new(r2 + 's')]

    def _generate_hand_from_range(self, poker_range):
        """
        Generate a random hand from the given poker range.
        """
        # Compile a list of all possible hands in the range
        all_hands = poker_range.get_all_hands()
        
        # Return a random hand from the range
        if all_hands:
            return random.choice(all_hands)
        else:
            # Return a random hand if the range is empty (shouldn't happen)
            return self.generate_random_hand()

    def calculate_open_raise_ev(self, position, hand, num_simulations=500):
        """
        Calculate the EV of opening with a specific hand from a given position.
        
        Parameters:
        position: Position at the table
        hand: tuple (rank1, rank2, suited)
        num_simulations: number of Monte Carlo simulations
        
        Returns:
        float: Expected value of raising
        """
        # Check cache first
        cache_key = (position, hand[0], hand[1], hand[2])
        if cache_key in self.hand_ev_cache:
            return self.hand_ev_cache[cache_key]
            
        # Initialize variables
        initial_pot = 1.5  # SB + BB
        raise_size = self.raise_size
        
        # Estimate response probabilities from each position
        # This is simplified - in reality, you'd need more complex modeling
        fold_prob = 0.7  # Probability that everyone folds
        call_prob = 0.2  # Probability that someone calls
        reraise_prob = 0.1  # Probability that someone reraises
        
        # Calculate EV for each scenario
        ev_all_fold = fold_prob * initial_pot  # We win the blinds
        
        # For call scenarios, calculate average equity against calling ranges
        # (This is a simplification - in reality, you'd model each player separately)
        equity_vs_call = 0
        for pos in self.positions:
            if pos != position and pos != 'BB':  # Exclude ourselves and BB (which doesn't raise)
                # Assume typical calling range - this should be refined
                calling_range = PokerRange("TT+, AQs+, AKo")  # Example calling range
                equity = self.calculate_equity(hand, calling_range, num_simulations // 5)
                equity_vs_call += equity
        
        if sum(1 for pos in self.positions if pos != position and pos != 'BB') > 0:
            equity_vs_call /= sum(1 for pos in self.positions if pos != position and pos != 'BB')
        
        # Calculate EV when called
        pot_after_call = initial_pot + 2 * raise_size
        ev_called = call_prob * (equity_vs_call * pot_after_call - (1 - equity_vs_call) * raise_size)
        
        # Calculate EV when reraised (simplified)
        reraise_size = self.three_bet_size
        pot_after_reraise = initial_pot + raise_size + reraise_size
        equity_vs_reraise = 0.4  # Simplified estimate
        
        # Assume we call the 3-bet 30% of the time
        fold_to_3bet_prob = 0.7
        call_3bet_prob = 0.3
        
        ev_reraised = reraise_prob * (
            fold_to_3bet_prob * (-raise_size) +  # We fold to the 3-bet
            call_3bet_prob * (equity_vs_reraise * (pot_after_reraise + reraise_size) - 
                             (1 - equity_vs_reraise) * reraise_size)  # We call the 3-bet
        )
        
        # Total EV of raising
        total_ev = ev_all_fold + ev_called + ev_reraised
        
        # Cache the result
        self.hand_ev_cache[cache_key] = total_ev
        
        return total_ev

    def generate_optimal_ranges(self, threshold_percentage=20, num_simulations=100):
        """
        Generate optimal ranges for each position based on EV calculations.
        
        Parameters:
        threshold_percentage: Percentage of hands to include in the range
        num_simulations: Number of Monte Carlo simulations for equity calculations
        """
        print(f"Generating optimal ranges (using {threshold_percentage}% of hands)...")
        
        # Generate all possible hands
        all_hands = []
        
        # Pairs
        for rank in range(2, 15):
            all_hands.append((rank, rank, False))
            
        # Suited hands (excluding pairs)
        for rank1 in range(3, 15):
            for rank2 in range(2, rank1):
                all_hands.append((rank1, rank2, True))
                
        # Offsuit hands (excluding pairs)
        for rank1 in range(3, 15):
            for rank2 in range(2, rank1):
                all_hands.append((rank1, rank2, False))
        
        # For each position, calculate the EV of raising with each hand
        for position in self.positions[:-1]:  # Exclude BB
            print(f"Processing position: {position}")
            position_evs = []
            
            for hand in all_hands:
                ev = self.calculate_open_raise_ev(position, hand, num_simulations)
                position_evs.append((hand, ev))
                
            # Sort hands by EV
            position_evs.sort(key=lambda x: x[1], reverse=True)
            
            # Determine how many hands to include
            num_hands = int(len(all_hands) * threshold_percentage / 100)
            
            # Create a range with the top hands
            top_hands = [hand for hand, _ in position_evs[:num_hands]]
            self.optimal_ranges[position] = PokerRange(hands=top_hands)
            
            print(f"  Created optimal range for {position} with {len(top_hands)} hands ({self.optimal_ranges[position].get_hand_percentage():.1f}%)")
            
            # Show some example hands
            print("  Top 5 hands:")
            for hand, ev in position_evs[:5]:
                print(f"    {self.hand_to_string(hand)}: EV = {ev:.3f}")
            print("  Last 5 included hands:")
            for hand, ev in position_evs[num_hands-5:num_hands]:
                print(f"    {self.hand_to_string(hand)}: EV = {ev:.3f}")
        
        print("Optimal ranges generated!")

    def save_optimal_ranges(self, filename="optimal_ranges.json"):
        """Save the optimal ranges to a JSON file."""
        if not self.optimal_ranges:
            print("No optimal ranges to save. Run generate_optimal_ranges() first.")
            return
            
        ranges_dict = {
            'stack_size': self.stack_size,
            'ranges': {pos: range_obj.to_dict() for pos, range_obj in self.optimal_ranges.items()}
        }
        
        with open(filename, 'w') as f:
            json.dump(ranges_dict, f, indent=2)
            
        print(f"Optimal ranges saved to {filename}")

    def load_optimal_ranges(self, filename="optimal_ranges.json"):
        """Load optimal ranges from a JSON file."""
        if not os.path.exists(filename):
            print(f"File {filename} not found. No ranges loaded.")
            return False
            
        try:
            with open(filename, 'r') as f:
                ranges_dict = json.load(f)
                
            # Check if the stack size matches
            if ranges_dict.get('stack_size', 0) != self.stack_size:
                print(f"Warning: Loaded ranges were generated for stack size {ranges_dict.get('stack_size')} BB, but current stack size is {self.stack_size} BB.")
                
            # Load ranges
            self.optimal_ranges = {
                pos: PokerRange.from_dict(range_dict) 
                for pos, range_dict in ranges_dict.get('ranges', {}).items()
            }
            
            print(f"Loaded optimal ranges from {filename}")
            
            # Show range statistics
            for pos, range_obj in self.optimal_ranges.items():
                print(f"  {pos}: {range_obj.get_hand_percentage():.1f}% of hands")
                
            return True
            
        except Exception as e:
            print(f"Error loading ranges: {e}")
            return False

    def get_action(self, position, hand, game_state):
        """
        Determine an action ('raise', 'call', or 'fold') based on position, hand, and game state.
        
        hand: tuple (rank1, rank2, suited) with rank1 >= rank2.
        game_state: dict, optionally including 'raiser_position' if facing a raise.
        """
        if 'raiser_position' not in game_state or game_state['raiser_position'] is None:
            # No previous raise: check if hand is in open-raise range.
            open_range = self.get_open_raise_range(position)
            if open_range and open_range.is_in_range(hand):
                return 'raise'
            else:
                return 'fold'
        else:
            # Facing a raise.
            raiser_position = game_state['raiser_position']
            opponent_range = self.get_open_raise_range(raiser_position)
            if opponent_range is None:
                return 'fold'
            
            equity = self.calculate_equity(hand, opponent_range)
            
            initial_pot = 1.5  # Assume small blind + big blind.
            pot_after_raise = initial_pot + self.raise_size
            to_call = self.raise_size
            pot_after_call = min(pot_after_raise + to_call, self.stack_size * 2)
            ev_call = equity * pot_after_call - (1 - equity) * to_call
            
            # Simplified EV for 3-betting.
            fold_prob, call_prob, four_bet_prob = 0.4, 0.5, 0.1
            pot_after_3bet_call = min(pot_after_raise + self.three_bet_size * 2, self.stack_size * 2)
            ev_3bet = (fold_prob * pot_after_raise) + \
                      (call_prob * (equity * pot_after_3bet_call - (1 - equity) * self.three_bet_size)) + \
                      (four_bet_prob * (-self.three_bet_size))
            ev_fold = 0

            if ev_3bet > max(ev_call, ev_fold):
                return 'raise'
            elif ev_call > ev_fold:
                return 'call'
            else:
                return 'fold'

    def generate_random_hand(self):
        """Generate a random hand as a tuple: (rank1, rank2, suited)."""
        rank1 = random.randint(2, 14)
        rank2 = random.randint(2, 14)
        if rank1 < rank2:
            rank1, rank2 = rank2, rank1
        suited = random.choice([True, False])
        return (rank1, rank2, suited)

    def hand_to_string(self, hand):
        """Convert a hand tuple to a string representation."""
        rank1, rank2, suited = hand
        r1 = PokerRange.num_to_rank(rank1)
        r2 = PokerRange.num_to_rank(rank2)
        if rank1 == rank2:
            return f"{r1}{r2}"
        else:
            return f"{r1}{r2}{'s' if suited else 'o'}"

    def calculate_gto_ranges(self, num_simulations=1000):
        """Run simulations to approximate GTO open-raise and facing-raise ranges."""
        print(f"\nCalculating GTO Preflop Ranges for Stack Size: {self.stack_size} BB")
        print("===========================================")

        # Open-raise ranges.
        print("\nOpen-Raising Ranges:")
        for pos in self.positions[:-1]:  # Exclude BB.
            actions = {'raise': [], 'fold': []}
            for _ in range(num_simulations):
                hand = self.generate_random_hand()
                action = self.get_action(pos, hand, {})
                actions[action].append(hand)
            raise_count = len(actions['raise'])
            if raise_count > 0:
                print(f"{pos}: {raise_count / num_simulations * 100:.2f}% of hands")
                sample_hands = [self.hand_to_string(h) for h in actions['raise'][:min(5, raise_count)]]
                print(f"  Example hands: {', '.join(sample_hands)}")
            else:
                print(f"{pos}: No raising hands identified")

        # Facing raise ranges.
        print("\nFacing Raise Ranges:")
        for pos in self.positions:
            for raiser_pos in self.positions[:self.positions.index(pos)]:
                actions = {'raise': [], 'call': [], 'fold': []}
                for _ in range(num_simulations):
                    hand = self.generate_random_hand()
                    game_state = {'raiser_position': raiser_pos}
                    action = self.get_action(pos, hand, game_state)
                    actions[action].append(hand)
                print(f"\n{pos} facing raise from {raiser_pos}:")
                for action in actions:
                    count = len(actions[action])
                    if count > 0:
                        percentage = count / num_simulations * 100
                        print(f"  {action.capitalize()}: {percentage:.2f}%")
                        sample_hands = [self.hand_to_string(h) for h in actions[action][:min(5, count)]]
                        print(f"    Example hands: {', '.join(sample_hands)}")

    def export_ranges_to_csv(self, filename="optimal_ranges.csv"):
        """Export the optimal ranges to a CSV file for easy viewing."""
        if not self.optimal_ranges:
            print("No optimal ranges to export. Run generate_optimal_ranges() first.")
            return
            
        with open(filename, 'w') as f:
            # Write header
            f.write("Position,Hand,Type\n")
            
            # Write data for each position
            for position, range_obj in self.optimal_ranges.items():
                # Write pairs
                for rank in range_obj.pairs:
                    hand_str = self.hand_to_string((rank, rank, False))
                    f.write(f"{position},{hand_str},Pair\n")
                    
                # Write suited hands
                for rank1, rank2 in range_obj.suited:
                    hand_str = self.hand_to_string((rank1, rank2, True))
                    f.write(f"{position},{hand_str},Suited\n")
                    
                # Write offsuit hands
                for rank1, rank2 in range_obj.offsuit:
                    hand_str = self.hand_to_string((rank1, rank2, False))
                    f.write(f"{position},{hand_str},Offsuit\n")
                    
        print(f"Ranges exported to {filename}")

def hand_to_tuple(hand_str):
    """
    Convert a hand string to a tuple (rank1, rank2, suited).
    
    Accepted formats:
      - 3-character format: "AKs" means Ace, King suited.
      - 4-character format: "AsAc" means Ace of spades and Ace of clubs.
    """
    if len(hand_str) == 3:
        # Format like "AKs" or "AQo"
        rank1 = hand_str[0]
        rank2 = hand_str[1]
        suited = hand_str[2] == 's'
    elif len(hand_str) == 4:
        # Format like "AsAc": first two characters are card1, last two are card2.
        card1 = hand_str[:2]  # e.g., "As"
        card2 = hand_str[2:]  # e.g., "Ac"
        rank1 = card1[0]      # 'A'
        rank2 = card2[0]      # 'A'
        suited = card1[1] == card2[1]  # Compare suits
    else:
        raise ValueError("Hand string must be either 3 or 4 characters long")
    
    rank1_num = PokerRange.rank_to_num(rank1)
    rank2_num = PokerRange.rank_to_num(rank2)
    # Ensure that rank1_num >= rank2_num
    if rank1_num < rank2_num:
        rank1_num, rank2_num = rank2_num, rank1_num
    return (rank1_num, rank2_num, suited)

# Example usage when running this module directly.
if __name__ == "__main__":
    # Get stack size from user input or default to 100 BB.
    stack_size = float(input("Enter starting stack size in BB (e.g., 100): ") or 100)
    strategy = GTOPreflopStrategy(stack_size=stack_size)
    
    # Try to load existing optimal ranges
    if not strategy.load_optimal_ranges():
        print("\nNo existing ranges found. Do you want to generate new optimal ranges?")
        generate_new = input("Generate new ranges? (y/n): ").lower() == 'y'
        
        if generate_new:
            print("\nGenerating new optimal ranges. This may take some time...")
            percentage = float(input("What percentage of hands would you like in each range? (default: 20%): ") or 20)
            simulations = int(input("How many simulations per hand? (default: 100, more = more accurate but slower): ") or 100)
            
            strategy.generate_optimal_ranges(threshold_percentage=percentage, num_simulations=simulations)
            strategy.save_optimal_ranges()
            
            # Export to CSV for easy viewing
            strategy.export_ranges_to_csv()
    
    print("\nIndividual Action Tests:")
    hand = hand_to_tuple('AsAc')  # AA in 4-character format.
    action = strategy.get_action('UTG', hand, {})
    print(f"UTG with AA: {action}")

    hand = hand_to_tuple('AsKs')  # AK suited.
    game_state = {'raiser_position': 'MP'}
    action = strategy.get_action('BTN', hand, game_state)
    print(f"BTN facing raise from MP with AKs: {action}")

    # Run simulations with optimized ranges
    strategy.calculate_gto_ranges(num_simulations=1000)