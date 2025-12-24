module Chess.Position.RepetitionMap;

import Chess.Assert;

namespace chess {
	namespace repetition {
		std::vector<Position> pastPositions;

		void push(const Position& pos) {
			//std::println("Pushing onto {} positions", pastPositions.size());
			pastPositions.push_back(pos);
		}
		void pop() {
			zAssert(!pastPositions.empty());
			pastPositions.pop_back();
		}
		int getPositionCount(const Position& pos) {
			return static_cast<int>(std::ranges::count(pastPositions, pos));
		}
		void clear() {
			pastPositions.clear();
		}
		int getTotalPositionCount() {
			return static_cast<int>(pastPositions.size());
		}
	}
}
