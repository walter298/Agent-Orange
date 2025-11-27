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
import :ThreadPool;

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
		Rating m_beta = worstPossibleRating<false>();
	public:
		template<bool Maximizing>
		bool update(Rating childRating) {
			auto updateImpl = [](Rating& bound, Rating childRating, auto combiner) {
				auto temp = bound;
				bound = combiner(bound, childRating);
				return temp != bound;
			}; 
			if constexpr (Maximizing) {
				return updateImpl(m_alpha, childRating, std::ranges::max);
			} else {
				return updateImpl(m_beta, childRating, std::ranges::min);
			}
		}

		bool canPrune() const {
			return m_beta <= m_alpha;
		}

		template<bool Maximizing>
		Rating getAllyRating() const {
			if constexpr (Maximizing) {
				return m_alpha;
			} else {
				return m_beta;
			}
		}
	};

	template<bool Maximizing>
	Rating minimax(const Node& node, AlphaBeta alphaBeta) {
		auto cachedRating = getPositionRating(node.getPos(), node.getLevelsToSearch());
		if (cachedRating) {
			return *cachedRating;
		}

		auto timeCalculated = std::chrono::steady_clock::now() - beginCalculation;
		if (timeCalculated > MAX_CALCULATION_TIME) {
			return node.getRating();
		}

		if (node.isDone()) {
			return node.getRating();
		}
		
		auto legalMoves = calcAllLegalMoves(node.getPos());
		if (legalMoves.moves.empty()) {
			return legalMoves.isCheckmate ? checkmatedRating<Maximizing>() : 0_rt;
		}
		auto movePriorities = getMovePriorities(node, legalMoves.moves, Maximizing);

		auto bestRating = worstPossibleRating<Maximizing>();
		
		for (const auto& movePriority : movePriorities) {
			auto child = Node::makeChild(node, movePriority);
			auto childRating = minimax<!Maximizing>(child, alphaBeta);
			storePositionRating(child.getPos(), movePriority.getDepth(), childRating);

			bestRating = extreme<Maximizing>(bestRating, childRating);
			alphaBeta.update<Maximizing>(bestRating);
			if (alphaBeta.canPrune()) {
				updateHistoryScore(movePriority.getMove(), node.getLevel());
				break;
			}
		}

		return bestRating;
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, int depth) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(rootPos, depth, rootPos.isWhite());

		auto legalMoves = calcAllLegalMoves(rootPos);
		if (legalMoves.moves.empty()) {
			return std::nullopt;
		}
		std::vector<Move> postponedMoves;

		auto movePriorities = getMovePriorities(root, legalMoves.moves, Maximizing);

		auto bestMove = Move::null();
		for (const auto& movePriority : movePriorities) {
			auto child = Node::makeChild(root, movePriority);
			auto childRating = minimax<!Maximizing>(child, alphaBeta);
			storePositionRating(child.getPos(), movePriority.getDepth(), childRating);
			if (alphaBeta.update<Maximizing>(childRating)) {
				bestMove = movePriority.getMove();
			}
		}

		assert(bestMove != Move::null());

		return bestMove;
	}

	std::optional<Move> findBestMove(const Position& pos, int depth) {
		ProfilerLock l{ getBestMoveProfiler() };

		beginCalculation = std::chrono::steady_clock::now();

		if (pos.getTurnData().isWhite) {
			return bestMoveImpl<true>(pos, depth);
		}
		return bestMoveImpl<false>(pos, depth);
	}
}