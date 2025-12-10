module Chess.SquareZone;

import Chess.Direction;

namespace chess {
	Bitboard calcSquareZone(Bitboard square) {
		Bitboard squares = 0;
		squares |= (dir::sliding::East::move(square) & dir::sliding::East::NON_BORDERS);
		squares |= (dir::sliding::West::move(square) & dir::sliding::West::NON_BORDERS);
		squares |= (dir::sliding::North::move(square) & dir::sliding::North::NON_BORDERS);
		squares |= (dir::sliding::South::move(square) & dir::sliding::South::NON_BORDERS);
		squares |= (dir::sliding::NorthEast::move(square) & dir::sliding::NorthEast::NON_BORDERS);
		squares |= (dir::sliding::NorthWest::move(square) & dir::sliding::NorthWest::NON_BORDERS);
		squares |= (dir::sliding::SouthEast::move(square) & dir::sliding::SouthEast::NON_BORDERS);
		squares |= (dir::sliding::SouthWest::move(square) & dir::sliding::SouthWest::NON_BORDERS);
		return squares;
	}
}