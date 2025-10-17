export module Chess.Evaluation;

export import std;
export import Chess.Move;
export import Chess.Position;

namespace chess {
	export std::optional<Move> bestMove(const Position& pos, int depth);
}