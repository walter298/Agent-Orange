export module Chess.Evaluation:StaticEvaluation;

import Chess.Position;
export import :Rating;

export namespace chess {
	Rating calcMaterialRating(const Position& pos);
}