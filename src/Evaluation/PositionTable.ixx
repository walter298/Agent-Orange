export module Chess.Evaluation:PositionTable;

export import std;
export import Chess.Position;
export import :Rating;


export namespace chess {
	std::optional<Rating> getPositionRating(const Position& pos, int depth);
	void storePositionRating(const Position& pos, int depth, Rating rating);
}