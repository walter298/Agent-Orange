module;

#include <boost/parser/parser.hpp>

module Chess.UCI;

import std;
import Chess.Evaluation;
import :GameState;

//damn macros are defined somewhere (probably windows.h)
#undef max
#undef min

namespace chess {
	GameState gameState;

	void handlePositionInput(std::istringstream& iss) {
		std::string fen;
		
		std::string token;
		iss >> token;

		std::vector<std::string> moves;

		if (token == "startpos") {
			iss >> token; //consumes "moves"
			fen = Position::STARTING_FEN_STRING;
		} else {
			assert(token == "fen");
			while (iss >> token && token != "moves") {
				fen += token + " ";
			}
			fen.pop_back();
		}

		while (iss >> token) {
			moves.push_back(token);
		}

		gameState.setPos(fen, moves);
	}

	void playUCI() {
		std::istringstream iss;
		std::string line;
		std::string token;

		while (std::getline(std::cin, line)) {
			iss.clear();
			iss.str(line);
			iss >> token;

			if (token == "quit") {
				std::exit(EXIT_SUCCESS);
			} else if (token == "position") {
				handlePositionInput(iss);
			} else if (token == "ucinewgame") {
				gameState.reset();
			} else if (token == "isready") {
				std::println("readyok");
			} else if (token == "uci") {
				constexpr auto ENGINE_INFO = "id name Agent Orange\n"
											 "id author Walter Stein-Smith\n"
											 "uciok";
				std::println("{}", ENGINE_INFO);
			} else if (token == "go") {
				std::println("{}", gameState.calcBestMove());
			}
		}
	}
}