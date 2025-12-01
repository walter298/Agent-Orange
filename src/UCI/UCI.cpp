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

	std::string getRemainingTokens(std::istringstream& iss) {
		auto buff = iss.str();
		auto currentPos = static_cast<size_t>(iss.tellg());
		return buff.substr(currentPos);
	}

	void playUCI(int depth) {
		std::istringstream iss;
		std::string line;
		std::string token;

		gameState.depth = depth;

		while (std::getline(std::cin, line)) {
			iss.clear();
			iss.str(line);
			iss >> token;

			if (token == "quit") {
				break;
			} else if (token == "position") {
				std::println("Pos: {}", iss.str());
				gameState.setPos(getRemainingTokens(iss));
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