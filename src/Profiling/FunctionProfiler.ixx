export module Chess.Profiler:FunctionProfiler;

import std;

export namespace chess {
	namespace chrono = std::chrono;

	export class Profiler {
	private:
		std::uint64_t m_timesRun = 0;
		chrono::nanoseconds m_totalTimeSpent{ 0 };
		chrono::system_clock::time_point m_startOfMeasurement;
	public:
		Profiler(std::string name);
		Profiler(std::string parentName, std::string name);

		void start() {
			m_startOfMeasurement = chrono::system_clock::now();
		}

		void end() {
			m_timesRun++;
			auto timeSpent = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now() - m_startOfMeasurement);
			m_totalTimeSpent += timeSpent;
		}

		std::uint64_t getTimesRun() const {
			return m_timesRun;
		}

		chrono::nanoseconds getTotalTimeSpent() {
			return m_totalTimeSpent;
		}

		chrono::nanoseconds getAverage() {
			return m_totalTimeSpent / m_timesRun;
		}
	};

	struct DummyProfiler {
		DummyProfiler(std::string name) {}
		DummyProfiler(std::string parentName, std::string name) {}
		void start() {}
		void end() {}
	};

	export using MaybeProfiler = std::conditional_t<PROFILING, Profiler, DummyProfiler>;

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