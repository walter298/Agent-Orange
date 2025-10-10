module;

#include <boost/parser/parser.hpp>

module Chess.UCI:Handshake;

import std;
import Chess.Position; //redundant import, needed to prevent compiler bug

namespace chess {
	void verifyUCIString(const std::string& input, const char* expected) {
		if (input != expected) {
			std::println("Error: expected {} but {} was sent", expected, input);
			std::exit(1);
		}
	}

	chess::Position parsePosition(std::string& input) {
		/*position fen 2q1k3 / 8 / 8 / 8 / 8 / 8 / 8 / 3K1Q2 w - -0 1 moves d1d2*/

		if (!std::getline(std::cin >> std::ws, input)) {
			std::println("Failed to read position string");
			std::exit(1);
		}

		auto posString = input;

		std::getline(std::cin, input); //todo: handle time control 

		namespace bp = boost::parser;
		auto beforeFEN = +(bp::char_ - bp::char_('f')) >> bp::lit("fen ");
		auto fenRes = bp::parse(posString, bp::omit[beforeFEN] >> +bp::char_);
		if (fenRes) {
			return Position::fromFENString(*fenRes);
		} else {
			return Position::fromStartPos(posString);
		}
	}

	void doUCIHandshake(std::string& input) {
		std::cin >> input;
		verifyUCIString(input, "uci");

		//send the engine info! Hopefully they already resign at this point
		constexpr auto ENGINE_INFO = "id name Agent Orange\n"
			"id author Walter Stein-Smith\n"
			"uciok";
		std::println("{}", ENGINE_INFO);

		std::cin >> input;
		verifyUCIString(input, "isready");
		std::println("readyok");

		std::cin >> input;
		verifyUCIString(input, "ucinewgame");

		std::cin >> input;
		verifyUCIString(input, "isready");

		std::println("readyok");
	}

	Position parseHandshakePosition() {
		std::string line;
		doUCIHandshake(line);
		return parsePosition(line);
	}
}