module Chess.MoveGeneration:KingMoveGeneration;

import Chess.SquareZone;

namespace chess {
	MoveGen KingMoveGenerator::operator()(Bitboard kingPos, Bitboard empty) const {
		auto squares = calcSquareZone(kingPos);
		return { .emptyDestSquares = squares & empty, .nonEmptyDestSquares = squares & ~empty };
	}
}