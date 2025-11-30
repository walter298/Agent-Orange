import std;

import Chess.BitboardImage;
import Chess.UCI;
import Chess.Move;
import Chess.Profiler;
import Chess.Tests;

namespace chess {
	void handleBitboardInput(const char** argv, int argc) {
		if (argc != 5) {
			std::println("Error: draw_bitboard requires 3 arguments: [bitboard, base, filename]");
			return;
		}
		constexpr auto BITBOARD_INDEX = 2;
		constexpr auto BASE_INDEX = 3;
		constexpr auto FILENAME_INDEX = 4;

		int base = 0;
		auto baseStr = argv[BASE_INDEX];
		auto baseStrEnd = argv[BASE_INDEX] + std::strlen(argv[BASE_INDEX]);
		auto baseRes = std::from_chars(baseStr, baseStrEnd, base, 10);
		if (baseRes.ec != std::errc{}) {
			std::println("Error: could not parse base argument");
			return;
		}

		Bitboard bitboard = 990;
		auto bitboardStr = argv[BITBOARD_INDEX];
		auto bitboardStrEnd = argv[BITBOARD_INDEX] + std::strlen(argv[BITBOARD_INDEX]);
		auto bitboardRes = std::from_chars(bitboardStr, bitboardStrEnd, bitboard, base);
		if (bitboardRes.ec != std::errc{}) {
			std::println("Error: could not parse bitboard argument");
			return;
		}

		auto colorGetter = [bitboard](Bitboard bit) -> RGB {
			if (bitboard & bit) {
				return { 97, 10, 255 };
			} else {
				return { 255, 255, 0 };
			}
		};

		drawBitboardImage(colorGetter, argv[FILENAME_INDEX]);
	}

}

int main(int argc, const char** argv) {
	chess::MaybeProfilerGuard guard;

	if (argc == 1) {
		std::println("Error: no command line arguments supplied");
	} else if (std::strcmp(argv[1], "local") == 0) {
		chess::playUCI();
	} else if (std::strcmp(argv[1], "test") == 0) {
		chess::tests::runAllTests();
	} else if (std::strcmp(argv[1], "draw_bitboard") == 0) {
		chess::handleBitboardInput(argv, argc);
	} else {
		std::println("{}", argv[1]);
		std::print("Error: ");
		for (int i = 0; i < argc; i++) {
			std::print("{} ", argv[i]);
		}
		std::println("are invalid command line arguments");
	}
	return 0;
}