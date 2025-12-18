export module Chess.MoveSearch:MoveOrdering;

export import Chess.Move;
export import :FixedVector;
export import :MovePriority;
export import :Node;

export namespace chess {
	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove);
}