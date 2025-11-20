export module Chess.Profiler:ProfilerOwner;

import std;

import :BasicProfiler;

namespace chess {
	class ProfilerOwner {
	private:
		BasicProfiler* m_profiler;
	public:
		ProfilerOwner(const std::string& name);
		ProfilerOwner(const std::string& parent, const std::string& name);

		void start() {
			m_profiler->start();
		}
		void end() {
			m_profiler->end();
		}
	};

	struct DummyProfiler {
		DummyProfiler(std::string name) {}
		DummyProfiler(std::string parentName, std::string name) {}
		void start() {}
		void end() {}
	};

	export using MaybeProfiler = std::conditional_t<PROFILING, ProfilerOwner, DummyProfiler>;

	export template<typename P>
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