export module Chess.Evaluation:Material;

export import Chess.Position;
export import Chess.Rating;

namespace chess {
	Rating calcMaterialRating(const Position& pos);
}