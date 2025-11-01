module Chess.MoveSearch:MoveHistory;

import Chess.Evaluation;
import Chess.LegalMoveGeneration;

namespace chess {
	/*LegalMoves getMovesInLikelyBestOrder(const Position& pos) {
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
			return diff1 < diff2;
		});
		return legalMoves;
	}*/

	std::vector<MovePriority> getMovePriories(const Position& pos, int maxDepth) {
		return {};
	}
}