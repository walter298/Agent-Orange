module Chess.MoveSearch:MovePriority;

import Chess.Rating;
import Chess.Evaluation;
import Chess.Profiler;
import :MoveHistory;

namespace chess {
	int calcDepth(Rating min, Rating max, Rating numerator, int minDepth, int maxDepth) {
		auto denominator = max - min;
		if (denominator == 0_rt) {
			return 0;
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
	auto addHistoryMoves(Moves&& moves, std::vector<MovePriority>& movePriorities, int minDepth, 
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
			auto minRating = std::ranges::min(moves | std::views::transform(historyMoveCalculator));
			auto maxRating = std::ranges::max(moves | std::views::transform(historyMoveCalculator));

			auto killerMovePriorities = killerMoves | std::views::transform([&](const Move& move) {
				auto historyRating = getHistoryRating(move);
				auto depth = calcDepthBranch<Maximizing>(historyRating, minRating, maxRating, minDepth, maxDepth);
				return MovePriority{ move, depth };
			});

			auto size = movePriorities.size();
			movePriorities.append_range(killerMovePriorities);
			std::ranges::sort(movePriorities.begin() + size, movePriorities.end(), [&](const MovePriority& a, const MovePriority& b) {
				return a.recommendedDepth > b.recommendedDepth;
			});
		}

		return std::ranges::subrange(partitionPoint, end);
	}

	auto addCaptures(std::vector<Move>& moves, std::vector<MovePriority>& movePriorities, int maxDepth) {
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

	template<bool Maximizing>
	FixedVector<MovePriority> getMovePrioritiesImpl(const std::vector<Move>& legalMoves, int maxDepth, int level) {
		if (maxDepth <= 0) { //max depth can be negative if in a capture sequence
			throw std::runtime_error{ "Error: maxDepth <= 0" };
		}
		
		auto temp = legalMoves;
		std::vector<MovePriority> priorities;
		priorities.reserve(legalMoves.size());

		auto nonCaptures = addCaptures(temp, priorities, maxDepth);
		auto maxHistoryDepth = maxDepth - 1; //always greater than or equal to 1
		auto minHistoryDepth = maxHistoryDepth / 2;
		auto unexploredMoves = addHistoryMoves<Maximizing>(nonCaptures, priorities, minHistoryDepth, maxHistoryDepth);
		auto movesToDiscard = std::clamp(0b1uz << level, 0uz, unexploredMoves.size());
		auto keptMoves = unexploredMoves | std::views::drop(movesToDiscard);
		if (!std::ranges::empty(unexploredMoves)) {
			priorities.append_range(unexploredMoves | std::views::transform([&](const Move& move) {
				return MovePriority{ move, minHistoryDepth };
			}));
		}

		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const std::vector<Move>& legalMoves, int maxDepth, int level, bool maximizing) {
		static MaybeProfiler profiler{ "findBestMove", "getMovePriorities" };
		ProfilerLock l{ profiler };

		if (maximizing) {
			return getMovePrioritiesImpl<true>(legalMoves, maxDepth, level);
		} else {
			return getMovePrioritiesImpl<false>(legalMoves, maxDepth, level);
		}
	}
}