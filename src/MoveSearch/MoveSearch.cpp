module;

#include <unifex/when_all_range.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

module Chess.MoveSearch;

import std;

import Chess.Assert;
import Chess.DebugPrint;
import Chess.EasyRandom;
import Chess.Evaluation;
import Chess.MoveGeneration;
import Chess.Position.RepetitionMap;
import Chess.Profiler;
import Chess.Rating;

import :KillerMoveHistory;
import :MoveOrdering;
import :Node;
import :PositionTable;

namespace chess {
	class AlphaBeta {
	private:
		//add or subtract 1 so that checkmate ratings are always worse than these bounds
		Rating m_alpha = worstPossibleRating<true>();
		Rating m_beta = worstPossibleRating<false>();
	public:
		void updateAlpha(Rating childRating) {
			m_alpha = std::max(childRating, m_alpha);
		}
		void updateBeta(Rating childRating) {
			m_beta = std::min(childRating, m_beta);
		}

		template<bool Maximizing>
		void update(Rating childRating) {
			if constexpr (Maximizing) {
				updateAlpha(childRating);
			}
			else {
				updateBeta(childRating);
			}
		}

		bool canPrune() const {
			return m_beta <= m_alpha;
		}

		Rating getAlpha() const {
			return m_alpha;
		}
		Rating getBeta() const {
			return m_beta;
		}
	};

	struct PositionRating {
		Rating rating = 0_rt;
		bool isRepetition = false;
	};

	bool wouldMakeRepetition(const Position& pos, Move pvMove, const RepetitionMap& repetitionMap) {
		Position child{ pos, pvMove };
		auto repetitionCount = repetitionMap.getPositionCount(child) + 1; //add 1 since we haven't actually pushed this position yet
		return repetitionCount >= 2; //return 2 (not 3) because the opposing player could then make a threefold repetition after this
	}

	template<typename Ret, bool Maximizing>
	Ret bestChildPosition(const Node& node, const Move& pvMove, AlphaBeta alphaBeta);

	template<typename Ret, bool Maximizing>
	Ret minimax(const Node& node, AlphaBeta alphaBeta) {
		if (node.getPositionData().legalMoves.empty()) {
			if constexpr (std::same_as<Ret, PositionRating>) {
				auto rating = node.getPositionData().isCheckmate ? checkmatedRating<Maximizing>() : 0_rt;
				return PositionRating{ rating, false };
			} else {
				return std::nullopt;
			}
		}

		if (node.getRepetitionMap().getPositionCount(node.getPos()) >= 3) {
			if constexpr (std::same_as<Ret, PositionRating>) {
				return { 0_rt, true };
			}
		}

		auto pvMove = Move::null();

		auto returnImpl = [&](const PositionEntry& entry) {
			if constexpr (std::same_as<Ret, PositionRating>) {
				return PositionRating{ entry.rating, false };
			}
			else {
				debugPrint(std::format("At the root, we are returning ({})", entry.bestMove.getUCIString()));
				return entry.bestMove;
			}
		};
		auto entryRes = getPositionEntry(node.getPos());
		if (entryRes) {
			const auto& entry = *entryRes;

			if (!wouldMakeRepetition(node.getPos(), entry.bestMove, node.getRepetitionMap()) && entry.depth >= node.getRemainingDepth()) {
				switch (entry.bound) {
				case InWindow:
					return returnImpl(entry);
					break;
				case LowerBound:
					if (entry.rating >= alphaBeta.getBeta()) {
						return returnImpl(entry);
					} else {
						alphaBeta.updateAlpha(entry.rating);
					}
					break;
				case UpperBound:
					if (entry.rating <= alphaBeta.getAlpha()) {
						return returnImpl(entry);
					} else {
						alphaBeta.updateBeta(entry.rating);
					}
					break;
				}
			}
			pvMove = entry.bestMove;
		}
		if constexpr (std::same_as<Ret, PositionRating>) { //node will not be done if we are at the root, so it is safe to have this check only for ratings
			if (node.isDone()) {
				return { node.getRating(), false }; //already checked whether position is a repetition
			}
		}

		return bestChildPosition<Ret, Maximizing>(node, pvMove, alphaBeta);
	}

	template<typename Ret, bool Maximizing>
	Ret bestChildPosition(const Node& node, const Move& pvMove, AlphaBeta alphaBeta) {
		auto originalAlphaBeta = alphaBeta;
		auto movePriorities = getMovePriorities(node, pvMove);
		auto bestMove = Move::null();
		PositionRating bestRating{ worstPossibleRating<Maximizing>(), false };

		auto bound = InWindow;
		bool didNotPrune = true;

		for (const auto& movePriority : movePriorities) {
			auto child = Node::makeChild(node, movePriority);
			auto childRating = minimax<PositionRating, !Maximizing>(child, alphaBeta);

			if constexpr (Maximizing) {
				if (childRating.rating > bestRating.rating) {
					bestRating = childRating;
					bestMove = movePriority.getMove();
				}
			} else {
				if (childRating.rating < bestRating.rating) {
					bestRating = childRating;
					bestMove = movePriority.getMove();
				}
			}

			alphaBeta.update<Maximizing>(bestRating.rating);
			if (alphaBeta.canPrune()) {
				updateHistoryScore(movePriority.getMove(), node.getRemainingDepth());
				bound = Maximizing ? LowerBound : UpperBound;
				didNotPrune = false;
				break;
			}
		}

		zAssert(bestMove != Move::null());
		if (didNotPrune) {
			if constexpr (Maximizing) {
				if (bestRating.rating <= originalAlphaBeta.getAlpha()) {
					bound = UpperBound;
				}
			}
			else {
				if (bestRating.rating >= originalAlphaBeta.getBeta()) {
					bound = LowerBound;
				}
			}
		}

		if (!bestRating.isRepetition) {
			PositionEntry newEntry{ bestMove, bestRating.rating, node.getRemainingDepth(), bound };
			storePositionEntry(node.getPos(), newEntry);
		}

		bestRating.isRepetition = false; //don't propagate repetition flag up the tree

		if constexpr (std::same_as<Ret, std::optional<Move>>) {
			return bestMove;
		} else {
			return bestRating;
		}
	}

	using MoveTask  = unifex::task<std::optional<Move>>;
	using Scheduler = unifex::static_thread_pool::scheduler;

	template<bool Maximizing>
	std::optional<Move> startAlphaBetaSearch(const Position& pos, SafeUnsigned<std::uint8_t> depth, RepetitionMap repetitionMap) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(pos, depth, repetitionMap);
		return minimax<std::optional<Move>, Maximizing>(root, alphaBeta);
	}

	constexpr auto THREAD_COUNT = 4;

	template<bool Maximizing>
	MoveTask findBestMoveImpl(Scheduler scheduler, const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
		co_await unifex::schedule(scheduler); //run asynchronously
		for (auto iterDepth = 1_su8; iterDepth < depth; ++iterDepth) {
			startAlphaBetaSearch<Maximizing>(pos, iterDepth, repetitionMap);
		}
		co_return startAlphaBetaSearch<Maximizing>(pos, depth, repetitionMap);
	}

	std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
		unifex::static_thread_pool pool{ THREAD_COUNT };

		if (pos.isWhite()) {
			return unifex::sync_wait(findBestMoveImpl<true>(pool.get_scheduler(), pos, depth, repetitionMap)).value();
		} else {
			return unifex::sync_wait(findBestMoveImpl<false>(pool.get_scheduler(), pos, depth, repetitionMap)).value();
		}
	}
}