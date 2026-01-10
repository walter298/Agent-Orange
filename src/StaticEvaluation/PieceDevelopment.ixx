export module Chess.Evaluation:PieceDevelopment;

export import Chess.Rating;
export import Chess.Position;

export namespace chess {
	Rating calcPieceDevelopmentRating(const Position& pos, const PositionData& posData);
}