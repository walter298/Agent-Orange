module;

#include <boost/parser/parser.hpp>

module Chess.UCI;

import std;
import Chess.Evaluation;
import :Handshake;

namespace chess {
	void getMoveInput(std::string& line) {
		while (true) {
			std::getline(std::cin, line);
			if (line.starts_with("position")) {
				break;
			}
		}
	}

	void playUCI() {
		int depth = 5;
		std::string line;

		auto pos = parseHandshakePosition();

		while (true) {
			auto engineMove = bestMove(pos, depth);
			std::println("{}", engineMove.getUCIString());
			pos.move(engineMove);

			getMoveInput(line);

			if (line == "quit") {
				std::exit(0);
			}

			std::string_view moveStr{ line.data() + (line.size() - 5), 5 };
			if (std::isspace(moveStr[0])) {
				moveStr = moveStr.substr(1, 4);
			}
			pos.move(moveStr);
		}
	}
}