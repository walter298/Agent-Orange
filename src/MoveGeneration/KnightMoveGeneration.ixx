export module Chess.MoveGeneration:KnightMoveGeneration;

export import Chess.MoveGen;

namespace chess {
	struct KnightMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};
	export constexpr KnightMoveGenerator knightMoveGenerator;
}