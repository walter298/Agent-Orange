export module Chess.MoveSearch:MoveHistory;

export import std;
export import Chess.Move;
export import Chess.Position;

export namespace chess {
	struct MovePriority {
		Move move;
		int recommendedDepth = 0;
	};

	std::vector<MovePriority> getMovePriories(const Position& pos, int maxDepth);
}