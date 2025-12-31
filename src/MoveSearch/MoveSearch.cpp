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
		Rating m_beta  = worstPossibleRating<false>();
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
			} else {
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

	bool wouldMakeRepetition(const Position& pos, Move pvMove) {
		Position child{ pos, pvMove };
		auto repetitionCount = repetition::getPositionCount(child) + 1; //add 1 since we haven't actually pushed this position yet
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

		if (repetition::getPositionCount(node.getPos()) >= 3) {
			if constexpr (std::same_as<Ret, PositionRating>) {
				debugPrint("Returning draw");
				return { 0_rt, true };
			} 
		}

		auto pvMove = Move::null();

		auto returnImpl = [&](const PositionEntry& entry) {
			if constexpr (std::same_as<Ret, PositionRating>) {
				return PositionRating{ entry.rating, false };
			} else {
				debugPrint(std::format("At the root, we are returning ({})", entry.bestMove.getUCIString()));
				return entry.bestMove;
			}
		};
		auto entryRes = getPositionEntry(node.getPos());
		if (entryRes) {
			const auto& entry = *entryRes;

			if (!wouldMakeRepetition(node.getPos(), entry.bestMove) && entry.depth >= node.getRemainingDepth()) {
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
			} else {
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

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, SafeUnsigned<std::uint8_t> depth) {
		AlphaBeta alphaBeta;
		zAssert(repetition::getPositionCount(rootPos) > 0);
		auto root = Node::makeRoot(rootPos, depth, rootPos.isWhite());
		return minimax<std::optional<Move>, Maximizing>(root, alphaBeta);
	}

	class SearchThread {
	private:
		struct State {
			std::jthread thread;
			std::optional<Move> move;

			State(const Position& pos, SafeUnsigned<std::uint8_t> depth, std::latch& latch)
				: thread{ &State::run, std::ref(*this), std::cref(pos), depth, std::ref(latch) }
			{
			}
			void run(const Position& pos, SafeUnsigned<std::uint8_t> depth, std::latch& latch) {
				auto iterativeDeepening = [&]<bool Maximizing>() -> std::optional<Move> {
					for (auto iterDepth = 1_su8; iterDepth < depth; ++iterDepth) {
						bestMoveImpl<Maximizing>(pos, iterDepth);
					}
					return bestMoveImpl<Maximizing>(pos, depth);
				};
				if (pos.getTurnData().isWhite) {
					move = iterativeDeepening.operator()<true>();
				} else {
					move = iterativeDeepening.operator()<false>();
				}
				latch.count_down();
			}
		};
		std::unique_ptr<State> m_state;
	public:
		SearchThread(const Position& pos, SafeUnsigned<std::uint8_t> depth, std::latch& latch)
			: m_state{ std::make_unique<State>(pos, depth, latch) }
		{
		}

		std::optional<Move> get() const {
			return m_state->move;
		}
	};

	std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth) {
		ProfilerLock l{ getBestMoveProfiler() };

		auto iterativeDeepening = [&]<bool Maximizing>() -> std::optional<Move> {
			for (auto iterDepth = 1_su8; iterDepth < depth; ++iterDepth) {
				bestMoveImpl<Maximizing>(pos, iterDepth);
			}
			return bestMoveImpl<Maximizing>(pos, depth);
		};
		if (pos.getTurnData().isWhite) {
			return iterativeDeepening.operator() < true > ();
		} else {
			return iterativeDeepening.operator() < false > ();
		}

		//constexpr auto THREAD_COUNT = 3;
		//std::latch latch{ THREAD_COUNT };

		//std::vector<SearchThread> searchThreads;
		//for (int i = 0; i < THREAD_COUNT; i++) {
		//	searchThreads.emplace_back(pos, depth, latch);
		//}
		//latch.wait(); //wait til all children are done searching

		//auto moveIndex = makeRandomNum(0uz, searchThreads.size() - 1);
		//return searchThreads[moveIndex].get();
	}
}