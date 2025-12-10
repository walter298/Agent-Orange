export module Chess.Position:PositionData;

export import std;
export import Chess.ArenaAllocator;
export import Chess.Bitboard;
export import Chess.Move;

export namespace chess {
	struct PositionData {
		arena::Vector<Move> legalMoves;
		Bitboard whiteSquares = 0;
		Bitboard blackSquares = 0;
		bool isCheck = false;
		bool isCheckmate = false;
	};
}