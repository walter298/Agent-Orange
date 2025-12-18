module Chess.AttackMap;

import Chess.Evaluation;

namespace chess {
    AttackMap::AttackMap(Bitboard attackedSquares, const PieceState& allies)
	    : m_attackedSquares{ attackedSquares }
	{
        for (const auto& pieceType : ALL_PIECE_TYPES | std::views::drop(1)) {
            auto locations = allies[pieceType];
            auto attackedAllies = locations & attackedSquares;
            m_threatenedMaterial -= std::popcount(attackedAllies) * getPieceRating(pieceType);
        }

        std::println("Threatened Material: {}", m_threatenedMaterial);
    }

	Rating AttackMap::materialChange(const Move& move) const {
        auto fromBoard = makeBitboard(move.from);
        auto toBoard = makeBitboard(move.to);

        auto evacuationSave = 0_rt;
        if (fromBoard & m_attackedSquares) { //we have material to either win or lose by moving an attacked piece
            auto movedPieceRating = getPieceRating(move.movedPiece);
            if (toBoard & m_attackedSquares) {
                evacuationSave -= movedPieceRating;
            } else if (move.promotionPiece != Piece::None) {
                evacuationSave += getPieceRating(move.promotionPiece) - getPieceRating(move.movedPiece);
            } else {
                evacuationSave += movedPieceRating;
            }
        }

        auto captureSave = 0_rt;
        if (move.capturedPiece != Piece::None) {
            captureSave += getPieceRating(move.capturedPiece);
        }

        return m_threatenedMaterial + (evacuationSave + captureSave);
    }
}