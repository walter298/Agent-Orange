module Chess.MoveGeneration:PawnMoveGeneration;
import Chess.Direction;
import Chess.RankCalculator;

namespace chess {
	template<dir::Direction FrontOfJumpedEnemyPawn>
	EnPessantData calcEnPessantDataImpl(Bitboard pawns, Square jumpedEnemyPawn) {
		EnPessantData ret;

		auto addAdjacentSquare = [&](Square square) {
			if (square != None) {
				auto board = makeBitboard(square);
				if (board & pawns) {
					ret.pawns |= board;
				}
			}
		};
		addAdjacentSquare(westSquare(jumpedEnemyPawn));
		addAdjacentSquare(eastSquare(jumpedEnemyPawn));

		if (ret.pawns) {
			ret.squareInFrontOfEnemyPawn = nextSquare(FrontOfJumpedEnemyPawn::move(makeBitboard(jumpedEnemyPawn)));
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
	MoveGen generatePawnAttacksImpl(Bitboard pawns, Bitboard enemySquares) {
		MoveGen ret;
		auto leftAttacks = LeftDirection::move(pawns) & LeftDirection::NON_BORDERS;
		auto rightAttacks = RightDirection::move(pawns) & RightDirection::NON_BORDERS;
		ret.nonEmptyDestSquares |= leftAttacks & enemySquares;
		ret.nonEmptyDestSquares |= rightAttacks & enemySquares;
		
		return ret;
	}

	MoveGen WhitePawnAttackGenerator::operator()(Bitboard pawns, Bitboard enemyPieces) const {
		using namespace dir::sliding;
		return generatePawnAttacksImpl<NorthWest, NorthEast>(pawns, enemyPieces);
	}
	MoveGen BlackPawnAttackGenerator::operator()(Bitboard pawns, Bitboard enemyPieces) const {
		using namespace dir::sliding;
		return generatePawnAttacksImpl<SouthWest, SouthEast>(pawns, enemyPieces);
	}

	template<dir::Direction ForwardDirection, dir::Direction BackwardDirection,
			 Bitboard JUMP_RANK, typename AttackGenerator>
	MoveGen generatePawnMovesImpl(Bitboard pawns, Bitboard empty, Bitboard enemyPieces, AttackGenerator attackGenerator) {
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
		ret |= attackGenerator(pawns, enemyPieces);

		return ret;
	}

	MoveGen WhitePawnMoveGenerator::operator()(Bitboard pawns, Bitboard empty, Bitboard enemyPieces) const {
		using namespace dir::sliding;
		return generatePawnMovesImpl<North, South, calcRank<2>()>(pawns, empty, enemyPieces, whitePawnAttackGenerator);
	}
	MoveGen BlackPawnMoveGenerator::operator()(Bitboard pawns, Bitboard empty, Bitboard enemyPieces) const {
		using namespace dir::sliding;
		return generatePawnMovesImpl<South, North, calcRank<7>()>(pawns, empty, enemyPieces, blackPawnAttackGenerator);
	}
}