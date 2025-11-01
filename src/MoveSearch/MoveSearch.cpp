module;

#include <cassert>

module Chess.MoveSearch;

import std;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;
import Chess.Position;
import Chess.Rating;

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

	LegalMoves getMovesInLikelyBestOrder(const Position& pos) {
		auto legalMoves = calcAllLegalMoves(pos);
		auto [unpartitionedBegin, end] = std::ranges::partition(legalMoves.moves, [](const Move& move) {
			if (move.capturedPiece == Piece::None) {
				return false;
			}
			return getPieceRating(move.movedPiece) <= getPieceRating(move.capturedPiece);
		});
		auto captures = std::ranges::subrange(legalMoves.moves.begin(), unpartitionedBegin);
		std::ranges::sort(captures, [](const Move& a, const Move& b) {
			auto diff1 = getPieceRating(a.capturedPiece) - getPieceRating(a.movedPiece);
			auto diff2 = getPieceRating(b.capturedPiece) - getPieceRating(b.movedPiece);
			return diff1 > diff2;
		});
		return legalMoves;
	}

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

		auto legalMoves = getMovesInLikelyBestOrder(node.getPos());
		if (legalMoves.moves.empty()) {
			return legalMoves.isCheckmate ? checkmatedRating<Maximizing>() : 0.0;
		}

		auto bestRating = worstPossibleRating<Maximizing>();

		for (const auto& move : legalMoves.moves) {
			auto child = Node::makeChild(node, move);
			auto newDepth = depth - 1;
			auto childRating = minimax<!Maximizing>(child, newDepth, alphaBeta);
			storePositionRating(child.getPos(), newDepth, childRating);

			bestRating = extreme<Maximizing>(bestRating, childRating);
			alphaBeta.update<Maximizing>(bestRating);
			if (alphaBeta.canPrune()) { 
				break;
			}
		}

		return bestRating;
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, int depth) {
		AlphaBeta alphaBeta;
		auto root = Node::makeRoot(rootPos);

		auto legalMoves = getMovesInLikelyBestOrder(root.getPos());
		if (legalMoves.moves.empty()) {
			return std::nullopt;
		}

		auto bestMove = Move::null();
		for (const auto& move : legalMoves.moves) {
			auto child = Node::makeChild(root, move);
			auto newDepth = depth - 1;
			auto childRating = minimax<!Maximizing>(child, newDepth, alphaBeta);
			storePositionRating(child.getPos(), newDepth, childRating);
			if (alphaBeta.update<Maximizing>(childRating)) {
				bestMove = move;
			}
		}

		assert(bestMove != Move::null());

		return bestMove;
	}

	std::optional<Move> findBestMove(const Position& pos, int depth) {
		beginCalculation = std::chrono::steady_clock::now();

		if (pos.getTurnData().isWhite) {
			return bestMoveImpl<true>(pos, depth);
		}
		return bestMoveImpl<false>(pos, depth);
	}
}