export module Chess.LegalMoveGeneration:PieceLocations;

export import Chess.Bitboard;

export namespace chess {
	struct PieceLocationData {
		Bitboard allyKing;
		Bitboard allies;
		Bitboard enemies;
		Bitboard empty;

		PieceLocationData(Bitboard allyKingPos, Bitboard allies, Bitboard enemies) :
			allyKing{ allyKingPos }, allies{ allies }, enemies{ enemies }, empty{ ~(allies | enemies) }
		{
		}
	};
}