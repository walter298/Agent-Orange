module;

#include <cassert>

module Chess.PositionCommand;

namespace chess {
	PositionCommand parsePositionCommand(const std::string& fenStr) {
		PositionCommand ret;

		std::istringstream iss{ fenStr };

		std::string token;
		iss >> token;

		//"position" token should already have been consumed in UCI command parser 
		assert(token != "position");

		auto getFENTokens = [&ret](std::istringstream& iss) {
			iss >> ret.board;
			iss >> ret.color;
			iss >> ret.castlingPrivileges;
			iss >> ret.enPessantSquare;
		};
		
		if (token == "startpos") {
			std::istringstream newIss{ std::string{ STARTING_FEN_STRING.data(), STARTING_FEN_STRING.size() } };
			getFENTokens(newIss);
		} else {
			assert(token == "fen");
			getFENTokens(iss);
		}

		//skip halfmove clock and fullmove number
		while (iss >> token && token != "moves");

		if (token == "moves") {
			while (iss >> token) {
				ret.moves.push_back(token);
			}
		}

		return ret; 
	}
}
