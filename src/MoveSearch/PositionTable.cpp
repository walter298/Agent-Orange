module;

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_node_map.hpp>

module Chess.MoveSearch:PositionTable;

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

	struct PositionEqual {
		bool operator()(const Position& lhs, const Position& rhs) const {
			auto lhsTurnData = lhs.getTurnData();
			auto rhsTurnData = rhs.getTurnData();
			auto sidesEqual = std::ranges::equal(lhsTurnData.allies, rhsTurnData.allies) &&
				std::ranges::equal(lhsTurnData.enemies, rhsTurnData.enemies);
			auto jumpedPawnsEqual = (lhsTurnData.allies.doubleJumpedPawn == rhsTurnData.allies.doubleJumpedPawn) &&
				(lhsTurnData.enemies.doubleJumpedPawn == rhsTurnData.enemies.doubleJumpedPawn);
			return sidesEqual && jumpedPawnsEqual;
		}
	};

	struct DepthToRating {
		int depth = 0;
		Rating rating = 0;
	};

	using PositionMap = boost::unordered_node_map<Position, DepthToRating, PositionHasher, PositionEqual>;

	PositionMap positionMap;

	std::optional<Rating> getPositionRating(const Position& pos, int depth) {
		ProfilerLock l{ getGetPositionRatingProfiler() };

		auto depthToRatings = positionMap.find(pos);
		if (depthToRatings == positionMap.end()) {
			return std::nullopt;
		}

		auto& depthToRating = depthToRatings->second;
		if (depthToRating.depth >= depth) {
			return depthToRating.rating;
		}
		return std::nullopt;
	}

	void storePositionRating(const Position& pos, int depth, Rating rating) {
		ProfilerLock l{ getStorePositionRatingProfiler() };

		auto posIt = positionMap.find(pos);
		if (posIt == positionMap.end()) {
			positionMap.emplace(pos, DepthToRating{ depth, rating });
		} else {
			auto& depthMap = posIt->second;
			if (depth < depthMap.depth) {
				return;
			}
			depthMap.depth = depth;
			depthMap.rating = rating;
		}
	}
}