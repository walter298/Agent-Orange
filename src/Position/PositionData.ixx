export module Chess.Position:PositionData;

export import std;
export import Chess.Bitboard;
export import Chess.Move;

export namespace chess {
	struct PositionData {
		std::vector<Move> legalMoves;
		Bitboard attackedPieces = 0;
		bool isCheck = false;
		bool isCheckmate = false;
	};
}