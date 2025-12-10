export module Chess.LegalMoveGeneration:SlidingMoveGeneration;

import std;

import Chess.Bitboard;
import Chess.Square;
import :ChainedMoveGenerator;
import Chess.Direction;
export import :MoveGen;

export namespace chess {
	template<dir::Direction Direction>
	struct SlidingMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const {
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
	};

	constexpr SlidingMoveGenerator<dir::sliding::NorthWest> northWestSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::NorthEast> northEastSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::SouthWest> southWestSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::SouthEast> southEastSlidingMoveGenerator;

	constexpr ChainedMoveGenerator diagonalMoveGenerator{
		northWestSlidingMoveGenerator,
		northEastSlidingMoveGenerator,
		southWestSlidingMoveGenerator,
		southEastSlidingMoveGenerator
	};

	constexpr SlidingMoveGenerator<dir::sliding::North> northSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::East> eastSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::South> southSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::West> westSlidingMoveGenerator;

	constexpr ChainedMoveGenerator orthogonalMoveGenerator{
		northSlidingMoveGenerator,
		eastSlidingMoveGenerator,
		southSlidingMoveGenerator,
		westSlidingMoveGenerator
	};
}