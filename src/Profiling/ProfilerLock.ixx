export module Chess.Profiler:ProfilerLock;

export namespace chess {
	template<typename P>
	struct ProfilerLock {
		P& profiler;

		ProfilerLock(P& profiler) : profiler{ profiler }
		{
			profiler.start();
		}
		~ProfilerLock() {
			profiler.end();
		}
	};
}