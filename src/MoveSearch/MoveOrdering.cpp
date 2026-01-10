module Chess.MoveSearch:MoveOrdering;

import Chess.Assert;
import Chess.Rating;
import Chess.Evaluation;
import Chess.MoveGeneration;

import Chess.Profiler;
import :KillerMoveHistory;
import :PositionTable;

namespace chess {
	struct PieceData {
		Piece piece = Piece::None;
		Square square = Square::None;
	};

	void deprioritizeNonEvasionMoves(const PieceData& attackedPiece, const Position::ImmutableTurnData& turnData,
		Bitboard empty, std::vector<MovePriority>& priorities)
	{
		auto attackerData = calcAttackers(turnData.isWhite, turnData.enemies, empty, makeBitboard(attackedPiece.square));
		auto attackers = attackerData.attackers.calcAllLocations();

		auto pieceRating = getPieceRating(attackedPiece.piece);
		
		std::ranges::stable_partition(priorities, [&](const MovePriority& p) {
			if (p.getExchangeRating() >= pieceRating) { //if we have a better capture, do it
				return true;
			}
			auto toBoard = makeBitboard(p.getMove().to);
			return static_cast<bool>(toBoard & attackers) || static_cast<bool>(toBoard & attackerData.allRays());
		});
	}

	std::generator<PieceData> getTargets(const PieceState& allies, Bitboard enemyDestSquares) {
		constexpr std::array MOST_VALUABLE_PIECES{ Queen, Rook, Bishop, Knight, Pawn };
		for (auto piece : MOST_VALUABLE_PIECES) {
			auto attackedAllies = allies[piece] & enemyDestSquares;
			if (!attackedAllies) {
				continue;
			}
			co_yield{ piece, nextSquare(attackedAllies) };
		}
	}

	void deprioritizeNonEvasionMoves(const Node& node, Bitboard allEnemySquares, std::vector<MovePriority>& movePriorities) {
		auto turnData = node.getPos().getTurnData();
		
		auto empty = ~(turnData.enemies.calcAllLocations() | turnData.allies.calcAllLocations());
		for (auto attackedPiece : getTargets(turnData.allies, allEnemySquares)) {
			deprioritizeNonEvasionMoves(attackedPiece, turnData, empty, movePriorities);
		}
	}

	FixedVector<MovePriority> getMovePrioritiesImpl(const Node& node, const Move& pvMove) {
		zAssert(node.getRemainingDepth() != 0_su8);

		const auto& posData = node.getPositionData();
		auto allEnemySquares = node.getPositionData().allEnemySquares();

		std::vector priorities{ std::from_range, posData.legalMoves | std::views::transform([&](const Move& move) {
			return MovePriority{ move, allEnemySquares, node.getRemainingDepth() - 1_su8 };
		}) };

		std::ranges::sort(priorities, [](const MovePriority& a, const MovePriority& b) {
			return a.getExchangeRating() > b.getExchangeRating();
		});

		deprioritizeNonEvasionMoves(node, allEnemySquares, priorities);

		if (pvMove != Move::null() && std::ranges::contains(posData.legalMoves, pvMove)) {
			auto pvMoveIt = std::ranges::find_if(priorities, [&](const MovePriority& p) {
				return p.getMove() == pvMove;
			});
			if (pvMoveIt == priorities.end()) {
				priorities.insert(priorities.begin(), MovePriority{ pvMove, allEnemySquares, node.getRemainingDepth() - 1_su8 });
			} else {
				std::iter_swap(priorities.begin(), pvMoveIt);
			}
		}

		zAssert(!priorities.empty());

		return FixedVector{ std::move(priorities) };
	}

	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove) {
		ProfilerLock l{ getMovePrioritiesProfiler() };
		return getMovePrioritiesImpl(node, pvMove);
	}
}