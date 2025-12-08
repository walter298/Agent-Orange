module Chess.MoveSearch:MovePriorityGeneration;

import Chess.Assert;
import Chess.Rating;
import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Profiler;
import :KillerMoveHistory;
import :PositionTable;

namespace chess {
	int calcDepth(Rating min, Rating max, Rating numerator, int minDepth, int maxDepth) {
		auto denominator = max - min;
		if (denominator == 0_rt) {
			return minDepth;
		}
		auto ret = (numerator / denominator) * static_cast<Rating>(maxDepth);
		return std::clamp(static_cast<int>(ret), minDepth, maxDepth);
	}

	int calcDepthWhite(Rating r, Rating min, Rating max, int minDepth, int maxDepth) {
		return calcDepth(min, max, r - min, minDepth, maxDepth);
	}

	int calcDepthBlack(Rating r, Rating min, Rating max, int minDepth, int maxDepth) {
		return calcDepth(min, max, max - r, minDepth, maxDepth);
	}

	template<bool Maximizing>
	int calcDepthBranch(Rating r, Rating min, Rating max, int minDepth, int maxDepth) {
		if constexpr (Maximizing) {
			return calcDepthWhite(r, min, max, minDepth, maxDepth);
		} else {
			return calcDepthBlack(r, min, max, minDepth, maxDepth);
		}
	}

	struct RatingBounds {
		Rating min = 0_rt;
		Rating max = 0_rt;
	};

	template<bool Maximizing, std::ranges::viewable_range Moves>
	auto addKillerMoves(Moves& moves, std::vector<MovePriority>& movePriorities, int minDepth, 
		int maxDepth)
	{
		auto [partitionPoint, end] = std::ranges::partition(moves, [&](const Move& move) {
			auto historyRating = getHistoryRating(move);
			return historyRating > 0_rt;
		});

		auto killerMoves = std::ranges::subrange(moves.begin(), partitionPoint);
		auto historyMoveCalculator = [](const Move& move) {
			return getHistoryRating(move);
		};

		if (!std::ranges::empty(killerMoves)) {
			auto minRating = std::ranges::min(killerMoves | std::views::transform(historyMoveCalculator));
			auto maxRating = std::ranges::max(killerMoves | std::views::transform(historyMoveCalculator));

			auto killerMovePriorities = killerMoves | std::views::transform([&](const Move& move) {
				auto historyRating = getHistoryRating(move);
				auto depth = calcDepthBranch<Maximizing>(historyRating, minRating, maxRating, minDepth, maxDepth);
				return MovePriority{ move, depth };
			});

			auto size = movePriorities.size();
			movePriorities.append_range(killerMovePriorities);
			std::ranges::sort(movePriorities.begin() + size, movePriorities.end(), [&](const MovePriority& a, const MovePriority& b) {
				return a.getDepth() > b.getDepth();
			});
		}

		return std::ranges::subrange(partitionPoint, end);
	}

	template<std::ranges::viewable_range Moves>
	auto addCaptures(Moves& moves, std::vector<MovePriority>& movePriorities, int maxDepth) {
		auto [partitionPoint, end] = std::ranges::partition(moves, [&](const Move& move) {
			return move.isMaterialChange();
		});

		auto captures = std::ranges::subrange(moves.begin(), partitionPoint);
		auto getMaterialChange = [](const Move& move) {
			auto captureChange = getPieceRating(move.capturedPiece) - getPieceRating(move.movedPiece);
			auto promotionChange = move.promotionPiece != Piece::None ? getPieceRating(move.promotionPiece) - getPieceRating(Pawn) : 0_rt;
			return captureChange + promotionChange;
		};

		if (!std::ranges::empty(captures)) {
			std::ranges::sort(captures, [&](const Move& a, const Move& b) {
				return getMaterialChange(a) > getMaterialChange(b);
			});
			movePriorities.append_range(captures | std::views::transform([&](const Move& move) {
				return MovePriority{ move, maxDepth }; //freeze depth for captures
			}));
		}

		return std::ranges::subrange(partitionPoint, end);
	}

	auto getPVEntry(const Node& node, std::vector<Move>& moves, std::vector<MovePriority>& priorities) {
		auto entryRes = getPositionEntry(node.getPos());
		if (entryRes) {
			auto& entry = entryRes->get();
			auto it = std::ranges::find(moves, entry.bestMove);
			if (it != moves.end()) {
				priorities.emplace_back(entry.bestMove, node.getRemainingDepth());
				std::iter_swap(it, moves.begin());
				return std::ranges::subrange(std::next(moves.begin()), moves.end());
			}
		}
		return std::ranges::subrange(moves.begin(), moves.end());
	}

	template<bool Maximizing>
	FixedVector<MovePriority> getMovePrioritiesImpl(const Node& node, const std::vector<Move>& legalMoves) {
		if (node.getRemainingDepth() <= 0) {
			std::println("Error: incorrect depth search: {}", node.getRemainingDepth());
			std::exit(-1);
		}

		std::vector<MovePriority> priorities;
		priorities.reserve(legalMoves.size());
		auto temp = legalMoves;

		auto nonPVMoves = getPVEntry(node, temp, priorities);

		auto captureDepth = node.getRemainingDepth();
		if (captureDepth > 1) {
			captureDepth--;
		}
		auto nonCaptures = addCaptures(nonPVMoves, priorities, captureDepth);
		auto maxHistoryDepth = node.getRemainingDepth() - 1; //always greater than or equal to 1
		auto minHistoryDepth = maxHistoryDepth / 2;
		auto nonKillerMoves = addKillerMoves<Maximizing>(nonCaptures, priorities, minHistoryDepth, maxHistoryDepth);

		if (!std::ranges::empty(nonKillerMoves)) {
			priorities.append_range(nonKillerMoves | std::views::transform([&](const Move& move) {
				return MovePriority{ move, minHistoryDepth };
			}));
		}

		zAssert(!priorities.empty());

		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const Node& node, const std::vector<Move>& legalMoves, bool maximizing) {
		ProfilerLock l{ getMovePrioritiesProfiler() };

		if (maximizing) {
			return getMovePrioritiesImpl<true>(node, legalMoves);
		} else {
			return getMovePrioritiesImpl<false>(node, legalMoves);
		}
	}
}