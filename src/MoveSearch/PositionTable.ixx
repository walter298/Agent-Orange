export module Chess.MoveSearch:PositionTable;

export import std;
export import Chess.Position;
export import Chess.Rating;
export import Chess.SafeInt;

export import :MovePriority;

export namespace chess {
	enum WindowBound : std::uint8_t {
		InWindow,
		LowerBound,
		UpperBound
	};

	struct PositionEntry {
		Move bestMove = Move::null();
		Rating rating = 0_rt;
		SafeUnsigned<std::uint8_t> depth{ 0 };
		WindowBound bound = InWindow;
	};
	using PositionEntryRef = std::reference_wrapper<const PositionEntry>;

	std::optional<PositionEntryRef> getPositionEntry(const Position& pos);
	void storePositionEntry(const Position& pos, const PositionEntry& entry);
}