module;

#define MAKE_PROFILER_IMPL(ProfilerType, funcName, strName, ...)\
	MaybeProfiler<ProfilerType>& funcName() { \
		static MaybeProfiler<ProfilerType> profiler{ strName, ##__VA_ARGS__ }; \
		return profiler; \
	}; \

#define MAKE_PROFILER(funcName, strName, ...) \
	MAKE_PROFILER_IMPL(BasicProfiler, funcName, strName, __VA_ARGS__)

export module Chess.Profiler:Tree;

import :ProfilerOwner;

using namespace std::literals;

export namespace chess {
	MAKE_PROFILER(getBestMoveProfiler, "findBestMove")

	struct LegalMoveGenerationProfiler : public BasicProfiler {
		size_t legalMovesGenerated = 0;

		nlohmann::json getUniqueJson() const override {
			namespace ch = std::chrono;

			nlohmann::json ret;
			ret["total_moves_generated"] = legalMovesGenerated;

			if (getTotalTimeSpent() < 1'000'000'000ns) {
				auto ratioMakeupFactor = (1'000'000'000.0 / static_cast<double>(getTotalTimeSpent().count()));
				ret["moves_per_second"] = static_cast<double>(legalMovesGenerated) * ratioMakeupFactor;
			} else {
				auto seconds = ch::duration_cast<ch::seconds>(getTotalTimeSpent());
				ret["moves_per_second"] = static_cast<double>(legalMovesGenerated) / static_cast<double>(seconds.count());
			}

			return ret;
		}
	};

	MAKE_PROFILER_IMPL(LegalMoveGenerationProfiler, getLegalMoveGenerationProfiler, "calcAllLegalMoves", "findBestMove")

	MAKE_PROFILER(getMoveAdderProfiler, "addMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getEnemyMoveProfiler, "calcEnemyMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getEnPessantProfiler, "getEnPessantMoves", "calcAllLegalMoves")
	MAKE_PROFILER(getMovePrioritiesProfiler, "getMovePriorities", "findBestMove")
	MAKE_PROFILER(getStaticEvaluationProfiler, "staticEvaluation", "findBestMove")
	MAKE_PROFILER(getStorePositionRatingProfiler, "storePositionRating", "findBestMove")
	MAKE_PROFILER(getGetPositionRatingProfiler, "getPositionRating", "findBestMove")

	void createTree() {
		static bool createdGraph = false;
		if (createdGraph) {
			throw std::runtime_error{ "Error: Profiler graph has already been created." };
		}
		createdGraph = true;

		getBestMoveProfiler();
		getLegalMoveGenerationProfiler();
		getEnemyMoveProfiler();
		getMoveAdderProfiler();
		getMovePrioritiesProfiler();
		getStaticEvaluationProfiler();
		getStorePositionRatingProfiler();
		getGetPositionRatingProfiler();
	}
}