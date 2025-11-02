module;

#include <boost/unordered/unordered_flat_map.hpp>

module Chess.MoveSearch:MoveHistory;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import :MoveHasher;

namespace chess {
	boost::unordered_flat_map<Move, Rating, MoveHasher> historyTable;

	void updateHistoryScore(const Move& move, int distFromRoot) {
		historyTable[move] += static_cast<Rating>(1ull << distFromRoot);
	}

	Rating getHistoryRating(const Move& move) {
		auto ratingIt = historyTable.find(move);
		if (ratingIt == historyTable.end()) {
			return 0rt;
		} else {
			return ratingIt->second;
		}
	}
}