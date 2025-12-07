export module Chess.LegalMoveGeneration;

import std;
export import Chess.Move;
export import Chess.Position;

export namespace chess {
	PositionData calcAllLegalMoves(const Position& pos);
	PositionData calcAllLegalMovesAndDrawBitboards(const Position& pos);
}