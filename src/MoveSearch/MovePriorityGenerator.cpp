module Chess.MoveSearch:MovePriorityGeneration;

import Chess.Assert;
import Chess.Rating;
import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Profiler;
import :KillerMoveHistory;
import :PositionTable;

namespace chess {
	template<std::ranges::viewable_range Moves>
	auto addKillerMoves(Moves& moves, arena::Vector<MovePriority>& movePriorities, std::uint8_t depth)
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
			auto killerMovePriorities = killerMoves | std::views::transform([&](const Move& move) {
				return MovePriority{ move, depth };
			});

			auto size = movePriorities.size();
			movePriorities.append_range(killerMovePriorities);
			std::ranges::sort(movePriorities.begin() + size, movePriorities.end(), [&](const MovePriority& a, const MovePriority& b) {
				auto historyRatingA = getHistoryRating(a.getMove());
				auto historyRatingB = getHistoryRating(b.getMove());
				return historyRatingA > historyRatingB;
			});
		}

		return std::ranges::subrange(partitionPoint, end);
	}

	template<std::ranges::viewable_range Moves>
	auto addCaptures(Moves& moves, arena::Vector<MovePriority>& movePriorities, std::uint8_t maxDepth) {
		auto [partitionPoint, end] = std::ranges::partition(moves, [&](const Move& move) {
			return move.capturedPiece != Piece::None;
		});

		auto captures = std::ranges::subrange(moves.begin(), partitionPoint);
		if (!std::ranges::empty(captures)) {
			std::ranges::sort(captures, [&](const Move& a, const Move& b) {
				return getPieceRating(a.capturedPiece) > getPieceRating(b.capturedPiece);
			});
			movePriorities.append_range(captures | std::views::transform([&](const Move& move) {
				return MovePriority{ move, maxDepth }; //freeze depth for captures
			}));
		}

		return std::ranges::subrange(partitionPoint, end);
	}

	auto getPVEntry(const Node& node, arena::Vector<Move>& moves, const Move& pvMove, arena::Vector<MovePriority>& priorities) {
		if (pvMove != Move::null()) {
			auto it = std::ranges::find(moves, pvMove);
			if (it != moves.end()) {
				priorities.emplace_back(pvMove, node.getRemainingDepth());
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

		auto captureDepth = node.getRemainingDepth();
		if (captureDepth > 1) { //don't want to blindly stop in the middle of a capture sequence
			captureDepth--;
		}
		
		auto nonCaptures = addCaptures(nonPVMoves, priorities, captureDepth);
		std::uint8_t nonCaptureDepth = node.getRemainingDepth() - std::uint8_t{ 1 }; //always greater than or equal to 1
		auto nonKillerMoves = addKillerMoves(nonCaptures, priorities, nonCaptureDepth);

		if (!std::ranges::empty(nonKillerMoves)) {
			priorities.append_range(nonKillerMoves | std::views::transform([&](const Move& move) {
				return MovePriority{ move, nonCaptureDepth };
			}));
		}

		zAssert(!priorities.empty());

		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove) {
		ProfilerLock l{ getMovePrioritiesProfiler() };
		return getMovePrioritiesImpl(node, pvMove);
	}
}