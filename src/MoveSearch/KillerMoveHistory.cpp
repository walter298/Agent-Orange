module;

#include <boost/unordered/unordered_flat_map.hpp>

module Chess.MoveSearch:KillerMoveHistory;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import :MoveHasher;

namespace chess {
	boost::unordered_flat_map<Move, Rating, MoveHasher> historyTable;

	void updateHistoryScore(const Move& move, int distFromRoot) {
		if (distFromRoot > 63) {
			distFromRoot = 63;
		}
		auto delta = static_cast<Rating>(1 << distFromRoot);
		auto& rating = historyTable[move];
		if (std::numeric_limits<Rating>::max() - rating > delta) {
			rating += delta;
		}
	}

	Rating getHistoryRating(const Move& move) {
		auto ratingIt = historyTable.find(move);
		if (ratingIt == historyTable.end()) {
			return 0_rt;
		} else {
			return ratingIt->second;
		}
	}
}