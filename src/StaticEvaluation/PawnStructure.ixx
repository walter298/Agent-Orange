export module Chess.Evaluation:PawnStructure;

export import Chess.Position;
export import Chess.Rating;

namespace chess {
	Rating calcPawnStructureRating(const Position& pos);
	void runPawnStructureTests();
}