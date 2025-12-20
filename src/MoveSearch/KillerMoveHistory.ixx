export module Chess.MoveSearch:KillerMoveHistory;

export import std;
export import Chess.Move;
export import Chess.Position;
export import Chess.Rating;
export import Chess.SafeInt;

export namespace chess {
	void updateHistoryScore(const Move& move, SafeUnsigned<std::uint8_t> remainingDepth);
	Rating getHistoryRating(const Move& move);
}