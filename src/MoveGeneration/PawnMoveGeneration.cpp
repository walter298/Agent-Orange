module Chess.LegalMoveGeneration:PawnMoveGeneration;
import :Direction;
import Chess.RankCalculator;

namespace chess {
	template<dir::Direction BehindEnPessantSquare>
	EnPessantData calcEnPessantDataImpl(Bitboard pawns, Square jumpedEnemyPawn) {
		EnPessantData ret;

		auto addAdjacentSquare = [&](Square square) {
			if (square != Square::None) {
				auto board = makeBitboard(square);
				if (board & pawns) {
					ret.pawns |= board;
				}
			}
		};
		addAdjacentSquare(westSquare(jumpedEnemyPawn));
		addAdjacentSquare(eastSquare(jumpedEnemyPawn));

		if (ret.pawns) {
			ret.destSquare = nextSquare(BehindEnPessantSquare::move(makeBitboard(jumpedEnemyPawn)));
		}
		return ret;
	}

	EnPessantData WhitePawnAttackGenerator::operator()(Bitboard pawns, Square jumpedEnemyPawn) const {
		using namespace dir::sliding;
		return calcEnPessantDataImpl<North>(pawns, jumpedEnemyPawn);
	}
	EnPessantData BlackPawnAttackGenerator::operator()(Bitboard pawns, Square jumpedEnemyPawn) const {
		using namespace dir::sliding;
		return calcEnPessantDataImpl<South>(pawns, jumpedEnemyPawn);
	}

	template<dir::Direction LeftDirection, dir::Direction RightDirection>
	MoveGen generatePawnAttacksImpl(Bitboard pawns, Bitboard enemies) {
		MoveGen ret;
		auto leftAttacks = LeftDirection::move(pawns) & LeftDirection::NON_BORDERS & enemies;
		auto rightAttacks = RightDirection::move(pawns) & RightDirection::NON_BORDERS & enemies;
		ret.nonEmptyDestSquares |= leftAttacks;
		ret.nonEmptyDestSquares |= rightAttacks;

		return ret;
	}

	MoveGen WhitePawnAttackGenerator::operator()(Bitboard pawns, Bitboard enemies) const {
		using namespace dir::sliding;
		return generatePawnAttacksImpl<NorthWest, NorthEast>(pawns, enemies);
	}
	MoveGen BlackPawnAttackGenerator::operator()(Bitboard pawns, Bitboard enemies) const {
		using namespace dir::sliding;
		return generatePawnAttacksImpl<SouthWest, SouthEast>(pawns, enemies);
	}

	template<dir::Direction ForwardDirection, dir::Direction BackwardDirection,
			 Bitboard JUMP_RANK, typename AttackGenerator>
	MoveGen generatePawnMovesImpl(Bitboard pawns, Bitboard empty, Bitboard enemies, AttackGenerator attackGenerator) {
		MoveGen ret;

		//calculate pawn advancements 1 square forward
		auto singleSquareAdvancedPawns = ForwardDirection::move(pawns) & empty;

		//calculate pawn advancements for jumping 2 squares forward
		auto pawnsOnJumpRank = pawns & JUMP_RANK;
		pawnsOnJumpRank &= BackwardDirection::move(singleSquareAdvancedPawns); //make sure that we cannot move over pieces when jumping two squares forward
		auto doubleSquareAdvancedPawns = ForwardDirection::move(ForwardDirection::move(pawnsOnJumpRank)) & empty;

		ret.emptyDestSquares |= doubleSquareAdvancedPawns;
		ret.emptyDestSquares |= singleSquareAdvancedPawns; 
		
		//calculate pawn attacks!
		ret |= attackGenerator(pawns, enemies);

		return ret;
	}

	MoveGen WhitePawnMoveGenerator::operator()(Bitboard pawns, Bitboard empty, Bitboard enemies) const {
		using namespace dir::sliding;
		return generatePawnMovesImpl<North, South, calcRank<2>()>(pawns, empty, enemies, whitePawnAttackGenerator);
	}
	MoveGen BlackPawnMoveGenerator::operator()(Bitboard pawns, Bitboard empty, Bitboard enemies) const {
		using namespace dir::sliding;
		return generatePawnMovesImpl<South, North, calcRank<7>()>(pawns, empty, enemies, blackPawnAttackGenerator);
	}
}