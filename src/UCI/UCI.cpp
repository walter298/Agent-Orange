module;

#include <cstdio>

module Chess.UCI;

import std;

import Chess.DebugPrint;
import Chess.Evaluation;
import Chess.Position.RepetitionMap;

import :GameState;

namespace chess {
	std::string getRemainingTokens(std::istringstream& iss) {
		auto buff = iss.str();
		auto currentPos = static_cast<size_t>(iss.tellg());
		return buff.substr(currentPos);
	}

	void playUCI(SafeUnsigned<std::uint8_t> depth) {
		Engine engine{ depth };

		std::istringstream iss;
		std::string line;
		std::string token;

		while (true) {
			if (!std::getline(std::cin, line)) {
				std::println("No more input. EOF: {}", std::cin.eof());
				break;
			}

			debugPrint(std::format("{}", line)); //DOES NOT SEND TO stdout
			
			iss.clear();
			iss.str(line);
			if (!(iss >> token)) {
				continue;
			}
			
			if (token == "quit") {
				break;
			} else if (token == "position") {
				engine.setPos(getRemainingTokens(iss));
			} else if (token == "ucinewgame") {
				engine.stopCalculating();
			} else if (token == "isready") {
				debugPrint("readyok");
				std::printf("readyok\n");
				std::fflush(stdout);
			} else if (token == "uci") {
				constexpr auto ENGINE_INFO = "id name Agent Smith\n"
											 "id author Walter Stein-Smith\n"
											 "uciok\n";
				debugPrint(ENGINE_INFO);
				std::printf(ENGINE_INFO);
				std::fflush(stdout);
			} else if (token == "go") {
				engine.printBestMoveAsync(); 
			} else if (token == "stop") {
				engine.stopCalculating();
			}
		}
	}
}