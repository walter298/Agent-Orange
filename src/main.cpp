import std;

import Chess.BitboardImage;
import Chess.LegalMoveGeneration;
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

	void playUCIWithDepth(const char** argv, int argc) {
		if (argc != 3) {
			std::println("Error: uci with depth requires 1 argument: [depth]");
			return;
		}
		std::uint8_t depth = 0;
		auto depthStr = argv[2];
		auto depthStrEnd = depthStr + std::strlen(depthStr);
		auto depthRes = std::from_chars(depthStr, depthStrEnd, depth, 10);
		if (depthRes.ec != std::errc{}) {
			std::println("Error: could not parse depth argument");
			return;
		}

		if (depth < 2) {
			std::println("Error: depth must be at least 2");
			return;
		}

		playUCI(depth);
	}

	void printCommandLineArgumentOptions() {
		std::println("Options:");
		std::println("(none)\t\t\t\t\t\t- Start the engine in UCI mode (default depth = 6)");
		std::println("uci [depth]\t\t\t\t\t- Start the engine in UCI mode with specified depth");
		std::println("test\t\t\t\t\t\t- Run all tests");
		std::println("draw_bitboard [bitboard, base, filename]\t- Draw a bitboard image");
		std::println("magic");
	}
}

int main(int argc, const char** argv) {
	chess::MaybeProfilerGuard guard;

	if (argc == 1) {
		chess::playUCI();
	} else if (std::strcmp(argv[1], "uci") == 0) {
		chess::playUCIWithDepth(argv, argc);
	} else if (std::strcmp(argv[1], "test") == 0) {
		chess::tests::runAllTests();
	} else if (std::strcmp(argv[1], "draw_bitboard") == 0) {
		chess::handleBitboardInput(argv, argc);
	} else if (std::strcmp(argv[1], "help") == 0) {
		chess::printCommandLineArgumentOptions();
	} else if (std::strcmp(argv[1], "magic") == 0) {
		chess::generateMagicBitboardTable();
	} else {
		std::print("Invalid command line arguments. ");
		chess::printCommandLineArgumentOptions();
	}
	return 0;
}