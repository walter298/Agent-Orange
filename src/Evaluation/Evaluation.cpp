module Chess.Evaluation;

import std;
import Chess.EasyRandom;
import Chess.LegalMoveGeneration;

namespace chess {
	/*bool isCheckmate(const Position& pos) {
		auto copy = pos;
		auto legalMoves = calcAllLegalMoves(copy);
		if (legalMoves.anyLegalMoves()) {
			return false;
		}
		auto [allies, enemies] = pos.getTurnSides();
		auto enemySquares = 
	}*/

	Move bestMove(const Position& pos, int depth) {
		auto legalMoves = calcAllLegalMoves(pos);
		if (legalMoves.empty()) {
			std::exit(0);
		}

		auto moveIndex = makeRandomNum(0uz, legalMoves.size() - 1);
		return legalMoves[moveIndex];
	}
}
