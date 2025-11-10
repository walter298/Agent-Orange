export module Chess.MoveSearch:MovePriority;

export import Chess.Move;
export import :FixedVector;

namespace chess {
	struct MovePriority {
		Move move;
		int recommendedDepth = 0;
		bool inCaptureSequence = false;
	};

	FixedVector<MovePriority> getMovePriorities(const std::vector<Move>& legalMoves, int depth, bool maximizing);
}