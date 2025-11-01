export module Chess.MoveSearch;

export import std;
import Chess.Position;

namespace chess {
	export std::optional<Move> findBestMove(const Position& pos, int depth);
}