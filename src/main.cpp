import std;

import Chess.UCI;
import Chess.Move;
import Chess.Profiler;
import Chess.Tests;

namespace {
	void printCommandLineArgumentOptions() {
		std::println("- local [test_position]");
		std::println("- host [ip address] [port]");
		std::println("- join_remote [ip address] [port]");
	}
}

int main(int argc, char** argv) {
	chess::MaybeProfilerGuard guard;

	if (argc == 1) {
		std::println("Error: no command line arguments supplied. Options: ");
		printCommandLineArgumentOptions();
	} else if (std::strcmp(argv[1], "local") == 0) {
		chess::playUCI();
	} else if (std::strcmp(argv[1], "test") == 0) {
		chess::tests::runAllTests();
	} else {
		std::println("{}", argv[1]);
		std::print("Error: ");
		for (int i = 0; i < argc; i++) {
			std::print("{} ", argv[i]);
		}
		std::println("are invalid command line arguments. Options: ");
		printCommandLineArgumentOptions();
	}
	return 0;
}