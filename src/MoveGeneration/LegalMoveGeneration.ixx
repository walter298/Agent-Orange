export module Chess.LegalMoveGeneration;

import std;
export import Chess.Move;
export import Chess.Position;

namespace chess {
	export std::vector<Move> calcAllLegalMoves(const Position& pos);
}