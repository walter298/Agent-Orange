export module Chess.Evaluation:Minimax;

export import std;
import Chess.Position;

namespace chess {
	export std::optional<Move> minimax(const Position& pos, int depth);
}