module;

#include <magic_enum/magic_enum.hpp>

#define assert_equality(value, expected) \
	{ \
		auto getPrintableValue = [&](auto a) { \
			using namespace std::literals; \
			if constexpr (std::is_enum_v<decltype(a)>) { \
				return magic_enum::enum_name(a); \
			} else if constexpr (std::same_as<Bitboard, decltype(a)>) {\
				return std::format("{:#x}", a);\
			} else if constexpr (std::is_pointer_v<decltype(a)>) {\
				if (a == nullptr) { \
					return "nullptr"s; \
				} else { \
					return std::format("{}", reinterpret_cast<uintptr_t>(a)); \
				}\
			} else { \
				return a; \
			} \
		};\
		if ((value != expected)) { \
			std::print("Test failed: {} != {}.", #value, #expected); \
			std::println(" Expected value: {}", getPrintableValue(expected)); \
		} \
	} \

module Chess.Tests;

import std;

import Chess.BitboardImage;
import Chess.Position;
import Chess.PositionCommand;
import Chess.LegalMoveGeneration;
import Chess.MoveSearch;
import Chess.Profiler;

import :Pipe;

namespace chess {
	namespace tests {
		//illegal move!
		//2025-10-26 00:28:26.320-->1:position startpos moves e2e4 a7a5 d2d4 d7d5 e4d5 d8d5 c2c4

		void testStartPos() {
			Position pos;
			pos.setPos(parsePositionCommand("startpos"));

			auto turnData = pos.getTurnData();
			assert_equality(turnData.isWhite, true);

			auto [white, black] = pos.getColorSides();
			assert_equality(&turnData.allies, &white);
			assert_equality(&turnData.enemies, &black);

			//test pawn locations
			assert_equality(turnData.allies[Pawn], 0x000000000000FF00);
			assert_equality(turnData.enemies[Pawn], 0x00FF000000000000);

			//test king locations
			assert_equality(nextSquare(turnData.allies[King]), E1);
			assert_equality(nextSquare(turnData.enemies[King]), E8);

			//test castling privileges
			assert_equality(turnData.allies.canCastleKingside(), true);
			assert_equality(turnData.allies.canCastleQueenside(), true);
			assert_equality(turnData.enemies.canCastleKingside(), true);
			assert_equality(turnData.enemies.canCastleQueenside(), true);

			//test en pessant square
			assert_equality(turnData.enemies.doubleJumpedPawn, Square::None);

			//test rook locations
			assert_equality(turnData.allies[Rook], 0x0000000000000081);
			assert_equality(turnData.enemies[Rook], 0x8100000000000000);

			//test knight locations
			assert_equality(turnData.allies[Knight], 0x0000000000000042);
			assert_equality(turnData.enemies[Knight], 0x4200000000000000);

			//test bishop locations
			assert_equality(turnData.allies[Bishop], 0x0000000000000024);
			assert_equality(turnData.enemies[Bishop], 0x2400000000000000);
		}

		void testPawnLocations() {
			Position pos;
			auto [white, black] = pos.getColorSides();

			auto testPawnLocation = [&](const char* priorMove, const PieceState& pieces, Square square) {
				auto board = makeBitboard(square);
				if (!(pieces[Pawn] & board)) {
					std::println("Error: after {}: no pawn on {}!", priorMove, magic_enum::enum_name(square));
				}
			};

			pos.setPos(parsePositionCommand("startpos"));

			pos.move("e2e4");
			testPawnLocation("e2e4", white, E4);

			pos.move("a7a5");
			testPawnLocation("a7a5", white, E4);
			testPawnLocation("a7a5", black, A5);

			pos.move("d2d4");
			testPawnLocation("d2d4", white, E4);
			testPawnLocation("d2d4", black, A5);

			pos.move("d7d5");
			testPawnLocation("d7d5", white, E4);
			testPawnLocation("d7d5", black, A5);

			pos.move("e4d5");
			testPawnLocation("e4d5", black, A5);

			pos.move("d8d5");
			testPawnLocation("d8d5", black, A5);

			pos.move("c2c4");
			testPawnLocation("c2c4", black, A5);

			auto legalMoves = calcAllLegalMoves(pos);
			auto illegalMoveIt = std::ranges::find_if(legalMoves.moves, [](const Move& move) {
				return move.from == D5 && move.to == A5;
			});
			if (illegalMoveIt != legalMoves.moves.end()) {
				std::println("Error: found illegal move d5a5 in testPawnLocations!");
			}
		}

		void testIllegalSquares() {
			Position pos;
			pos.setPos(parsePositionCommand("fen rn3b2/pp2p2p/4b3/k1pN1p2/P3pB2/1PP4P/5PP1/3R2K1 b KQkq - 0 1"));

			auto legalMoves = calcAllLegalMoves(pos);
			auto illegalMoveIt = std::ranges::find_if(legalMoves.moves, [](const Move& move) {
				return move.movedPiece == King && move.to == A4;
			});
			if (illegalMoveIt != legalMoves.moves.end()) {
				std::println("testIllegalSquares failed: found illegal king move to enemy-defended A4 square in testIllegalKingSquares!");
			}
		}

		void testThatLegalMovesExist() {
			constexpr auto POSITION_COMMAND = "fen r4b1r/p1p1p1pp/8/2Pp4/PP1PPBk1/6Q1/8/5RK1 b - - 1 31";
			Position pos;
			pos.setPos(parsePositionCommand(POSITION_COMMAND));

			auto moves = calcAllLegalMoves(pos);
			if (moves.moves.empty()) {
				std::println("Error: no legal moves found in testThatLegalMovesExist!");
			}
		}

		void testThatLegalMovesExist2() {
			Position pos;
			pos.setPos(parsePositionCommand("startpos moves g1f3 a7a5 d2d4 d7d5 c1f4 b7b5 d1d3 c8a6 e2e3 b5b4 d3d2 a6f1 e1f1 f7f6 c2c3 g7g5 f4g3 h7h5 h2h4 a5a4 c3b4 g5g4 f3e1 d8d7 e1d3 e8d8 d2c2 f8h6 b1d2 f6f5 a1c1 b8a6 b4b5 d7b5 g3c7 a6c7 c2c7 d8e8 f1e2 b5a6 c7c3 e8d7 e2e1 d7d8 d3e5 d8e8 f2f3 e7e6 e1f2 g8e7 c1c2 a8c8 c3d3 a6d3 c2c8 e7c8 e5d3 h6g7 d3c5 g7d4 e3d4 c8b6 c5e6 e8e7 e6g5 f5f4 g2g3 h8f8 g3f4 f8f4 f2e3 f4f5 b2b3 a4a3 b3b4 b6c4 d2c4 d5c4 b4b5 f5b5 h1c1 b5b4 d4d5 e7d7 g5e4 g4f3 e3f3 b4b5 d5d6 b5b4 f3e3 d7c6 e3d4 c6b6 d6d7 b6c7 c1d1 c7d8 d4c3 b4b2 c3c4 b2h2 e4f6 h2h4 c4b3 h4h3 b3a4 h5h4 d1e1 d8c7 e1e8 h3d3 a4b4 d3d4 b4c3 d4d1 c3c2 d1d4 c2c3 d4d1 c3c4 d1c1 c4b3 c1b1 b3a3 b1d1 a3b2 d1d2 b2b3 d2d3 b3b4 d3d4 b4c5 d4d7 f6d7 c7d7 e8h8 d7e6 h8h4 e6f5 h4h1 f5e4 a2a4 e4f3 a4a5 f3g2 h1a1 g2f2 a5a6 f2e2 a6a7 e2d2 a7a8q d2c2 a8b8 c2d2 c5b6 d2c2 b6a7 c2d2 a7a8 d2c2 a1b1 c2d2 b8d8 d2c2 b1b4 c2c1 d8c7 c1d2 c7c4 d2e3 b4b3 e3d2 c4c3 d2d1 b3b1"));

			auto legalMoves = calcAllLegalMoves(pos);
			if (legalMoves.moves.empty()) {
				std::println("Error: no legal moves found in testThatLegalMovesExist2!");
			}
		}

		void testThatLegalMovesExist3() {
			Position pos;
			pos.setPos(parsePositionCommand("fen KQ6/Q7/8/8/8/8/8/4k3 w - - 0 1 moves a7c7 e1d2 b8d8"));
			
			auto legalMoves = calcAllLegalMoves(pos);
			if (legalMoves.moves.empty()) {
				std::println("Error: no legal moves found in testThatLegalMovesExist3!");
			}
		}

		void testThatLegalMovesExist4() {
			Position pos;
			pos.setPos(parsePositionCommand("startpos"));

			std::string moves;

			for (int i = 0; i < 20; i++) {
				auto bestMove = findBestMove(pos, 6);
				if (!bestMove) {
					std::println("POTENTIAL Error: no best move found in testThatLegalMovesExist4 at ply {}!", i + 1);
					std::println("Moves: {}", moves);
					return;
				}
				pos.move(*bestMove);
				moves += bestMove->getUCIString() + " ";
			}
		}

		void testEnPassant() {
			Position pos;
			pos.setPos(parsePositionCommand("fen 8/8/8/2k5/5p2/8/6P1/3K4 w - - 0 1"));
			pos.move("g2g4");
			pos.move("f4g3");

			auto [white, black] = pos.getColorSides();

			assert_equality(nextSquare(white[Pawn]), Square::None);
			assert_equality(nextSquare(black[Pawn]), G3);
		}

		void testEnPassant2() {
			Position pos;
			pos.setPos(parsePositionCommand("fen rnbqkbnr/pp1ppppp/8/8/2p5/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"));
			pos.move("d2d4");

			auto [white, black] = pos.getColorSides();

			//verify that white has moved the D pawn two squares forward
			assert_equality(white.doubleJumpedPawn, D4);

			auto legalMoves = calcAllLegalMoves(pos);
			auto enPassantMoveIt = std::ranges::find_if(legalMoves.moves, [](const Move& move) {
				return move.capturedPawnSquareEnPassant == D4;
			});
			if (enPassantMoveIt == legalMoves.moves.end()) {
				std::println("testEnPassant2 failed: en passant not among legal moves");
				return;
			}

			auto verifyStateAfterEnPassant = [](const Position& pos) {
				auto [white, black] = pos.getColorSides();
				//verify that black has captured the D pawn en passant
				if (containsSquare(white[Pawn], D4)) {
					std::println("testEnPassant2 failed: white pawn still on d4 after en passant capture");
				}
				if (!containsSquare(black[Pawn], D3)) {
					std::println("testEnPassant2 failed: black pawn not on d3 after en passant capture");
				}
			};

			auto temp = pos;

			pos.move("c4d3");
			verifyStateAfterEnPassant(pos);

			temp.move(*enPassantMoveIt);
			verifyStateAfterEnPassant(temp);

			if (temp != pos) {
				std::println("testEnPassant2 failed: positions after en passant do not match");
			}
		}

		void testPin() {
			Position pos;
			pos.setPos(parsePositionCommand("fen k7/8/8/3r4/8/8/8/rB1BK3 w - - 0 1"));

			auto legalMoveData = calcAllLegalMoves(pos);
			auto d1BishopMove = std::ranges::find_if(legalMoveData.moves, [](const Move& move) {
				return move.from == D1;
			});
			if (d1BishopMove == legalMoveData.moves.end()) {
				std::println("testPin failed: d1 bishop is not pinned but it is not moveable");
			}
		}

		void testPinWithCheck() {
			Position pos;
			pos.setPos(parsePositionCommand("fen 8/8/1r2knR1/2b5/1p2pBBP/1P1p4/5PP1/6K1 b - - 4 44"));

			auto legalMoveData = calcAllLegalMoves(pos);
			auto illegalKnightMove = std::ranges::find_if(legalMoveData.moves, [](const Move& move) {
				return move.from == F6;
			});
			if (illegalKnightMove != legalMoveData.moves.end()) {
				std::println("testPinWithCheck failed: illegal knight move: {}", illegalKnightMove->getUCIString());
			}
		}

		void testBitboardImageCreation() {
			Position pos;
			pos.setPos(parsePositionCommand("startpos"));

			auto [white, black] = pos.getColorSides();
			auto whitePieces = white.calcAllLocations();
			auto blackPieces = black.calcAllLocations();

			auto colorGetter = [&](Bitboard bitboard) -> RGB {
				if (bitboard & whitePieces) {
					return { 255, 255, 255 }; 
				} else if (bitboard & blackPieces) {
					return { 0, 0, 0 };
				} else {
					return { 240, 255, 91 }; 
				}
			};
			drawBitboardImage(colorGetter, "start_position.bmp");
		}

		void doUCIHandshake(const Pipe& pipe) {
			pipe.write("uci\n");
			std::println("{}", pipe.read("id name"));
			pipe.write("ucinewgame\n");
			pipe.write("isready\n");
			std::println("{}", pipe.read("readyok"));
		}

		void testUCIInput() {
			Pipe pipe;
			doUCIHandshake(pipe);

			pipe.write("position startpos moves e2e4\ngo\n");
			std::println("{}", pipe.read("bestmove"));
			
			pipe.write("ucinewgame\nposition startpos moves e2e4 h7h5 d2d4\ngo\n");
			std::println("{}", pipe.read("bestmove"));

			pipe.write("ucinewgame\nposition startpos moves e2e4 h7h5 d2d4 d7d5\ngo\n");
			std::println("{}", pipe.read("bestmove"));

			pipe.write("ucinewgame\nposition startpos moves e2e4 h7h5 d2d4 d7d5 e4d5\ngo\n");
			std::println("{}", pipe.read("bestmove"));
		}

		void runAllTests() {
			std::println("Running tests...");

			testStartPos();
			testPawnLocations();
			testIllegalSquares();
			testThatLegalMovesExist();
			testThatLegalMovesExist2();
			testThatLegalMovesExist3();
			//testThatLegalMovesExist4(); //really really long!
			testEnPassant();
			testEnPassant2();
			testPin();
			testPinWithCheck();
			testBitboardImageCreation();
			testUCIInput();
		}
	}
}