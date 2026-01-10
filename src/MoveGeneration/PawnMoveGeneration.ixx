export module Chess.MoveGeneration:PawnMoveGeneration;

export import std;
export import Chess.Square;
export import Chess.MoveGen;

namespace chess {
	export struct EnPessantData {
		Bitboard pawns = 0;
		Square squareInFrontOfEnemyPawn = None;
	};

	struct WhitePawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard enemyPieces) const;
	};
	struct WhitePawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard empty, Bitboard enemyPieces) const;
	};
	struct BlackPawnAttackGenerator {
		EnPessantData operator()(Bitboard pawns, Square jumpedEnemyPawn) const;
		MoveGen operator()(Bitboard pawns, Bitboard enemyPieces) const;
	};
	struct BlackPawnMoveGenerator {
		MoveGen operator()(Bitboard pawns, Bitboard empty, Bitboard enemyPieces) const;
	};
	export constexpr WhitePawnMoveGenerator whitePawnMoveGenerator;
	export constexpr BlackPawnMoveGenerator blackPawnMoveGenerator;
	export constexpr WhitePawnAttackGenerator whitePawnAttackGenerator;
	export constexpr BlackPawnAttackGenerator blackPawnAttackGenerator;

	template<typename T>
	concept PawnMoveGenerator =
		std::same_as<std::remove_cvref_t<T>, WhitePawnAttackGenerator> ||
		std::same_as<std::remove_cvref_t<T>, WhitePawnMoveGenerator> ||
		std::same_as<std::remove_cvref_t<T>, BlackPawnAttackGenerator> ||
		std::same_as<std::remove_cvref_t<T>, BlackPawnMoveGenerator>;

	static_assert(PawnMoveGenerator<WhitePawnMoveGenerator>);
	static_assert(PawnMoveGenerator<WhitePawnAttackGenerator>);
	static_assert(PawnMoveGenerator<BlackPawnMoveGenerator>);
	static_assert(PawnMoveGenerator<BlackPawnAttackGenerator>);

//	template<typename MoveGenerator>
//	static MoveGen invokeAttackGenerator(Bitboard movingEnemies, Bitboard empty, MoveGenerator attackGen) {
//		if constexpr (PawnMoveGenerator<MoveGenerator>) {
//			return attackGen(movingEnemies, ALL_SQUARES);
//		} else {
//			return attackGen(movingEnemies, empty);
//		}
//	}
}