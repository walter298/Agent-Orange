module;

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>

module Chess.MoveSearch:PositionTable;

import Chess.ArenaAllocator;
import Chess.Profiler;
import :MoveHasher;

namespace chess {
	struct PositionHasher {
		size_t operator()(const Position& pos) const {
			auto [white, black] = pos.getColorSides();

			auto allPieces = white.calcAllLocations() | black.calcAllLocations();

			return static_cast<size_t>(allPieces);
		}
	};

	using PositionMap  = boost::unordered_node_map<Position, PositionEntry, PositionHasher>;

	PositionMap positionMap;

	std::optional<PositionEntryRef> getPositionEntry(const Position& pos) {
		ProfilerLock l{ getGetPositionEntryProfiler() };

		auto entry = positionMap.find(pos);
		if (entry == positionMap.end()) {
			return std::nullopt;
		}

		return std::cref(entry->second);
	}

	void storePositionEntry(const Position& pos, const PositionEntry& entry) {
		ProfilerLock l{ getStorePositionEntryProfiler() };

		auto posIt = positionMap.find(pos);
		if (posIt == positionMap.end()) {
			positionMap.emplace(pos, entry);
		} else {
			auto& storedEntry = posIt->second;
			if (entry.depth >= storedEntry.depth) {
				storedEntry = entry;
			} //sometimes we get shallower depths when we start a new game, ignore these
		}
	}
}