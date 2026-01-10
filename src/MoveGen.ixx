export module Chess.MoveGen;

export import Chess.Bitboard;

export namespace chess {
	struct MoveGen {
		Bitboard emptyDestSquares = 0;
		Bitboard nonEmptyDestSquares = 0;

		MoveGen& operator|=(const MoveGen& other) {
			emptyDestSquares |= other.emptyDestSquares;
			nonEmptyDestSquares |= other.nonEmptyDestSquares;
			return *this;
		}

		MoveGen& operator&=(Bitboard bitboard) {
			emptyDestSquares &= bitboard;
			nonEmptyDestSquares &= bitboard;
			return *this;
		}

		Bitboard all() const {
			return emptyDestSquares | nonEmptyDestSquares;
		}
	};

	MoveGen operator|(MoveGen a, MoveGen b) {
		return { a.emptyDestSquares | b.emptyDestSquares, a.nonEmptyDestSquares | b.nonEmptyDestSquares };
	}
}