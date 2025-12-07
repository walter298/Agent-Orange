export module Chess.MoveSearch:PositionTable;

export import std;
export import Chess.Position;
export import Chess.Rating;

export namespace chess {
	enum WindowBound {
		InWindow,
		LowerBound,
		UpperBound
	};

	struct PositionEntry {
		Rating rating = 0_rt;
		WindowBound bound = InWindow;
		int depth = 0;
		Move bestMove = Move::null();
	};
	using PositionEntryRef = std::reference_wrapper<const PositionEntry>;

	std::optional<PositionEntryRef> getPositionEntry(const Position& pos);
	void storePositionEntry(const Position& pos, const PositionEntry& entry);
}