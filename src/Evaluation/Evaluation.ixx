export module Chess.Evaluation;

export import Chess.Move;
export import Chess.Position;

namespace chess {
	export Move bestMove(const Position& pos, int depth);
}