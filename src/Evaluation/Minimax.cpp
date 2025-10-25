module Chess.Evaluation:Minimax;

import std;
import Chess.Position;
import Chess.LegalMoveGeneration;
import :PositionTable;
import :Rating;
import :StaticEvaluation;
import :ThreadPool;

namespace chess {
	template<bool Maximizing>
	struct Comp {
		static constexpr decltype(auto) extreme(const auto& a, const auto& b) {
			if constexpr (Maximizing) {
				return std::max(a, b);
			} else {
				return std::min(a, b);
			}
		}
		static constexpr decltype(auto) extreme(auto&& range) {
			if constexpr (Maximizing) {
				return std::ranges::max(range);
			} else {
				return std::ranges::min(range);
			}
		}
		template<std::ranges::viewable_range Range>
		static constexpr decltype(auto) extreme(Range&& range, auto projection) {
			if constexpr (Maximizing) {
				return std::ranges::max(range, {}, projection);
			} else {
				return std::ranges::min(range, {}, projection);
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
			ret.m_inCaptureSequence = (move.capturedPiece != Piece::None);
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

	template<bool Maximizing>
	Rating minimax(const Node& node, int depth);

	template<bool MaximizingNoAlternate>
	auto projectMove(const Node& parent, int depthNoChange) {
		return [&parent, depthNoChange](const Move& move) {
			auto child = Node::makeChild(parent, move);
			auto newDepth = depthNoChange - 1;
			auto childRating = minimax<!MaximizingNoAlternate>(child, newDepth);
			storePositionRating(child.getPos(), newDepth, childRating);
			return childRating;
		};
	}

	template<bool Maximizing>
	Rating minimax(const Node& node, int depth) {
		auto cachedRating = getPositionRating(node.getPos(), depth);
		if (cachedRating) {
			return *cachedRating;
		}

		if (depth <= 0 && !node.inCaptureSequence()) {
			return node.getMaterialRating();
		}

		auto legalMoves = calcAllLegalMoves(node.getPos());
		if (legalMoves.empty()) {
			return node.getMaterialRating();
		}

		auto extractRating = projectMove<Maximizing>(node, depth);
		auto ratings = legalMoves | std::views::transform( extractRating);

		return Comp<Maximizing>::extreme(ratings);
	}

	template<bool Maximizing>
	std::optional<Move> bestMoveImpl(const Position& rootPos, int depth) {
		auto root = Node::makeRoot(rootPos);

		auto legalMoves = calcAllLegalMoves(root.getPos());
		if (legalMoves.empty()) {
			return std::nullopt;
		}

		auto extractRating = projectMove<Maximizing>(root, depth);
		
		return Comp<Maximizing>::extreme(legalMoves, extractRating);
	}

	std::optional<Move> minimax(const Position& pos, int depth) {
		if (pos.getTurnData().isWhite) {
			return bestMoveImpl<true>(pos, depth);
		}
		return bestMoveImpl<false>(pos, depth);
	}
}