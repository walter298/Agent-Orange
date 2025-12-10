export module Chess.Evaluation:KingSafety;

export import Chess.Position;
export import Chess.Rating;

export namespace chess {
	Rating calcKingSafetyRating(const Position& pos, const PositionData& positionData);
}