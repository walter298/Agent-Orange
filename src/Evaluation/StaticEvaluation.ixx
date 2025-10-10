export module Chess.Evaluation:StaticEvaluation;

import Chess.Position;
export import :Rating;

export namespace chess {
	Rating calcPositionRating(const Position& pos);
}