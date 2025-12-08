module Chess.UCI;

import std;
import Chess.Evaluation;
import :GameState;

namespace chess {
	GameState gameState;

	std::string getRemainingTokens(std::istringstream& iss) {
		auto buff = iss.str();
		auto currentPos = static_cast<size_t>(iss.tellg());
		return buff.substr(currentPos);
	}

	void playUCI(std::uint8_t depth) {
		std::println("Running with depth {}", depth);

		std::istringstream iss;
		std::string line;
		std::string token;

		gameState.depth = depth;

		while (true) {
			if (!std::getline(std::cin, line)) {
				std::println("No more input. EOF: {}", std::cin.eof());
				break;
			}
			
			iss.clear();
			iss.str(line);
			iss >> token;

			if (token == "quit") {
				break;
			} else if (token == "position") {
				std::println("Handling Position: {}", iss.str());
				gameState.setPos(getRemainingTokens(iss));
				std::println("Set the position!");
			} else if (token == "ucinewgame") {
				gameState.reset();
			} else if (token == "isready") {
				std::println("readyok");
			} else if (token == "uci") {
				constexpr auto ENGINE_INFO = "id name Agent B\n"
											 "id author Walter Stein-Smith\n"
											 "uciok";
				std::println("{}", ENGINE_INFO);
			} else if (token == "go") {
				std::println("{}", gameState.calcBestMove());
			}
		}
	}
}