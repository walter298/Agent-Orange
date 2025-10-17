module Chess.Evaluation;

import Chess.EasyRandom;
import Chess.LegalMoveGeneration;

namespace chess {
	std::optional<Move> bestMove(const Position& pos, int depth) {
		auto legalMoves = calcAllLegalMoves(pos);
		if (legalMoves.empty()) {
			return std::nullopt;
		}

		auto moveIndex = makeRandomNum(0uz, legalMoves.size() - 1);
		return legalMoves[moveIndex];
	}
}
