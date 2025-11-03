module Chess.MoveSearch:MovePriority;

import Chess.Rating;
import Chess.Evaluation;
import :MoveHistory;

namespace chess {
	using MoveIterator = std::vector<Move>::iterator;

	int minDepth(int maxDepth) {
		if (maxDepth <= 0) {
			return 0;
		}
		return std::clamp(maxDepth / 2, 1, maxDepth);
	}

	int getRecommendedDepth(Rating rating, Rating bestRating, int maxDepth) {
		if (maxDepth <= 0) {
			return 0;
		}
		if (bestRating == 0rt) {
			return minDepth(maxDepth);
		}
		auto maxDepthScore = static_cast<Rating>(maxDepth);
		auto val = static_cast<int>(rating * maxDepthScore / bestRating);
		
		if (val <= 0) {
			val = 0;
		}
		if (val > maxDepth) val = maxDepth;
		return val;
	}

	MoveIterator addHistoryMoves(std::vector<Move>& moves, std::vector<MovePriority>& movePriorities, int maxDepth) {
		auto [unrecordedMovesBegin, end] = std::ranges::partition(moves, [&](const Move& move) {
			auto historyRating = getHistoryRating(move);
			return historyRating > 0rt;
		});

		auto recordedMoves = std::ranges::subrange(moves.begin(), unrecordedMovesBegin);

		if (!std::ranges::empty(recordedMoves)) {
			auto bestHistoryRating = std::ranges::max(recordedMoves | std::views::transform([](const Move& move) {
				return getHistoryRating(move);
			}));

			movePriorities.append_range(recordedMoves | std::views::transform([&](const Move& move) {
				auto historyRating = getHistoryRating(move);
				//auto depth = getRecommendedDepth(historyRating, bestHistoryRating, maxDepth);
				return MovePriority{ move, maxDepth };
			}));
		}

		return unrecordedMovesBegin;
	}

	MoveIterator addCaptures(std::vector<Move>& moves, MoveIterator begin, std::vector<MovePriority>& movePriorities,
		int maxDepth)
	{
		auto nonHistoryMoves = std::ranges::subrange(begin, moves.end());
		auto [nonCapturedBegin, end] = std::ranges::partition(nonHistoryMoves, [&](const Move& move) {
			return move.capturedPiece != Piece::None;
		});

		auto captures = std::ranges::subrange(begin, nonCapturedBegin);

		if (!std::ranges::empty(captures)) {
			auto bestCaptureRating = std::ranges::max(captures | std::views::transform([](const Move& move) {
				return getPieceRating(move.capturedPiece) - getPieceRating(move.movedPiece);
			}));

			movePriorities.append_range(captures | std::views::transform([&](const Move& move) {
				auto captureDiff = getPieceRating(move.capturedPiece) - getPieceRating(move.movedPiece);
				//auto depth = getRecommendedDepth(captureDiff, bestCaptureRating, maxDepth);
				return MovePriority{ move, maxDepth };
			}));
		}

		return nonCapturedBegin;
	}

	FixedVector<MovePriority> getMovePriorities(const std::vector<Move>& legalMoves, int maxDepth) {
		auto temp = legalMoves;
		std::vector<MovePriority> priorities;

		auto unrecordedBegin  = addHistoryMoves(temp, priorities, maxDepth);
		auto noncapturedBegin = addCaptures(temp, unrecordedBegin, priorities, maxDepth);

		auto likelyBadMoves = std::ranges::subrange(noncapturedBegin, temp.end());
		if (!std::ranges::empty(likelyBadMoves)) {
			priorities.append_range(likelyBadMoves | std::views::transform([&](const Move& move) {
				auto depth = minDepth(maxDepth);
				return MovePriority{ move, maxDepth };
			}));
		}

		std::ranges::sort(priorities, [](const MovePriority& a, const MovePriority& b) {
			return a.recommendedDepth > b.recommendedDepth;
		});

		return FixedVector{ std::move(priorities) };
	}
}