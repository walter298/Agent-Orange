export module Chess.LegalMoveGeneration:Magic;

export import Chess.Square;
export import Chess.Bitboard;
export import :ChainedMoveGenerator;

export namespace chess {
	void generateMagicBitboardTable();

	struct BishopAttackGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};
	constexpr BishopAttackGenerator bishopMoveGenerator;

	struct RookAttackGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};
	constexpr RookAttackGenerator rookMoveGenerator;

	constexpr ChainedMoveGenerator queenMoveGenerator{
		bishopMoveGenerator, rookMoveGenerator
	};
}