export module Chess.LegalMoveGeneration;

import std;
export import Chess.Move;
export import Chess.Position;

export namespace chess {
	struct LegalMoves {
		std::vector<Move> moves;
		bool isCheckmate = false;
	};
	LegalMoves calcAllLegalMoves(const Position& pos);
	LegalMoves calcAllLegalMovesAndDrawBitboards(const Position& pos);
}