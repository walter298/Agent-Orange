module Chess.Profiler:ProfilerOwner;

import :ProfilerMap;

namespace chess {
	ProfilerOwner::ProfilerOwner(const std::string& name)
		: m_profiler{ mapProfiler(name) }
	{
	}

	ProfilerOwner::ProfilerOwner(const std::string& parent, const std::string& name)
		: ProfilerOwner{ name }
	{
		addChild(parent, name);
	}
}