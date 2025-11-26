module Chess.LegalMoveGeneration:SlidingMoveGeneration;

import Chess.Profiler;

namespace chess {
	template<dir::Direction Direction>
	MoveGen calcSlidingMoves(Bitboard movingPieces, Bitboard empty) {
		MoveGen ret;

		for (int i = 0; i < 7; i++) {
			movingPieces = Direction::move(movingPieces) & Direction::NON_BORDERS;
			if (!movingPieces) { //if we hit the wraparound border, break
				break;
			}

			ret.emptyDestSquares |= movingPieces & empty;
			ret.nonEmptyDestSquares |= movingPieces & ~empty;

			movingPieces &= empty;
		}
		return ret;
	}

	MoveGen DiagonalMoveGenerator::operator()(Bitboard movingPieces, Bitboard empty) const {
		return calcSlidingMoves<dir::sliding::NorthWest>(movingPieces, empty) |
			   calcSlidingMoves<dir::sliding::NorthEast>(movingPieces, empty) |
			   calcSlidingMoves<dir::sliding::SouthWest>(movingPieces, empty) |
			   calcSlidingMoves<dir::sliding::SouthEast>(movingPieces, empty);
	}

	MoveGen OrthogonalMoveGenerator::operator()(Bitboard movingPieces, Bitboard empty) const {
		return calcSlidingMoves<dir::sliding::North>(movingPieces, empty) |
			   calcSlidingMoves<dir::sliding::East>(movingPieces, empty)  |
			   calcSlidingMoves<dir::sliding::South>(movingPieces, empty) |
			   calcSlidingMoves<dir::sliding::West>(movingPieces, empty);
	}
}