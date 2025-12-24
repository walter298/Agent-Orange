module Chess.MoveSearch;

import std;

import Chess.Assert;
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

	template<typename Ret, bool Maximizing>
	Ret minimaxImpl(const Node& node, const Move& pvMove, AlphaBeta alphaBeta);

	template<typename Ret, bool Maximizing>
	Ret minimax(const Node& node, AlphaBeta alphaBeta) {
		if constexpr (std::same_as<Ret, Rating>) {
			auto repetitionCount = repetition::getPositionCount(node.getPos());
			if (repetitionCount == 3) {
				return 0_rt;
			}
		}

		auto returnImpl = [](const PositionEntry& entry) {
			if constexpr (std::same_as<Ret, Rating>) {
				return entry.rating;
			} else {
				return entry.bestMove;
			}
		};

		auto entryRes = getPositionEntry(node.getPos());
		if (entryRes) {
			const auto& entry = entryRes->get();
			if (entry.depth >= node.getRemainingDepth()) {
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
				return minimaxImpl<Ret, Maximizing>(node, entry.bestMove, alphaBeta);
			}
		}
		return minimaxImpl<Ret, Maximizing>(node, Move::null(), alphaBeta);
	}

	template<typename Ret, bool Maximizing>
	Ret minimaxImpl(const Node& node, const Move& pvMove, AlphaBeta alphaBeta) {
		if constexpr (std::same_as<Ret, Rating>) {
			if (node.isDone()) {
				return node.getRating();
			}
		}

		const auto& posData = node.getPositionData();

		if (posData.legalMoves.empty()) {
			if constexpr (std::same_as<Ret, Rating>) {
				return posData.isCheckmate ? checkmatedRating<Maximizing>() : 0_rt;
			} else {
				return std::nullopt;
			}
		}

		auto originalAlphaBeta = alphaBeta;
		auto movePriorities = getMovePriorities(node, pvMove);
		auto bestMove = Move::null();
		auto bestRating = worstPossibleRating<Maximizing>();

		auto bound = InWindow;
		bool didNotPrune = true;

		for (const auto& movePriority : movePriorities) {
			auto child = Node::makeChild(node, movePriority);

			auto childRating = minimax<Rating, !Maximizing>(child, alphaBeta);

			if constexpr (Maximizing) {
				if (childRating > bestRating) {
					bestRating = childRating;
					bestMove = movePriority.getMove();
				}
			} else {
				if (childRating < bestRating) {
					bestRating = childRating;
					bestMove = movePriority.getMove();
				}
			}

			alphaBeta.update<Maximizing>(bestRating);
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
				if (bestRating <= originalAlphaBeta.getAlpha()) {
					bound = UpperBound;
				}
			} else {
				if (bestRating >= originalAlphaBeta.getBeta()) {
					bound = LowerBound;
				}
			}
		}

		PositionEntry newEntry{ bestMove, bestRating, node.getRemainingDepth(), bound };
		storePositionEntry(node.getPos(), newEntry);

		if constexpr (std::same_as<Ret, std::optional<Move>>) {
			return bestMove;
		} else {
			return bestRating;
		}
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, SafeUnsigned<std::uint8_t> depth) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(rootPos, depth, rootPos.isWhite());
		return minimax<std::optional<Move>, Maximizing>(root, alphaBeta);
	}

	std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth) {
		ProfilerLock l{ getBestMoveProfiler() };

		auto iterativeDeepening = [&]<bool Maximizing>() {
			for (auto iterDepth = 1_su8; iterDepth < depth; ++iterDepth) {
				bestMoveImpl<Maximizing>(pos, iterDepth);
			}
			std::println("Searching at depth {}", static_cast<unsigned int>(depth.get()));
			return bestMoveImpl<Maximizing>(pos, depth);
		};
		if (pos.getTurnData().isWhite) {
			return iterativeDeepening.operator()<true>();
		} else {
			return iterativeDeepening.operator()<false>();
		}
	}
}