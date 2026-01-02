module;

#include <boost/unordered/concurrent_flat_map.hpp>

module Chess.MoveSearch:KillerMoveHistory;

import Chess.Assert;
import Chess.Evaluation;
import Chess.MoveGeneration;

import :MoveHasher;

namespace chess {
	boost::concurrent_flat_map<Move, Rating, MoveHasher> historyTable;

	void updateHistoryScore(const Move& move, SafeUnsigned<std::uint8_t> remainingDepth) {
		auto delta = static_cast<Rating>((remainingDepth * remainingDepth).get());
		zAssert(delta >= 0);

		historyTable.visit(move, [&](auto& kv) {
			auto& rating = kv.second;
			if (std::numeric_limits<Rating>::max() - rating > delta) {
				rating += delta;
			}
		});
	}

	Rating getHistoryRating(const Move& move) {
		auto ret = 0_rt;
		historyTable.visit(move, [&](const auto& kv) {
			ret = kv.second;
		});
		return ret;
	}
}