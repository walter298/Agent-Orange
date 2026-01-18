export module Chess.MoveSearch:MoveOrdering;

export import Chess.Arena;
export import Chess.Move;
export import :MovePriority;
export import :Node;

export namespace chess {
	arena::Vector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove, std::span<const Move> killerMoves);
}