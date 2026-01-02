export module Chess.Profiler:BasicProfiler;

import std;
import nlohmann.json;

import Chess.ProgramEnd;

export namespace chess {
	namespace chrono = std::chrono;

	class BasicProfiler : public std::enable_shared_from_this<BasicProfiler> {
	protected:
		std::uint64_t m_timesRun = 0;
		chrono::nanoseconds m_totalTimeSpent{ 0 };
		chrono::system_clock::time_point m_startOfMeasurement;
	public:
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

		chrono::nanoseconds getTotalTimeSpent() const {
			return m_totalTimeSpent;
		}

		virtual ~BasicProfiler() = default;
	};

	struct DummyProfiler {
		void consolidate(const DummyProfiler& other) {}
		void start() {}
		void end() {}
	};

	template<typename Profiler = BasicProfiler>
	using MaybeProfiler = std::conditional_t<PROFILING, Profiler, DummyProfiler>;
}
