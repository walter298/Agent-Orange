module;

#include <boost/unordered/unordered_flat_map.hpp>

module Chess.Position.RepetitionMap;

import Chess.Assert;

namespace chess {
	void RepetitionMap::push(const Position& pos) {
		m_positionCounts[pos.hash()]++;
	}
	void RepetitionMap::pop(const Position& pos) {
		zAssert(m_positionCounts.contains(pos.hash()));
		auto& posCount = m_positionCounts.at(pos.hash());
		zAssert(posCount > 0);
		posCount--;
	}
	int RepetitionMap::getPositionCount(const Position& pos) const {
		auto positionIt = m_positionCounts.find(pos.hash());
		if (positionIt == m_positionCounts.end()) {
			return 0;
		}
		return positionIt->second;
	}
	void RepetitionMap::clear() {
		m_positionCounts.clear();
	}
	int RepetitionMap::getTotalPositionCount() const {
		return std::ranges::fold_left(m_positionCounts, 0, [](auto acc, const auto& kv) {
			return acc + kv.second;
		});
	}
}
