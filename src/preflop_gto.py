import random

class PokerRange:
    def __init__(self, range_str):
        self.range_str = range_str
        self.pairs = []
        self.suited = []
        self.offsuit = []
        self.parse_range()

    def parse_range(self):
        """Parse a range string into pairs, suited, and offsuit hands."""
        parts = self.range_str.split(',')
        for part in parts:
            part = part.strip()
            if '+' in part:
                if part[0].isdigit() or part[0] in 'TJQKA':
                    # Pairs like "88+"
                    start = self.rank_to_num(part[:2])
                    self.pairs.extend(range(start, 15))
                elif part[1] == 's':
                    # Suited like "ATs+"
                    rank1, rank2 = self.rank_to_num(part[0]), self.rank_to_num(part[2])
                    self.suited.extend([(r1, r2) for r1 in range(rank1, 15) for r2 in range(rank2, r1)])
                elif part[1] == 'o':
                    # Offsuit like "AQo+"
                    rank1, rank2 = self.rank_to_num(part[0]), self.rank_to_num(part[2])
                    self.offsuit.extend([(r1, r2) for r1 in range(rank1, 15) for r2 in range(rank2, r1)])
            else:
                if len(part) == 2 and part[0] == part[1]:
                    self.pairs.append(self.rank_to_num(part[0]))
                elif part[2] == 's':
                    self.suited.append((self.rank_to_num(part[0]), self.rank_to_num(part[1])))
                elif part[2] == 'o':
                    self.offsuit.append((self.rank_to_num(part[0]), self.rank_to_num(part[1])))

    def rank_to_num(self, rank):
        """Convert card rank to numeric value (2-14)."""
        ranks = {'2': 2, '3': 3, '4': 4, '5': 5, '6': 6, '7': 7, '8': 8, '9': 9, 'T': 10, 'J': 11, 'Q': 12, 'K': 13, 'A': 14}
        return ranks[rank]

    def num_to_rank(self, num):
        """Convert numeric rank back to string."""
        ranks = {2: '2', 3: '3', 4: '4', 5: '5', 6: '6', 7: '7', 8: '8', 9: '9', 10: 'T', 11: 'J', 12: 'Q', 13: 'K', 14: 'A'}
        return ranks[num]

    def is_in_range(self, hand):
        """Check if a hand (rank1, rank2, suited) is in this range."""
        rank1, rank2, suited = hand
        if rank1 == rank2:
            return rank1 in self.pairs
        elif suited:
            return (rank1, rank2) in self.suited
        else:
            return (rank1, rank2) in self.offsuit

class GTOPreflopStrategy:
    def __init__(self, stack_size=100):
        self.stack_size = stack_size  # Starting stack in BB
        self.positions = ['UTG', 'UTG+1', 'MP', 'MP+1', 'CO', 'BTN', 'SB', 'BB']
        # Initial open-raising ranges (used as a starting point)
        self.open_raise_ranges = {
            'UTG': PokerRange("88+, ATs+, KJs+, QJs, JTs, T9s, 98s, AQo+, KQo"),
            'UTG+1': PokerRange("77+, A9s+, KTs+, QTs+, J9s+, T8s+, 98s, AJo+, KJo+"),
            'MP': PokerRange("66+, A8s+, K9s+, Q9s+, J9s+, T8s+, 98s, 87s, ATo+, KJo+, QJo"),
            'MP+1': PokerRange("55+, A7s+, K8s+, Q8s+, J8s+, T7s+, 97s+, 86s+, 76s, A9o+, KTo+, QTo+, JTo"),
            'CO': PokerRange("44+, A2s+, K6s+, Q7s+, J7s+, T7s+, 97s+, 86s+, 75s+, 65s, A8o+, K9o+, Q9o+, J9o+, T9o"),
            'BTN': PokerRange("22+, A2s+, K2s+, Q5s+, J6s+, T6s+, 95s+, 85s+, 74s+, 64s+, 54s, A2o+, K8o+, Q8o+, J8o+, T8o+, 98o"),
            'SB': PokerRange("22+, A2s+, K2s+, Q5s+, J6s+, T6s+, 95s+, 85s+, 74s+, 64s+, 54s, A2o+, K8o+, Q8o+, J8o+, T8o+, 98o"),
            'BB': None  # BB doesn't open-raise
        }
        self.raise_size = min(2.5, self.stack_size)  # Raise size in BB, capped by stack
        self.three_bet_size = min(7.5, self.stack_size)  # 3-bet size in BB, capped by stack

    def get_open_raise_range(self, position):
        """Return the open-raising range for the given position."""
        return self.open_raise_ranges.get(position, None)

    def calculate_equity(self, hand, opponent_range):
        """
        Placeholder for equity calculation. Replace with a real equity calculator.
        Returns a random value for demonstration.
        """
        return random.uniform(0, 1)

    def get_action(self, position, hand, game_state):
        """
        Determine action (raise, call, fold) based on position, hand, and game state.
        hand: tuple (rank1, rank2, suited) where rank1 > rank2.
        game_state: dict with 'raiser_position' if there is a raise.
        """
        if 'raiser_position' not in game_state or game_state['raiser_position'] is None:
            # No raise before, check open-raise range
            open_range = self.get_open_raise_range(position)
            if open_range and open_range.is_in_range(hand):
                return 'raise'
            else:
                return 'fold'
        else:
            # Facing a raise
            raiser_position = game_state['raiser_position']
            opponent_range = self.get_open_raise_range(raiser_position)
            if opponent_range is None:
                return 'fold'

            equity = self.calculate_equity(hand, opponent_range)

            # Pot and bet sizes adjusted for stack size
            initial_pot = 1.5  # SB + BB
            pot_after_raise = initial_pot + self.raise_size
            to_call = self.raise_size
            pot_after_call = min(pot_after_raise + to_call, self.stack_size * 2)  # Cap by effective stacks

            # EV for calling
            ev_call = equity * pot_after_call - (1 - equity) * to_call

            # EV for 3-betting (simplified opponent response model)
            fold_prob, call_prob, four_bet_prob = 0.4, 0.5, 0.1
            pot_after_3bet_call = min(pot_after_raise + self.three_bet_size * 2, self.stack_size * 2)
            ev_3bet = (fold_prob * pot_after_raise) + \
                      (call_prob * (equity * pot_after_3bet_call - (1 - equity) * self.three_bet_size)) + \
                      (four_bet_prob * (-self.three_bet_size))

            # EV for folding
            ev_fold = 0

            if ev_3bet > max(ev_call, ev_fold):
                return 'raise'
            elif ev_call > ev_fold:
                return 'call'
            else:
                return 'fold'

    def generate_random_hand(self):
        """Generate a random poker hand as (rank1, rank2, suited)."""
        rank1 = random.randint(2, 14)
        rank2 = random.randint(2, 14)
        if rank1 < rank2:
            rank1, rank2 = rank2, rank1
        suited = random.choice([True, False])
        return (rank1, rank2, suited)

    def hand_to_string(self, hand):
        """Convert hand tuple to string representation."""
        rank1, rank2, suited = hand
        r1 = PokerRange('').num_to_rank(rank1)
        r2 = PokerRange('').num_to_rank(rank2)
        return f"{r1}{r2}{'s' if suited else 'o' if rank1 != rank2 else ''}"

    def calculate_gto_ranges(self, num_simulations=1000):
        """
        Calculate and display approximate GTO open-raise and facing-raise ranges.
        """
        print(f"\nCalculating GTO Preflop Ranges for Stack Size: {self.stack_size} BB")
        print("===========================================")

        # Open-raise ranges
        print("\nOpen-Raising Ranges:")
        for pos in self.positions[:-1]:  # Exclude BB
            actions = {'raise': [], 'fold': []}
            for _ in range(num_simulations):
                hand = self.generate_random_hand()
                action = self.get_action(pos, hand, {})
                actions[action].append(hand)
            raise_count = len(actions['raise'])
            if raise_count > 0:
                print(f"{pos}: {raise_count / num_simulations * 100:.2f}% of hands")
                # Sample a few hands for illustration
                sample_hands = [self.hand_to_string(h) for h in actions['raise'][:min(5, raise_count)]]
                print(f"  Example hands: {', '.join(sample_hands)}")
            else:
                print(f"{pos}: No raising hands identified")

        # Facing raise ranges
        print("\nFacing Raise Ranges:")
        for pos in self.positions:
            for raiser_pos in self.positions[:self.positions.index(pos)]:  # Positions before current pos
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

# Helper function to convert hand string to tuple
def hand_to_tuple(hand_str):
    """Convert hand string (e.g., 'AsKs') to (rank1, rank2, suited)."""
    rank1, rank2 = hand_str[0], hand_str[1]
    suited = hand_str[2] == 's'
    rank1_num = PokerRange('').rank_to_num(rank1)
    rank2_num = PokerRange('').rank_to_num(rank2)
    if rank1_num < rank2_num:
        rank1_num, rank2_num = rank2_num, rank1_num
    return (rank1_num, rank2_num, suited)

