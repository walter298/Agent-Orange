module Chess.AttackMap;

import Chess.Evaluation;

namespace chess {
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

        return evacuationSave + captureSave;
    }
}