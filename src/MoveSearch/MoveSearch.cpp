module;

#include <cassert>

module Chess.MoveSearch;

import std;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Position;
import Chess.Profiler;
import Chess.Rating;

import :KillerMoveHistory;
import :MovePriorityGeneration;
import :Node;
import :PositionTable;

namespace chess {
	std::chrono::steady_clock::time_point beginCalculation;
	constexpr std::chrono::seconds MAX_CALCULATION_TIME{ 120 };

	template<bool Maximizing>
	Rating extreme(Rating a, Rating b) {
		if constexpr (Maximizing) {
			return std::ranges::max(a, b);
		} else {
			return std::ranges::min(a, b);
		}
	}

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

	template<bool Maximizing>
	Rating minimax(const Node& node, AlphaBeta alphaBeta);

	template<typename Ret, bool Maximizing>
	Ret bestChildPosition(const Node& node, AlphaBeta alphaBeta) {
		auto legalMoves = calcAllLegalMoves(node.getPos());
		if (legalMoves.moves.empty()) {
			if constexpr (std::same_as<Ret, Rating>) {
				return legalMoves.isCheckmate ? checkmatedRating<Maximizing>() : 0_rt;
			} else {
				return std::nullopt;
			}
		}

		auto movePriorities = getMovePriorities(node, legalMoves.moves, Maximizing);
		auto bestMove = Move::null();
		auto bestRating = worstPossibleRating<Maximizing>();

		auto bound = InWindow;
		for (const auto& movePriority : movePriorities) {
			auto child = Node::makeChild(node, movePriority);
			auto childRating = minimax<!Maximizing>(child, alphaBeta);

			bestRating = extreme<Maximizing>(bestRating, childRating);
			if (bestRating == childRating) {
				bestMove = movePriority.getMove();
			}

			alphaBeta.update<Maximizing>(bestRating);
			if (alphaBeta.canPrune()) {
				updateHistoryScore(movePriority.getMove(), node.getLevel());
				bound = Maximizing ? LowerBound : UpperBound;
				break;
			}
		}

		assert(bestMove != Move::null());
		if (bound == InWindow) {
			if constexpr (Maximizing) {
				if (bestRating <= alphaBeta.getAlpha()) {
					bound = UpperBound;
				}
			} else {
				if (bestRating >= alphaBeta.getBeta()) {
					bound = LowerBound;
				}
			}
		}

		storePositionEntry(node.getPos(), { bestRating, bound, node.getLevelsToSearch(), bestMove });

		if constexpr (std::same_as<Ret, std::optional<Move>>) {
			return bestMove;
		} else {
			return bestRating;
		}
	}

	template<bool Maximizing>
	Rating minimax(const Node& node, AlphaBeta alphaBeta) {
		auto cachedEntryRes = getPositionEntry(node.getPos());
		if (cachedEntryRes) {
			const auto& cachedEntry = cachedEntryRes->get();
			if (cachedEntry.depth >= node.getLevelsToSearch()) {
				switch (cachedEntry.bound) {
				case InWindow:
					return cachedEntry.rating;
					break;
				case LowerBound:
					if (cachedEntry.rating >= alphaBeta.getBeta()) { //prune
						return cachedEntry.rating;
					}
					alphaBeta.updateAlpha(cachedEntry.rating);
					break;
				case UpperBound:
					if (cachedEntry.rating <= alphaBeta.getAlpha()) {
						return cachedEntry.rating;
					}
					alphaBeta.updateBeta(cachedEntry.rating);
					break;
				}
			}
		}

		auto timeCalculated = std::chrono::steady_clock::now() - beginCalculation;
		if (timeCalculated > MAX_CALCULATION_TIME || node.isDone()) {
			return node.getRating();
		}
		
		return bestChildPosition<Rating, Maximizing>(node, alphaBeta);
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, int depth) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(rootPos, depth, rootPos.isWhite());
		return bestChildPosition<std::optional<Move>, Maximizing>(root, alphaBeta);
	}

	std::optional<Move> findBestMove(const Position& pos, int depth) {
		ProfilerLock l{ getBestMoveProfiler() };

		beginCalculation = std::chrono::steady_clock::now();

		auto iterativeDeepening = [&]<bool Maximizing>() {
			//to slow at the moment!
			/*for (auto iterDepth = 1; iterDepth < depth; iterDepth++) {
				std::println("Iterative deepening at depth {}", iterDepth);
				bestMoveImpl<Maximizing>(pos, iterDepth);
			}*/
			return bestMoveImpl<Maximizing>(pos, depth);
		};
		if (pos.getTurnData().isWhite) {
			return iterativeDeepening.operator()<true>();
		} else {
			return iterativeDeepening.operator()<false>();
		}
	}
}