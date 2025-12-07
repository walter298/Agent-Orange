export module Chess.LegalMoveGeneration:PawnMoveGeneration;

export import Chess.Square;
export import :MoveGen;

namespace chess {
	export struct EnPessantData {
		Bitboard pawns = 0;
		Square squareInFrontOfEnemyPawn = None;
	};

	struct WhitePawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard empty) const;
	};
	struct WhitePawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard empty) const;
	};
	struct BlackPawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard empty) const;
	};
	struct BlackPawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard empty) const;
	};
	export constexpr WhitePawnMoveGenerator whitePawnMoveGenerator;
	export constexpr BlackPawnMoveGenerator blackPawnMoveGenerator;
	export constexpr WhitePawnAttackGenerator whitePawnAttackGenerator;
	export constexpr BlackPawnAttackGenerator blackPawnAttackGenerator;
}