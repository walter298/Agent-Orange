module Chess.Profiler:FunctionProfiler;

import :ProfilerMap;

namespace chess {
	using namespace std::literals;

	Profiler::Profiler(std::string name) {
		mapProfiler(name, this);
	}

	Profiler::Profiler(std::string parentName, std::string name)
		: Profiler{ name }
	{
		addChild(parentName, name);
	}
}