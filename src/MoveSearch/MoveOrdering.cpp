module Chess.MoveSearch:MoveOrdering;

import Chess.Assert;
import Chess.AttackMap;
import Chess.Rating;
import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Profiler;
import :KillerMoveHistory;
import :PositionTable;

namespace chess {
	struct PVMoveInsertionResult {
		std::span<MovePriority> nonPVMoves;
		bool insertedPVMove = false;
	};

	PVMoveInsertionResult addPVEntry(const Move& pvMove, std::vector<MovePriority>& priorities) {
		if (pvMove != Move::null()) {
			auto it = std::ranges::find_if(priorities, [&](const MovePriority& p) {
				return p.getMove() == pvMove;
			});
			if (it != priorities.end()) { //if pv move is not found, then there was an incorrect hash made in the position table
				std::iter_swap(it, priorities.begin());
				std::span nonPVMoves{ priorities.data() + 1, priorities.size() - 1 };
				return { nonPVMoves, true };
			}
		}
		return { std::span{ priorities.data(), priorities.size() }, false };
	}

	FixedVector<MovePriority> getMovePrioritiesImpl(const Node& node, const Move& pvMove) {
		zAssert(node.getRemainingDepth() != 0_su8);

		const auto& posData = node.getPositionData();
		auto enemySquares = node.getEnemySquares();

		std::vector priorities{ std::from_range, posData.legalMoves | std::views::transform([&](const Move& move) {
			return MovePriority{ move, enemySquares, node.getRemainingDepth() - 1_su8 };
		}) };

		auto [nonPVMoves, insertedPVMove] = addPVEntry(pvMove, priorities);
		
		std::ranges::sort(nonPVMoves, [](const MovePriority& a, const MovePriority& b) {
			return a.getExchangeRating() > b.getExchangeRating();
		});

		auto sacrificeIt = std::ranges::upper_bound(nonPVMoves, 0_rt, std::greater{}, &MovePriority::getExchangeRating);
		if (sacrificeIt != nonPVMoves.end()) {
			auto sacrificeIndex = std::ranges::distance(nonPVMoves.begin(), sacrificeIt);
			if (insertedPVMove) {
				sacrificeIndex++; //make the sacrifice index not relative
			}
			if (sacrificeIndex != 0) { //disallow pruning of all legal moves
				priorities.erase(priorities.begin() + sacrificeIndex, priorities.end());
			}
		}
		
		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove) {
		ProfilerLock l{ getMovePrioritiesProfiler() };
		return getMovePrioritiesImpl(node, pvMove);
	}
}