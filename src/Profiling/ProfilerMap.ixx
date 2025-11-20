export module Chess.Profiler:ProfilerMap;

import std;

import :BasicProfiler;

namespace chess {
	struct ProfilerNode {
		BasicProfiler profiler;
		std::unordered_set<std::string> childNames;
	};

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func);
	BasicProfiler* mapProfiler(const std::string& name);
	void addChild(const std::string& parent, const std::string& child);
	const ProfilerNode& getProfilerNode(const std::string& name);
}