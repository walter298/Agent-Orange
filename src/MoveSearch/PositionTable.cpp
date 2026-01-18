module;

//#include <boost/unordered/unordered_flat_map.hpp>
//#include <boost/unordered/unordered_node_map.hpp>
#include <boost/unordered/concurrent_flat_map.hpp>

module Chess.MoveSearch:PositionTable;

import :MoveHasher;

namespace chess {
	using PositionMap = boost::concurrent_flat_map<std::uint64_t, PositionEntry>;

	PositionMap positionMap;

	std::optional<PositionEntry> getPositionEntry(const Position& pos, SafeUnsigned<std::uint8_t> depth) {
		PositionEntry ret;
		auto found = positionMap.visit(pos.hash(), [&](const auto& kv) {
			ret = kv.second;
		});
		if (!found || ret.depth < depth) {
			return std::nullopt;
		}
		return ret;
	}

	void storePositionEntry(const Position& pos, const PositionEntry& entry) {
		positionMap.emplace_or_visit(pos.hash(), entry, [&](auto& storedKV) {
			if (entry.depth >= storedKV.second.depth) {
				storedKV.second = entry;
			} //sometimes we get shallower depths when we start a new game, ignore these
		});
	}

	void clearTranspositionTable() {
		positionMap.clear();
	}
}