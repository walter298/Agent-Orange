module Chess.Position.RepetitionMap;

namespace chess {
	namespace repetition {
		std::vector<Position> pastPositions;

		void push(const Position& pos) {
			pastPositions.push_back(pos);
		}
		void pop() {
			pastPositions.pop_back();
		}
		int getPositionCount(const Position& pos) {
			return static_cast<int>(std::ranges::count(pastPositions, pos));
		}
		void clear() {
			pastPositions.clear();
		}
	}
}
