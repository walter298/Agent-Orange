export module Chess.MoveSearch:MoveHistory;

export import std;
export import Chess.Move;
export import Chess.Position;
export import Chess.Rating;

export namespace chess {
	void updateHistoryScore(const Move& move, int distFromRoot);
	Rating getHistoryRating(const Move& move);
}