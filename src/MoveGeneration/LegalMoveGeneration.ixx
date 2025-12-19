export module Chess.LegalMoveGeneration;

import std;

export import Chess.Move;
export import Chess.Position;

export import :TableStore;

export namespace chess {
	PositionData calcPositionData(const Position& pos);
	PositionData calcPositionDataAndDrawBitboards(const Position& pos);
}