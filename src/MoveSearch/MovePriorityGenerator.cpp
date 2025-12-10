module Chess.MoveSearch:MovePriorityGeneration;

import Chess.Assert;
import Chess.AttackMap;
import Chess.Rating;
import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Profiler;
import :KillerMoveHistory;
import :PositionTable;

namespace chess {
	template<std::ranges::viewable_range Range>
	auto makePriorityRange(Range moves, std::uint8_t depth) {
		return moves | std::views::transform([&](const Move& move) {
			return MovePriority{ move, depth };
		});
	}

	template<std::ranges::viewable_range Moves>
	void discardSacrifices(const Node& node, Moves& moves, arena::Vector<MovePriority>& movePriorities) {
		auto enemySquares = node.getEnemySquares();
		auto [partitionPoint, end] = std::ranges::partition(moves, [&](const Move& move) {
			return !(makeBitboard(move.to) & enemySquares);
		});

		std::uint8_t depth = node.getRemainingDepth() - std::uint8_t{ 1 };

		auto nonSacrifices = makePriorityRange(std::ranges::subrange(moves.begin(), partitionPoint), depth);
		movePriorities.append_range(nonSacrifices);

		if (movePriorities.empty()) { //put sacrifices only if we have no other options
			auto sacrifices = makePriorityRange(std::ranges::subrange(partitionPoint, end), depth);
			movePriorities.append_range(sacrifices);
		}
	}

//	template<std::ranges::viewable_range Moves>
//	auto addKillerMoves(const Node& node, Moves& moves, arena::Vector<MovePriority>& movePriorities)
//	{
//		auto [partitionPoint, end] = std::ranges::partition(moves, [&](const Move& move) {
//			auto historyRating = getHistoryRating(move);
//			return historyRating > 0_rt;
//		});
//
//		auto killerMoves = std::ranges::subrange(moves.begin(), partitionPoint);
//		auto historyMoveCalculator = [](const Move& move) {
//			return getHistoryRating(move);
//		};
//
//		if (!std::ranges::empty(killerMoves)) {
//			std::uint8_t depth = node.getRemainingDepth() - 1;
//			auto killerMovePriorities = makePriorityRange(killerMoves, depth);
//
//			auto size = movePriorities.size();
//			movePriorities.append_range(killerMovePriorities);
//			std::ranges::sort(movePriorities.begin() + size, movePriorities.end(), [&](const MovePriority& a, const MovePriority& b) {
//				auto historyRatingA = getHistoryRating(a.getMove());
//				auto historyRatingB = getHistoryRating(b.getMove());
//				return historyRatingA > historyRatingB;
//			});
//		}
//
//		return std::ranges::subrange(partitionPoint, end);
//	}

	Rating getEvasionRating(const Move& move, Bitboard enemySquares) {
		auto fromBoard = makeBitboard(move.from);
		auto toBoard = makeBitboard(move.to);
		if ((fromBoard & enemySquares)) {
			if (toBoard & enemySquares) { //if evacuation square is defended
				return getPieceRating(move.capturedPiece) - getPieceRating(move.movedPiece);
			}
		}
		return 0_rt;
	}

	template<std::ranges::viewable_range Moves>
	auto addNonSacrifices(const Node& node, Moves& moves, arena::Vector<MovePriority>& priorities) {
		AttackMap attackMap{ node.getEnemySquares() };

		auto materialChangeComp = [&](const Move& a, const Move& b) {
			return attackMap.materialChange(a) > attackMap.materialChange(b);
		};
		std::ranges::sort(moves, materialChangeComp);

		auto sacrificeIt = std::ranges::upper_bound(moves, 0_rt, std::greater{}, [&](const Move& move) {
			return attackMap.materialChange(move);
		});
		auto nonSacrifices = std::ranges::subrange(moves.begin(), sacrificeIt);
		priorities.append_range(makePriorityRange(nonSacrifices, node.getRemainingDepth() - 1));

		return std::ranges::subrange(sacrificeIt, moves.end());
	}

	auto getPVEntry(const Node& node, arena::Vector<Move>& moves, const Move& pvMove, arena::Vector<MovePriority>& priorities) {
		std::uint8_t depth = node.getRemainingDepth() - std::uint8_t{ 1 };
		if (pvMove != Move::null()) {
			auto it = std::ranges::find(moves, pvMove);
			if (it != moves.end()) {
				priorities.emplace_back(pvMove, depth);
				std::iter_swap(it, moves.begin());
				return std::ranges::subrange(std::next(moves.begin()), moves.end());
			}
		}
		return std::ranges::subrange(moves.begin(), moves.end());
	}

	FixedVector<MovePriority> getMovePrioritiesImpl(const Node& node, const Move& pvMove) {
		zAssert(node.getRemainingDepth() != 0);

		const auto& posData = node.getPositionData();

		arena::Vector<MovePriority> priorities;
		priorities.reserve(posData.legalMoves.size());
		auto temp = posData.legalMoves;

		auto nonPVMoves = getPVEntry(node, temp, pvMove, priorities);
		auto sacrifices = addNonSacrifices(node, nonPVMoves, priorities);

		if (priorities.empty()) {
			priorities.append_range(makePriorityRange(sacrifices, node.getRemainingDepth() - 1));
		}

		zAssert(!priorities.empty());

		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove) {
		ProfilerLock l{ getMovePrioritiesProfiler() };
		return getMovePrioritiesImpl(node, pvMove);
	}
}