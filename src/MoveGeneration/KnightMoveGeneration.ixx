export module Chess.LegalMoveGeneration:KnightMoveGeneration;

export import :MoveGen;

namespace chess {
	struct KnightMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};
	export KnightMoveGenerator knightMoveGenerator;
}