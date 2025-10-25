module Chess.Tests:CaptureTests;

import std;
import Chess.LegalMoveGeneration;
import Chess.Evaluation;

namespace chess {
	namespace tests {
		void test1() {
			constexpr auto FEN = "rnbqkbnr/ppppppp1/B7/7p/4P3/8/PPPP1PPP/RNBQK1NR b KQkq - 1 2";
			Position pos;
			pos.setFen(FEN);

			auto moves = calcAllLegalMoves(pos);
			for (const auto& move : moves.moves) {
				if (move.capturedPiece != Piece::None) {
					std::println("{}", move.getUCIString());
				}
			}

			std::println("----");

			Position startPos;
			startPos.setStartPos();
			startPos.move("e2e4");
			startPos.move("h7h5");
			startPos.move("f1a6");

			auto legalMovesFromStart = calcAllLegalMoves(startPos);
			if (!std::ranges::equal(legalMovesFromStart.moves, moves.moves)) {
				std::println("Error: moves from test position do not match moves from start position after moves!");
			}
			for (const auto& move : legalMovesFromStart.moves) {
				if (move.capturedPiece != Piece::None) {
					std::println("{}", move.getUCIString());
				}
			}

			auto bestMove1 = bestMove(pos, 2);
			auto bestMove2 = bestMove(startPos, 2);
			if (bestMove1 != bestMove2) {
				std::println("Error: best moves do not match!");
			} else if (bestMove1) {
				std::println("Best move: {}", bestMove1->getUCIString());
			} else {
				std::println("No best move found!");
			}
		}

		void runCaptureTests() {
			test1();
		}
	}
}