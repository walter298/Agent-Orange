export module Chess.Profiler:FinishedProgramFlag;

import std;

namespace chess {
	bool finishedProgram = false;

	export void markProgramAsFinished() {
		if (finishedProgram) {
			std::println("Error: program can only marked as finished once.");
			std::exit(-1);
		}
		finishedProgram = true;
	}

	export bool isProgramFinished() {
		return finishedProgram;
	}
}