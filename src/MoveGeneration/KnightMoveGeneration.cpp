module Chess.LegalMoveGeneration:KnightMoveGeneration;
import Chess.Direction;

namespace chess {
	template<dir::Direction Direction>
	MoveGen calcKnightJump(Bitboard movingPieces, Bitboard empty) {
		MoveGen ret;
		auto jump = Direction::move(movingPieces) & Direction::NON_BORDERS;
		ret.emptyDestSquares |= (jump & empty);
		ret.nonEmptyDestSquares |= (jump & ~empty);
		return ret;
	}

	MoveGen KnightMoveGenerator::operator()(Bitboard movingPieces, Bitboard empty) const {
		return calcKnightJump<dir::knight::NorthEastEast>(movingPieces, empty) |
			calcKnightJump<dir::knight::NorthNorthEast>(movingPieces, empty)   |
			calcKnightJump<dir::knight::NorthNorthWest>(movingPieces, empty)   |
			calcKnightJump<dir::knight::NorthWestWest>(movingPieces, empty)    |
			calcKnightJump<dir::knight::SouthEastEast>(movingPieces, empty)    |
			calcKnightJump<dir::knight::SouthSouthEast>(movingPieces, empty)   |
			calcKnightJump<dir::knight::SouthSouthWest>(movingPieces, empty)   |
			calcKnightJump<dir::knight::SouthWestWest>(movingPieces, empty);
	}
}