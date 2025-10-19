export module Chess.LegalMoveGeneration:PawnMoveGeneration;

export import Chess.Square;
export import :MoveGen;

namespace chess {
	export struct EnPessantData {
		Bitboard pawns = 0;
		Square destSquare = None;
	};

	export EnPessantData calcEnPessantData(Bitboard pawns, Square enPessantSquare, bool isWhite);

	struct WhitePawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard enemies) const;
	};
	struct WhitePawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard emptySquares, Bitboard enemies) const;
	};
	struct BlackPawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard enemies) const;
	};
	struct BlackPawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard emptySquares, Bitboard enemies) const;
	};
	export WhitePawnMoveGenerator whitePawnMoveGenerator;
	export BlackPawnMoveGenerator blackPawnMoveGenerator;
	export WhitePawnAttackGenerator whitePawnAttackGenerator;
	export BlackPawnAttackGenerator blackPawnAttackGenerator;
}