module Chess.LegalMoveGeneration:KingMoveGeneration;
import :Direction;

namespace chess {
	MoveGen KingMoveGenerator::operator()(Bitboard kingPos, Bitboard empty) const {
		Bitboard squares = 0;
		squares |= (dir::sliding::East::move(kingPos) & dir::sliding::East::NON_BORDERS);
		squares |= (dir::sliding::West::move(kingPos) & dir::sliding::West::NON_BORDERS);
		squares |= (dir::sliding::North::move(kingPos) & dir::sliding::North::NON_BORDERS);
		squares |= (dir::sliding::South::move(kingPos) & dir::sliding::South::NON_BORDERS);
		squares |= (dir::sliding::NorthEast::move(kingPos) & dir::sliding::NorthEast::NON_BORDERS);
		squares |= (dir::sliding::NorthWest::move(kingPos) & dir::sliding::NorthWest::NON_BORDERS);
		squares |= (dir::sliding::SouthEast::move(kingPos) & dir::sliding::SouthEast::NON_BORDERS);
		squares |= (dir::sliding::SouthWest::move(kingPos) & dir::sliding::SouthWest::NON_BORDERS);

		return { .emptyDestSquares = squares & empty, .nonEmptyDestSquares = squares & ~empty };
	}
}