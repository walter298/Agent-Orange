module Chess.Evaluation;

import Chess.EasyRandom;
import Chess.LegalMoveGeneration;
import :Minimax;

namespace chess {
	std::optional<Move> bestMove(const Position& pos, int depth) {
		return minimax(pos, depth);
	}
}
