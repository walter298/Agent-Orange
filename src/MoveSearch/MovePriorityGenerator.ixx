export module Chess.MoveSearch:MovePriorityGeneration;

export import Chess.Move;
export import :FixedVector;
export import :MovePriority;
export import :Node;

export namespace chess {
	FixedVector<MovePriority> getMovePriorities(const Node& node, const Move& pvMove);
}