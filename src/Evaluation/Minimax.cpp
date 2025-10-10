module Chess.Evaluation:Minimax;

import std;
import Chess.Position;
import Chess.LegalMoveGeneration;
import :StaticEvaluation;

namespace chess {
	Rating minimax(const Position& pos, int depth, bool maximizing) {
		if (depth == 0) {
			return calcPositionRating(pos);
		}
		auto legalMoves = calcAllLegalMoves(pos);

		std::vector<Position> childPositions;

		if (maximizing) {
			
		}
		return 0;
	}
}