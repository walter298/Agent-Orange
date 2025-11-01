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

		if (token == "startpos") {
			iss.clear();
			iss.str(std::string{ STARTING_FEN_STRING.data(), STARTING_FEN_STRING.size() });
		} else {
			assert(token == "fen");
		}

		iss >> ret.board;
		iss >> ret.color;
		iss >> ret.castlingPrivileges;
		iss >> ret.enPessantSquare;

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
