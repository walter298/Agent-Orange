export module Chess.MoveSearch:PositionTable;

export import std;
export import Chess.Position;
export import Chess.Rating;

export namespace chess {
	std::optional<Rating> getPositionRating(const Position& pos, int depth);
	void storePositionRating(const Position& pos, int depth, Rating rating);
	void improveHistoricalMoveRating(const Move& move, Rating rating);
}