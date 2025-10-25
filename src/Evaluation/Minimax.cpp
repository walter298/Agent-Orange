module Chess.Evaluation:Minimax;

import std;
import Chess.Position;
import Chess.LegalMoveGeneration;
import :PositionTable;
import :Rating;
import :StaticEvaluation;
import :ThreadPool;

namespace chess {
	class AlphaBeta {
	private:
		Rating m_alpha = std::numeric_limits<Rating>::lowest();
		Rating m_beta  = std::numeric_limits<Rating>::max();
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
			ret.m_materialRating = calcMaterialRating(root);
			return ret;
		}
		static Node makeChild(const Node& parent, const Move& move) {
			Node ret;
			ret.m_pos = { parent.m_pos, move }; //initialize BEFORE calculating its rating
			ret.m_materialRating = calcMaterialRating(ret.m_pos);
			ret.m_inCaptureSequence = (move.capturedPiece != Piece::None) || move.promotedPiece != Piece::None;
			return ret;
		}
		const Position& getPos() const {
			return m_pos;
		}
		Rating getMaterialRating() const {
			return m_materialRating;
		}
		bool inCaptureSequence() const {
			return m_inCaptureSequence;
		}
	};

	LegalMoves getMovesInLikelyBestOrder(const Position& pos) {
		auto legalMoves = calcAllLegalMoves(pos);
		std::ranges::partition(legalMoves.moves, [](const Move& move) {
			return move.capturedPiece != Piece::None;
		});
		return legalMoves;
	}

	template<bool Maximizing>
	Rating minimax(const Node& node, int depth, AlphaBeta alphaBeta) {
		auto cachedRating = getPositionRating(node.getPos(), depth);
		if (cachedRating) {
			return *cachedRating;
		}

		if (depth <= 0 && !node.inCaptureSequence()) {
			return node.getMaterialRating();
		}

		auto legalMoves = getMovesInLikelyBestOrder(node.getPos());
		if (legalMoves.moves.empty()) {
			if (legalMoves.isCheckmate) {
				if constexpr (Maximizing) {
					return std::numeric_limits<Rating>::lowest(); 
				} else {
					return std::numeric_limits<Rating>::max();
				}
			}
			return 0.0; //stalemate
		}

		for (const auto& move : legalMoves.moves) {
			auto child = Node::makeChild(node, move);
			auto newDepth = depth - 1;
			auto childRating = minimax<!Maximizing>(child, newDepth, alphaBeta);
			storePositionRating(child.getPos(), newDepth, childRating);

			alphaBeta.update<Maximizing>(childRating);
			if (alphaBeta.canPrune()) { 
				break;
			}
		}

		return alphaBeta.getAllyRating<Maximizing>();
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

		return bestMove;
	}

	std::optional<Move> minimax(const Position& pos, int depth) {
		if (pos.getTurnData().isWhite) {
			return bestMoveImpl<true>(pos, depth);
		}
		return bestMoveImpl<false>(pos, depth);
	}
}