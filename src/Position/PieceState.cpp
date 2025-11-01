module Chess.Position.PieceState;

namespace chess {
    Piece PieceState::findPiece(Square square) const {
        auto board = makeBitboard(square);

        for (auto piece : ALL_PIECE_TYPES) {
            if ((*this)[piece] & board) {
                return piece;
            }
        }
        return Piece::None;
    }
}