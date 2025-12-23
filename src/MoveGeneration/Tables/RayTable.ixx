export module Chess.MoveGeneration:RayTable;

export import Chess.Bitboard;
export import Chess.Square;

export namespace chess {
	Bitboard getRay(Square from, Square to);
}