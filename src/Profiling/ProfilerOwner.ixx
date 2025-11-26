export module Chess.Profiler:ProfilerOwner;

import std;

export import :BasicProfiler;
import :ProfilerMap;

namespace chess {
	template<std::derived_from<BasicProfiler> Profiler = BasicProfiler>
	class ProfilerOwner {
	private:
		Profiler m_profiler;
		std::string m_name;
	public:
		ProfilerOwner(auto&& profiler, const std::string& name, const std::string& parent)
			: m_profiler{ std::forward<Profiler>(profiler) }, m_name{ name }
		{
			mapProfiler(name, &m_profiler);
			if (!parent.empty()) {
				addChild(name, parent);
			}
		}

		ProfilerOwner(const std::string& name, const std::string& parent = "")
			: ProfilerOwner{ Profiler{}, name, parent }
		{
		}

		void start() {
			m_profiler.start();
		}
		void end() {
			m_profiler.end();
		}

		auto& get(this auto&& self) {
			return self.m_profiler;
		}

		~ProfilerOwner() {
			if (!isProgramFinished()) {
				std::println("Error: {} destructed before program ended", m_name);
				std::exit(-1);
			}
		}
	};

	struct DummyProfiler {
		DummyProfiler(auto&&, std::string, std::string) {}
		DummyProfiler(std::string, std::string) {}
		void start() {}
		void end() {}
	};

	export template<typename Profiler = BasicProfiler>
	using MaybeProfiler = std::conditional_t<PROFILING, ProfilerOwner<Profiler>, DummyProfiler>;
}