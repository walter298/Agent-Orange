module;

#include <cassert>

module Chess.MoveSearch;

import std;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Position;
import Chess.Rating;

import :MoveHistory;
import :MovePriority;
import :PositionTable;
import :ThreadPool;

namespace chess {
	int startingDepth = 0;
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

	class Node {
	private:
		Position m_pos;
		Rating m_materialRating = 0;
		bool m_inCaptureSequence = false;
	
		Node() = default;
	public:
		static Node makeRoot(const Position& root) {
			Node ret;
			ret.m_pos = root;
			ret.m_materialRating = staticEvaluation(root);
			return ret;
		}
		static Node makeChild(const Node& parent, const Move& move) {
			Node ret;
			ret.m_pos = { parent.m_pos, move }; //initialize BEFORE calculating its rating
			ret.m_materialRating = staticEvaluation(ret.m_pos);
			ret.m_inCaptureSequence = (move.capturedPiece != Piece::None) || move.promotionPiece != Piece::None;
			return ret;
		}
		const Position& getPos() const {
			return m_pos;
		}
		Rating getRating() const {
			return m_materialRating;
		}
		bool inCaptureSequence() const {
			return m_inCaptureSequence;
		}
	};

	template<bool Maximizing>
	Rating minimax(const Node& node, int depth, AlphaBeta alphaBeta) {
		auto cachedRating = getPositionRating(node.getPos(), depth);
		if (cachedRating) {
			return *cachedRating;
		}

		auto timeCalculated = std::chrono::steady_clock::now() - beginCalculation;
		if (timeCalculated > MAX_CALCULATION_TIME || (depth <= 0 && !node.inCaptureSequence())) {
			return node.getRating();
		}

		auto legalMoves = calcAllLegalMoves(node.getPos());
		if (legalMoves.moves.empty()) {
			return legalMoves.isCheckmate ? checkmatedRating<Maximizing>() : 0rt;
		}
		auto movePriorities = getMovePriorities(legalMoves.moves, depth - 1);

		auto bestRating = worstPossibleRating<Maximizing>();
		auto distFromRoot = std::abs(startingDepth - depth);

		for (const auto& [move, recommendedDepth] : movePriorities) {
			auto child = Node::makeChild(node, move);
			auto childRating = minimax<!Maximizing>(child, recommendedDepth, alphaBeta);
			storePositionRating(child.getPos(), recommendedDepth, childRating);

			bestRating = extreme<Maximizing>(bestRating, childRating);
			alphaBeta.update<Maximizing>(bestRating);
			if (alphaBeta.canPrune()) {
				updateHistoryScore(move, distFromRoot);
				break;
			}
		}

		return bestRating;
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, int depth) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(rootPos);

		auto legalMoves = calcAllLegalMoves(rootPos);
		if (legalMoves.moves.empty()) {
			return std::nullopt;
		}
		auto movePriorities = getMovePriorities(legalMoves.moves, depth - 1);

		auto bestMove = Move::null();
		for (const auto& [move, recommendedDepth] : movePriorities) {
			auto child = Node::makeChild(root, move);
			auto childRating = minimax<!Maximizing>(child, recommendedDepth, alphaBeta);
			storePositionRating(child.getPos(), recommendedDepth, childRating);
			if (alphaBeta.update<Maximizing>(childRating)) {
				bestMove = move;
			}
		}

		assert(bestMove != Move::null());

		return bestMove;
	}

	std::optional<Move> findBestMove(const Position& pos, int depth) {
		startingDepth = depth;
		beginCalculation = std::chrono::steady_clock::now();

		if (pos.getTurnData().isWhite) {
			return bestMoveImpl<true>(pos, depth);
		}
		return bestMoveImpl<false>(pos, depth);
	}
}