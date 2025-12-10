export module Chess.Evaluation;

import Chess.Position;
export import Chess.Rating;
export import :InternalTests;

export namespace chess {
	Rating getPieceRating(Piece piece);
	Rating staticEvaluation(const Position& pos, const PositionData& positionData);
}