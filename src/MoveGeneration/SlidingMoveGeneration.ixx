export module Chess.LegalMoveGeneration:SlidingMoveGeneration;

import std;

import Chess.Bitboard;
import Chess.Square;
import :Direction;
export import :MoveGen;

namespace chess {
	struct DiagonalMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};
	struct OrthogonalMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const;
	};

	export DiagonalMoveGenerator diagonalMoveGenerator;
	export OrthogonalMoveGenerator orthogonalMoveGenerator;
}