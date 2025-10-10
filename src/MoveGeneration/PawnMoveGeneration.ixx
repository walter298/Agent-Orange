export module Chess.LegalMoveGeneration:PawnMoveGeneration;

export import Chess.Square;
export import :MoveGen;

namespace chess {
	struct EnPessantData {
		Bitboard pawns = 0;
		Square capturedPawn = None;
	};

	export EnPessantData calcEnPessantData(Bitboard pawns, Square enPessantSquare, bool isWhite);

	struct WhitePawnAttackGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard enemies) const;
	};
	struct WhitePawnMoveGenerator {
		Bitboard calcEnPessantData(Bitboard pawns, Square enPessantSquare) { return 0; };
		MoveGen operator()(Bitboard pawns, Bitboard emptySquares, Bitboard enemies) const;
	};
	struct BlackPawnAttackGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard enemies) const;
	};
	struct BlackPawnMoveGenerator {
		Bitboard calcEnPessantData(Bitboard pawns, Square enPessantSquare) { return 0; };
		MoveGen operator()(Bitboard pawns, Bitboard emptySquares, Bitboard enemies) const;
	};
	export WhitePawnMoveGenerator whitePawnMoveGenerator;
	export BlackPawnMoveGenerator blackPawnMoveGenerator;
	export WhitePawnAttackGenerator whitePawnAttackGenerator;
	export BlackPawnAttackGenerator blackPawnAttackGenerator;
}