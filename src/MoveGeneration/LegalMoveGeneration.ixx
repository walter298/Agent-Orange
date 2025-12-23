export module Chess.MoveGeneration:LegalMoveGeneration;

import std;

export import Chess.Move;
export import Chess.Position;

export namespace chess {
	PositionData calcPositionData(const Position& pos);
	PositionData calcPositionDataAndDrawBitboards(const Position& pos);
}