export module Chess.Profiler:ProfilerGuard;

import std;

namespace chess {
	struct Dummy {};

	struct ProfilerGuard {
		ProfilerGuard();
		~ProfilerGuard();
	};

	export using MaybeProfilerGuard = std::conditional_t<PROFILING, ProfilerGuard, Dummy>;
}