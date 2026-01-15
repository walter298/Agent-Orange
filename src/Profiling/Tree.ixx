module;

#if PROFILING
#define MAKE_PROFILER(funcName, strName, ...) \
	export BasicProfiler& funcName() { \
		thread_local auto profiler = addNode(strName, ##__VA_ARGS__);\
		return *profiler; \
	}; 
#else
#define MAKE_PROFILER(funcName, strName, ...) \
	export DummyProfiler& funcName() {\
		thread_local DummyProfiler profiler; \
		return profiler; \
	}; 
#endif

export module Chess.Profiler:Tree;

import :ProfilerMap;

using namespace std::literals;

namespace chess {
	MAKE_PROFILER(getBestMoveProfiler, "findBestMove")
	MAKE_PROFILER(getLegalMoveGenerationProfiler, "calcAllLegalMoves", "findBestMove")
	MAKE_PROFILER(getMoveAdderProfiler, "addMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getCalcPinnedAlliesProfiler, "calcPinnedAllies", "calcAllLegalMoves")
	MAKE_PROFILER(getCalcPinDataProfiler, "calcPinData", "calcPinnedAllies")
	MAKE_PROFILER(getEnemyMoveProfiler, "calcEnemyMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getEnPessantProfiler, "getEnPessantMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getMovePrioritiesProfiler, "getMovePriorities", "findBestMove")
	MAKE_PROFILER(getStaticEvaluationProfiler, "staticEvaluation", "findBestMove")
	MAKE_PROFILER(getStorePositionEntryProfiler, "storePositionEntry", "findBestMove")
	MAKE_PROFILER(getGetPositionEntryProfiler, "getPositionEntry", "findBestMove")

	export void createTree() {
		static bool createdTree = false;
		if (createdTree) {
			throw std::runtime_error{ "Error: Profiler tree has already been created." };
		}
		createdTree = true;

		getBestMoveProfiler();
		getLegalMoveGenerationProfiler();
		getEnemyMoveProfiler();
		getMoveAdderProfiler();
		getMovePrioritiesProfiler();
		getStaticEvaluationProfiler();
		getStorePositionEntryProfiler();
		getGetPositionEntryProfiler();
	}
}