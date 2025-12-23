export module Chess.Position:PositionData;

export import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.PieceMap;
export import Chess.Rating;

export namespace chess {
	using MoveVector = std::vector<Move>;

	struct PositionData {
		MoveVector legalMoves;
		Bitboard whiteSquares = 0;
		Bitboard blackSquares = 0;
		bool isCheck = false;
		bool isCheckmate = false;
	};
}