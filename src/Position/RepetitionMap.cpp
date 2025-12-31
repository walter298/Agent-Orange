module;

#include <boost/unordered/unordered_flat_map.hpp>

module Chess.Position.RepetitionMap;

import Chess.Assert;

namespace chess {
	namespace repetition {
		boost::unordered_flat_map<std::uint64_t, int> pastPositions;

		void push(const Position& pos) {
			pastPositions[pos.hash()]++;
		}
		void pop(const Position& pos) {
			zAssert(pastPositions.contains(pos.hash()));
			auto& posCount = pastPositions.at(pos.hash());
			zAssert(posCount > 0);
			posCount--;
		}
		int getPositionCount(const Position& pos) {
			return pastPositions.at(pos.hash());
		}
		void clear() {
			pastPositions.clear();
		}
		int getTotalPositionCount() {
			return std::ranges::fold_left(pastPositions, 0, [](auto acc, const auto& kv) {
				return acc + kv.second;
			});
		}
	}
}
