module;

#include <cassert>

module Chess.MoveSearch:MovePriorityGeneration;

import Chess.Rating;
import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Profiler;
import :KillerMoveHistory;

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
	auto addKillerMoves(Moves&& moves, std::vector<MovePriority>& movePriorities, int minDepth, 
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
				return a.getDepth() > b.getDepth();
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
	FixedVector<MovePriority> getMovePrioritiesImpl(const Node& node, const std::vector<Move>& legalMoves) {
		if (node.getLevelsToSearch() <= 0) {
			std::println("Error: incorrect depth search: {}", node.getLevelsToSearch());
			std::exit(-1);
		}

		std::vector<MovePriority> priorities;
		priorities.reserve(legalMoves.size());
		auto temp = legalMoves;

		auto nonCaptures = addCaptures(temp, priorities, node.getLevelsToSearch());
		auto maxHistoryDepth = node.getLevelsToSearch() - 1; //always greater than or equal to 1
		auto minHistoryDepth = maxHistoryDepth / 2;
		auto nonKillerMoves = addKillerMoves<Maximizing>(nonCaptures, priorities, minHistoryDepth, maxHistoryDepth);

		auto movesToDiscard = std::clamp(0b1uz << node.getLevel(), 0uz, nonKillerMoves.size());

		if (node.inWinningAttackSequence()) {
			if (!priorities.empty() && !node.inWinningAttackSequence()) {
				return FixedVector{ std::move(priorities) };
			}
		}

		//prevent discarding moves so that there will be literally zero moves to evaluate
		if (priorities.empty() && movesToDiscard == nonKillerMoves.size()) {
			assert(!std::ranges::empty(nonKillerMoves));
			movesToDiscard -= 1; //should not wrap around because nonKillerMoves.size() > 0
			assert(movesToDiscard != std::numeric_limits<size_t>::max());
		}
		auto reducedNonKillerMoves = nonKillerMoves | std::views::drop(movesToDiscard);

		if (!std::ranges::empty(reducedNonKillerMoves)) {
			priorities.append_range(reducedNonKillerMoves | std::views::transform([&](const Move& move) {
				return MovePriority{ move, minHistoryDepth };
			}));
		} 

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